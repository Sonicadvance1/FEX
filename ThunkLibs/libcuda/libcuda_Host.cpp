// SPDX-License-Identifier: MIT
#include "common/Host.h"
#include "cuda_defines.h"
#include "cuda_exports.h"

#include <stdio.h>
#include <dlfcn.h>
#include <string>
#include <thread>
#include <chrono>
#include <format>
#include <string_view>
#include <inttypes.h>
#include <sys/mman.h>
#include <unistd.h>
#include "thunkgen_host_libcuda.inl"

#define FEXFN_IMPL(fn) fexfn_impl_libcuda_##fn
#define FEXFN_UNPACK(fn) fexfn_unpack_libcuda_##fn
#define LDR_PTR(fn) fexldr_ptr_libcuda_##fn

struct override_entry {
  std::string_view name;
  void* ptr;
};

constexpr static std::array<override_entry, 2> proc_override = {
  {{"cuCtxCreate_v2", (void*)FEXFN_IMPL(cuCtxCreate_v2)}, {"cuGetExportTable", (void*)FEXFN_IMPL(cuGetExportTable)}}};

static CUresult FEXFN_IMPL(cuGetProcAddress_v2)(const char* symbol, guest_layout<void**> pfn, int cudaVersion, cuuint64_t flags,
                                                CUdriverProcAddressQueryResult* symbolStatus) {
  host_layout<void*> host_data {};
  void* ptr {};
  CUresult ret {};
  for (auto& over : proc_override) {
    if (symbol == over.name) {
      ptr = over.ptr;
      if (symbolStatus) {
        *symbolStatus = (CUdriverProcAddressQueryResult)0; // CU_GET_PROC_ADDRESS_SUCCESS
      }
      ret = (CUresult)0; // CUDA_SUCCESS
      break;
    }
  }

  if (!ptr) {
    ret = LDR_PTR(cuGetProcAddress_v2)(symbol, &ptr, cudaVersion, flags, symbolStatus);
  }

  host_data.data = ptr;
  *pfn.get_pointer() = to_guest(host_data);
  return ret;
}

CUresult FEXFN_IMPL(cuCtxCreate_v2)(guest_layout<CUcontext*> pctx, unsigned int flags, CUdevice dev) {
  host_layout<CUcontext> host_data {};
  CUcontext ctx;
  auto ret = LDR_PTR(cuCtxCreate_v2)(&ctx, flags, dev);
  host_data.data = ctx;
  *pctx.get_pointer() = to_guest(host_data);
  return ret;
}

void cudart_initialize(cuda::export_table_header* header, const void*);

auto cudart_interface = []() consteval {
  const cuda::export_table_data<13> d {
    .header =
      {
        .uuid = {0x6b, 0xd5, 0xfb, 0x6c, 0x5b, 0xf4, 0xe7, 0x4a, 0x89, 0x87, 0xd9, 0x39, 0x12, 0xfd, 0x9d, 0xf9},
        .Initializer = cudart_initialize,
      },
    .entries =
      {
        // 0: size
        sizeof(d.entries) + sizeof(uint64_t),
        // 1: get_module_from_cubin
        1,
        // 2: get_primary_context
        2,
        // 3: UNK
        3,
        // 4: UNK
        4,
        // 5: UNK
        5,
        // 6: get_module_from_cubin_ex1
        6,
        // 7: UNK
        7,
        // 8: get_module_from_cubin_ex2
        8,
        // 9: launch_kernel
        9,
        // 10: UNK - Zero from host.
        0,
        // 11: UNK
        11,
        // 12: UNK
        12,
      },
  };

  return d;
}();

uint64_t host_cudart_interface[13] {};

CUresult FEXFN_IMPL(cudart_2_get_primary_context)(CUcontext* pctx, CUdevice dev) {
  fprintf(stderr, "%s(%p, %d)\n", __FUNCTION__, pctx, dev);
  using func_type = CUresult (*)(CUcontext*, CUdevice);

  uint64_t ptr_location = host_cudart_interface[2];
  auto ret = ((func_type)ptr_location)(pctx, dev);
  fprintf(stderr, "\tret: %d\n", ret);
  return ret;
}

