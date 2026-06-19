// SPDX-License-Identifier: MIT
#include "common/Guest.h"
#include "cuda_defines.h"
#include "cuda_exports.h"
#include <cstdio>
#include <format>
#include <thread>
#include <chrono>
#include <inttypes.h>

#include "thunkgen_guest_libcuda.inl"
#include <cstring>
#include <dlfcn.h>
#include <functional>
#include <string_view>
#include <string>
#include <unordered_map>

#include <cstdint>

// Maps cuda API function names to the address of a guest function which is
// linked to the corresponding host function pointer
const std::unordered_map<std::string_view, uintptr_t /* guest function address */> HostPtrInvokers = std::invoke([]() {
#define PAIR(name, unused) Ret[#name] = reinterpret_cast<uintptr_t>(GetCallerForHostFunction(name));
  std::unordered_map<std::string_view, uintptr_t> Ret;
  FOREACH_internal_SYMBOL(PAIR);
  return Ret;
#undef PAIR
});

extern "C" {

// This variable controls the behavior of cuGetProcAddress for functions we don't know the signature of:
// - if false (default), we return a nullptr (since the application might have a fallback code path)
// - if true, we return a stub function that fatally errors upon being called
constexpr bool stub_unknown_functions = false;

// Fatally erroring function with a thunk-like interface. This is used as a placeholder for unknown CUDA functions
[[noreturn]]
static void FatalError(void* raw_args) {
  auto called_function = reinterpret_cast<PackedArguments<void, uintptr_t>*>(raw_args)->a0;
  fprintf(stderr, "FATAL: Called unknown CUDA function at address %p\n", reinterpret_cast<void*>(called_function));
  __builtin_trap();
}

static void* MakeGuestCallable(const char* origin, void* func, const char* name) {
  auto It = HostPtrInvokers.find(name);
  if (It == HostPtrInvokers.end()) {
    fprintf(stderr, "%s: Unknown cuda function at address %p: %s\n", origin, func, name);
    if (stub_unknown_functions) {
      const auto StubHostPtrInvoker = CallHostFunction<FatalError, void>;
      LinkAddressToFunction((uintptr_t)func, reinterpret_cast<uintptr_t>(StubHostPtrInvoker));
      return func;
    }
    return nullptr;
  }
  LinkAddressToFunction((uintptr_t)func, It->second);
  return func;
}

struct override_entry {
  std::string_view name;
  void* ptr;
};

constexpr static std::array<override_entry, 3> proc_override = {{
  {"cuGetProcAddress", (void*)cuGetProcAddress_v2},
  {"cuGetProcAddress_v2", (void*)cuGetProcAddress_v2},
  {"cuGetExportTable", (void*)cuGetExportTable},
}};

CUresult cuGetProcAddress_v2(const char* symbol, void** pfn, int cudaVersion, cuuint64_t flags, CUdriverProcAddressQueryResult* symbolStatus) {

  for (auto& over : proc_override) {
    if (symbol == over.name) {
      *pfn = over.ptr;
      if (symbolStatus) {
        *symbolStatus = (CUdriverProcAddressQueryResult)0;
      }
      return (CUresult)0; // CUDA_SUCCESS
    }
  }

  void* ptr {};
  auto Ret = fexfn_pack_cuGetProcAddress_v2(symbol, &ptr, cudaVersion, flags, symbolStatus);

  if (!Ret && ptr) {
    *pfn = MakeGuestCallable(__FUNCTION__, ptr, symbol);
  }
  return Ret;
}

void cudart_initialize(cuda::export_table_header* header, const void* host_entry);

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
        sizeof(d.entries),
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
        10,
        // 11: UNK
        11,
        // 12: UNK
        12,
      },
  };

  return d;
}();

#define countof(array) (sizeof(array) / sizeof(array[0]))
void cudart_initialize(cuda::export_table_header* header, const void* host_entry) {
  fprintf(stderr, "initialzing cudart entry\n");
  auto guest_entry_with_data = reinterpret_cast<cuda::export_table_data<countof(cudart_interface.entries)>*>(header);

  // Initialize the guest entries
  guest_entry_with_data->entries[2] = reinterpret_cast<uint64_t>(&cudart_2_get_primary_context);

  header->Initialized = true;
}

#define FEXFN_PACK(fn) fexfn_pack_##fn