void cudart_initialize(cuda::export_table_header* header, const void* host_entry) {
  auto host_entries = reinterpret_cast<const uint64_t*>(host_entry);
  const uint64_t host_entries_size = host_entries[0] / sizeof(uint64_t);
  const auto min_known_entries = std::min<size_t>(host_entries_size, 13);

  // Save the host entries
  memcpy(host_cudart_interface, host_entries, min_known_entries * sizeof(uint64_t));

  cudart_interface.entries[2] = reinterpret_cast<uint64_t>(&FEXFN_IMPL(cudart_2_get_primary_context));
  fprintf(stderr, "Set ptr 2 to 0x%" PRIx64 "\n", cudart_interface.entries[2]);
  header->Initialized = true;
}

void tool_runtime_initialize(cuda::export_table_header* header, const void*);
auto tool_runtime_interface = []() consteval {
  const cuda::export_table_data<7> d {
    .header =
      {
        .uuid = {0xa0, 0x94, 0x79, 0x8c, 0x2e, 0x74, 0x2e, 0x74, 0x93, 0xf2, 0x08, 0x00, 0x20, 0x0c, 0x0a, 0x66},
        .Initializer = tool_runtime_initialize,
      },
    // Might just be callbacks with different argument counts?
    .entries =
      {
        // 0: size
        sizeof(d.entries),
        // 1: UNK
        13,
        // 2: UNK
        14,
        // 3: UNK
        15,
        // 4: UNK
        16,
        // 5: UNK
        17,
        // 6: UNK
        18,
      },
  };

  return d;
}();


uint64_t host_tool_runtime_interface[7] {};

CUresult FEXFN_IMPL(tool_runtime_2_callback_hook)(void* hptr, uint64_t* size) {
  fprintf(stderr, "host: %s(%p, %p)\n", __FUNCTION__, hptr, size);
  // hptr == 24 bytes
  using func_type = CUresult (*)(void*, uint64_t*);

  uint64_t ptr_location = host_cudart_interface[2];
  auto ret = ((func_type)ptr_location)(hptr, size);
  fprintf(stderr, "\tret: %d\n", ret);
  *size = 468;
  return ret;
}

CUresult FEXFN_IMPL(tool_runtime_6_callback_hook)(void* hptr, uint64_t* size) {
  fprintf(stderr, "host: %s(%p, %p)\n", __FUNCTION__, hptr, size);
  // hptr == 24 bytes
  using func_type = CUresult (*)(void*, uint64_t*);

  uint64_t ptr_location = host_cudart_interface[6];
  auto ret = ((func_type)ptr_location)(hptr, size);
  fprintf(stderr, "\tret: %d\n", ret);
  *size = 468;
  return ret;
}

void tool_runtime_initialize(cuda::export_table_header* header, const void* host_entry) {
  auto host_entries = reinterpret_cast<const uint64_t*>(host_entry);
  const uint64_t host_entries_size = host_entries[0] / sizeof(uint64_t);
  const auto min_known_entries = std::min<size_t>(host_entries_size, 7);

  // Save the host entries
  memcpy(host_tool_runtime_interface, host_entries, min_known_entries * sizeof(uint64_t));

  tool_runtime_interface.entries[2] = reinterpret_cast<uint64_t>(&FEXFN_IMPL(tool_runtime_2_callback_hook));
  tool_runtime_interface.entries[6] = reinterpret_cast<uint64_t>(&FEXFN_IMPL(tool_runtime_6_callback_hook));

  fprintf(stderr, "Set ptr 2 to 0x%" PRIx64 "\n", tool_runtime_interface.entries[2]);
  fprintf(stderr, "Set ptr 6 to 0x%" PRIx64 "\n", tool_runtime_interface.entries[6]);

  header->Initialized = true;
}

void tools_tls_initialize(cuda::export_table_header* header, const void*);

auto tools_tls_interface = []() consteval {
  const cuda::export_table_data<3> d {
    .header =
      {
        .uuid = {0x42, 0xd8, 0x5a, 0x81, 0x23, 0xf6, 0xcb, 0x47, 0x82, 0x98, 0xf6, 0xe7, 0x8a, 0x3a, 0xec, 0xdc},
        .Initializer = tools_tls_initialize,
      },
    .entries =
      {
        // 0: size
        sizeof(d.entries),
        // 1: UNK
        19,
        // 2: UNK
        20,
      },
  };

  return d;
}();

void tools_tls_initialize(cuda::export_table_header* header, const void* host_entry) {
  auto host_entries = reinterpret_cast<const uint64_t*>(host_entry);
  const uint64_t host_entries_size = host_entries[0] / sizeof(uint64_t) - 1;
  host_entries++;

  fprintf(stderr, "Host TLS size: %" PRId64 "\n", host_entries_size);
  header->Initialized = true;
}

void context_local_initialize(cuda::export_table_header* header, const void*);

auto context_local_interface = []() consteval {
  const cuda::export_table_data<1> d {
    .header =
      {
        .uuid = {0xc6, 0x93, 0x33, 0x6e, 0x11, 0x21, 0xdf, 0x11, 0xa8, 0xc3, 0x68, 0xf3, 0x55, 0xd8, 0x95, 0x93},
        .Initializer = context_local_initialize,
      },
    .entries =
      {
        // nullptr as final entry in list
        0,
      },
  };

  return d;
}();

void context_local_initialize(cuda::export_table_header* header, const void* host_entry) {
  // Host interface has 37 entries on my tested platform. Not sure what they are used for.
  header->Initialized = true;
}

void context_wrapper_initialize(cuda::export_table_header* header, const void*);

auto context_wrapper_interface = []() consteval {
  const cuda::export_table_data<3> d {
    .header =
      {
        .uuid = {0x26, 0x3e, 0x88, 0x60, 0x7c, 0xd2, 0x61, 0x43, 0x92, 0xf6, 0xbb, 0xd5, 0x00, 0x6d, 0xfa, 0x7e},
        .Initializer = context_wrapper_initialize,
      },
    .entries =
      {
        // 0: size
        sizeof(d.entries),
        // 1: UNK
        21,
        // 2: UNK
        22,
      },
  };

  return d;
}();

void context_wrapper_initialize(cuda::export_table_header* header, const void* host_entry) {
  // Host interface has 16 entries on my tested platform.
  auto host_entries = reinterpret_cast<const uint64_t*>(host_entry);
  const uint64_t host_entries_size = host_entries[0] / sizeof(uint64_t);

  fprintf(stderr, "Context wrapper size: %" PRId64 "\n", host_entries_size);
  header->Initialized = true;
}

void anti_zluda_initialize(cuda::export_table_header* header, const void*);

auto anti_zluda_interface = []() consteval {
  const cuda::export_table_data<3> d {
    .header =
      {
        .uuid = {0xd4, 0x08, 0x20, 0x55, 0xbd, 0xe6, 0x70, 0x4b, 0x8d, 0x34, 0xba, 0x12, 0x3c, 0x66, 0xe1, 0xf2},
        .Initializer = anti_zluda_initialize,
      },
    .entries =
      {
        // 0: size
        sizeof(d.entries),
        // 1: UNK
        23,
        // 2: UNK
        24,
      },
  };

  return d;
}();

uint64_t host_anti_zluda_interface[3] {};