void tool_runtime_initialize(cuda::export_table_header* header, const void* host_entry);
auto tool_runtime_callbacks = []() consteval {
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

CUresult tool_runtime_2_callback_hook(void* a1, uint64_t* a2) {
  fprintf(stderr, "%s called from 0x%" PRIx64 "\n", __FUNCTION__, (uint64_t)__builtin_return_address(0));
  return FEXFN_PACK(tool_runtime_2_callback_hook)(a1, a2);
}

void tool_runtime_initialize(cuda::export_table_header* header, const void* host_entry) {
  fprintf(stderr, "initialzing tool_runtime entry\n");
  auto guest_entry_with_data = reinterpret_cast<cuda::export_table_data<countof(tool_runtime_callbacks.entries)>*>(header);

  // Initialize the guest entries
  guest_entry_with_data->entries[2] = reinterpret_cast<uint64_t>(&tool_runtime_2_callback_hook);
  guest_entry_with_data->entries[6] = reinterpret_cast<uint64_t>(&tool_runtime_6_callback_hook);

  header->Initialized = true;
}

auto tools_tls_interface = []() consteval {
  const cuda::export_table_data<3> d {
    .header =
      {
        .uuid = {0x42, 0xd8, 0x5a, 0x81, 0x23, 0xf6, 0xcb, 0x47, 0x82, 0x98, 0xf6, 0xe7, 0x8a, 0x3a, 0xec, 0xdc},
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

void context_local_test() {
  fprintf(stderr, "%s called from 0x%" PRIx64 "\n", __FUNCTION__, (uint64_t)__builtin_return_address(0));
}

void context_local_initialize(cuda::export_table_header* header, const void* host_entry);
auto context_local_interface = []() consteval {
  const cuda::export_table_data<4> d {
    .header =
      {
        .uuid = {0xc6, 0x93, 0x33, 0x6e, 0x11, 0x21, 0xdf, 0x11, 0xa8, 0xc3, 0x68, 0xf3, 0x55, 0xd8, 0x95, 0x93},
        .Initializer = context_local_initialize,
      },
    .entries =
      {
        // 0: Unknown
        21,
        // 1: Unknown
        22,
        // 2: Unknown
        23,
        // nullptr as final entry in list
        0,
      },
  };

  return d;
}();

void context_local_initialize(cuda::export_table_header* header, const void* host_entry) {
  fprintf(stderr, "initialzing context_local entry\n");
  auto guest_entry_with_data = reinterpret_cast<cuda::export_table_data<countof(context_local_interface.entries)>*>(header);

  // Initialize the guest entries
  guest_entry_with_data->entries[0] = reinterpret_cast<uint64_t>(&context_local_test);

  header->Initialized = true;
}

uint32_t context_wrapper_3_unk() {
  fprintf(stderr, "%s called from 0x%" PRIx64 "\n", __FUNCTION__, (uint64_t)__builtin_return_address(0));
  return 0;
}

void context_wrapper_initialize(cuda::export_table_header* header, const void* host_entry);
auto context_wrapper_interface = []() consteval {
  const cuda::export_table_data<4> d {
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
        24,
        // 2: UNK
        25,
        // 3: UNK
        26,
      },
  };

  return d;
}();

void context_wrapper_initialize(cuda::export_table_header* header, const void* host_entry) {
  fprintf(stderr, "initialzing context_wrapper entry\n");
  auto guest_entry_with_data = reinterpret_cast<cuda::export_table_data<countof(context_wrapper_interface.entries)>*>(header);

  // Initialize the guest entries
  guest_entry_with_data->entries[3] = reinterpret_cast<uint64_t>(&context_wrapper_3_unk);

  header->Initialized = true;
}

void anti_zluda_initialize(cuda::export_table_header* header, const void* host_entry);
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
        27,
        // 2: UNK
        28,
      },
  };

  return d;
}();

void anti_zluda_initialize(cuda::export_table_header* header, const void* host_entry) {
  fprintf(stderr, "initialzing anti_zluda entry\n");

  auto guest_entry_with_data = reinterpret_cast<cuda::export_table_data<countof(anti_zluda_interface.entries)>*>(header);

  // Initialize the guest entries
  guest_entry_with_data->entries[1] = reinterpret_cast<uint64_t>(&anti_zluda_check_1);

  header->Initialized = true;
}

constexpr static std::array<void*, 6> export_table = {{
  &cudart_interface,
  &tool_runtime_callbacks,
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

CUresult cuGetExportTable(const void** ppExportTable, const CUuuid* pExportTableId) {
  std::string uuid {};
  for (size_t i = 0; i < sizeof(CUuuid); ++i) {
    const bool last = (i + 1) == sizeof(CUuuid);
    uuid += std::format("0x{:02x}{}", pExportTableId->_0[i], last ? "" : ", ");
  }

  fprintf(stderr, "%s called from 0x%" PRIx64 "\n", __FUNCTION__, (uint64_t)__builtin_return_address(0));
  fprintf(stderr, "\t%s\n", uuid.c_str());
  auto guest_entry = FindEntry(pExportTableId);

  auto guest_entry_with_data = reinterpret_cast<const cuda::export_table_data<0>*>(guest_entry);
  if (guest_entry && guest_entry->Initialized) {
    *ppExportTable = &guest_entry_with_data->entries[0];
    return (CUresult)0; // CUDA_SUCCESS
  }

  const void* ptr {};
  auto Ret = fexfn_pack_cuGetExportTable(&ptr, pExportTableId);

  if (Ret != 0) {
    return Ret;
  }

  if (!guest_entry) {
    fprintf(stderr, "Guest: cuGetExportTable: Couldn't find UUID: %s\n", uuid.c_str());

    return (CUresult)4; // CUDA_ERROR_DEINITIALIZED
  }

  if (guest_entry->Initializer && !guest_entry->Initialized) {
    guest_entry->Initializer(guest_entry, ptr);
  }

  *ppExportTable = &guest_entry_with_data->entries[0];
  return (CUresult)0; // CUDA_SUCCESS
}
}

LOAD_LIB(libcuda)