void anti_zluda_initialize(cuda::export_table_header* header, const void* host_entry) {
  // Host interface has 2 entries on my tested platform.
  auto host_entries = reinterpret_cast<const uint64_t*>(host_entry);
  const uint64_t host_entries_size = host_entries[0] / sizeof(uint64_t);
  const auto min_known_entries = std::min<size_t>(host_entries_size, 3);

  // Save the host entries
  memcpy(host_anti_zluda_interface, host_entries, min_known_entries * sizeof(uint64_t));

  anti_zluda_interface.entries[1] = reinterpret_cast<uint64_t>(&FEXFN_UNPACK(anti_zluda_check_1));

  fprintf(stderr, "Set ptr 1 to 0x%" PRIx64 "\n", anti_zluda_interface.entries[1]);

  header->Initialized = true;
}

CUresult FEXFN_IMPL(anti_zluda_check_1)(uint32_t runtime_version, uint64_t timestamp, anti_zluda_result* result) {
  fprintf(stderr, "host: %s\n", __FUNCTION__);
  using func_type = CUresult (*)(uint32_t, uint64_t, anti_zluda_result*);

  uint64_t ptr_location = host_anti_zluda_interface[1];
  auto ret = ((func_type)ptr_location)(runtime_version, timestamp, result);
  fprintf(stderr, "\tret: %d\n", ret);
  return ret;
}

constexpr static std::array<void*, 6> export_table = {{
  &cudart_interface,
  &tool_runtime_interface,
  &tools_tls_interface,
  &context_local_interface,
  &context_wrapper_interface,
  &anti_zluda_interface,
}};

cuda::export_table_header* FindEntry(const CUuuid* TableID) {
  for (auto entry : export_table) {
    auto header = reinterpret_cast<cuda::export_table_header*>(entry);
    if (memcmp(header->uuid, TableID, sizeof(CUuuid)) == 0) {
      return header;
    }
  }

  return nullptr;
}

CUresult FEXFN_IMPL(cuGetExportTable)(guest_layout<const void**> ppExportTable, const CUuuid* pExportTableId) {
  // This function returns a pointer to an driver internal export table that is undocumented publicly.
  //
  // Some documentation about the UUIDs that has been reversed.
  // https://github.com/vosen/ZLUDA/blob/1b9ba2b2333746c5e2b05a2bf24fa6ec3828dcdf/zluda_dark_api/src/lib.rs#L197
  //
  // These two UUIDs are required by the static cuda runtime at startup. Might require more but it currently halts without these
  // implemented. UUID: [0x6b, 0xd5, 0xfb, 0x6c, 0x5b, 0xf4, 0xe7, 0x4a, 0x89, 0x87, 0xd9, 0x39, 0x12, 0xfd, 0x9d, 0xf9]
  // - cudart interface (0x68 bytes?)
  // UUID: [0xa0, 0x94, 0x79, 0x8c, 0x2e, 0x74, 0x2e, 0x74, 0x93, 0xf2, 0x08, 0x00, 0x20, 0x0c, 0x0a, 0x66]
  // - Tools runtime callback hooks (0x38 bytes?)

  const void* ptr {};
  auto ret = LDR_PTR(cuGetExportTable)(&ptr, pExportTableId);

  if (ret != 0) {
    return ret;
  }

  auto entry = FindEntry(pExportTableId);
  if (entry) {
    if (entry->Initializer && !entry->Initialized) {
      entry->Initializer(entry, ptr);
    }
    auto entry_with_data = reinterpret_cast<const cuda::export_table_data<0>*>(entry);
    host_layout<const void*> host_data {};
    host_data.data = &entry_with_data->entries[0];
    *ppExportTable.get_pointer() = to_guest(host_data);
    return (CUresult)0; // CUDA_SUCCESS
  }

  std::string uuid {};
  for (size_t i = 0; i < sizeof(CUuuid); ++i) {
    const bool last = (i + 1) == sizeof(CUuuid);
    uuid += std::format("0x{:02x}{}", pExportTableId->_0[i], last ? "" : ", ");
  }
  fprintf(stderr, "Host: cuGetExportTable: Couldn't find UUID: %s\n", uuid.c_str());

  return (CUresult)4; // CUDA_ERROR_DEINITIALIZED
}

EXPORTS(libcuda)
