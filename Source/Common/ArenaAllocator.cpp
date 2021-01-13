#include "Common/MathUtils.h"

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <list>
#include <stdio.h>
#include <memory>
#include <sys/mman.h>

//#if !(defined(ENABLE_ASAN) && ENABLE_ASAN)
//// These need to be provided as C mangled function pointers
//extern "C" {
//// glibc provided symbols
//extern void *__libc_malloc(size_t size);
//extern void *__libc_calloc(size_t num, size_t size);
//extern void *__libc_realloc(void *ptr, size_t size);
//extern void *__libc_memalign(size_t alignment, size_t size);
//extern void __libc_free(void *ptr);
//
//void *(*current_malloc)(size_t size) = __libc_malloc;
//void *(*current_realloc)(void *ptr, size_t size) = __libc_realloc;
//void *(*current_memalign)(size_t alignemnt, size_t size) = __libc_memalign;
//void (*current_free)(void *ptr) = __libc_free;
//
//void *malloc(size_t size) {
//  return current_malloc(size);
//}
//
//void *calloc(size_t num, size_t size) {
//  void *ptr = current_malloc(num * size);
//  memset(ptr, 0, num * size);
//  return ptr;
//}
//
//void *realloc(void *ptr, size_t size) {
//  return current_realloc(ptr, size);
//}
//
//void *memalign(size_t alignment, size_t size) {
//  return current_memalign(alignment, size);
//}
//
//void free(void *ptr) {
//  current_free(ptr);
//}
//}
//
//namespace SlabAllocator {
//  struct Slabs;
//
//  class SlabIterator {
//  public:
//    SlabIterator(Slabs *_Slab)
//      : Slab {_Slab} {}
//
//    SlabIterator operator++();
//    bool operator==(const SlabIterator &rhs) const;
//    Slabs *operator*();
//
//  private:
//    Slabs *Slab{};
//  };
//
//  // Make the slab size be 1GB
//  constexpr static size_t SLAB_SIZE = (1 * 1024 * 1024 * 1024);
//  constexpr static size_t PAGE_SIZE = 4096;
//  constexpr static size_t BITSET_SIZE = SLAB_SIZE / PAGE_SIZE;
//  struct Slabs {
//    union {
//      void *Ptr;
//      uintptr_t UPtr;
//    };
//
//    std::bitset<BITSET_SIZE> UsedPages;
//
//    ::SlabAllocator::Slabs *Prev{};
//    ::SlabAllocator::Slabs *Next{};
//
//    SlabIterator begin() const noexcept {
//      return SlabIterator{reinterpret_cast<Slabs*>(Ptr)};
//    }
//
//    SlabIterator end() const noexcept {
//      return SlabIterator{nullptr};
//    }
//
//  };
//
//  SlabIterator SlabIterator::operator++() {
//    Slab = Slab->Next;
//    return *this;
//  }
//
//  bool SlabIterator::operator==(const SlabIterator &rhs) const {
//    return Slab == rhs.Slab;
//  }
//  Slabs *SlabIterator::operator*() {
//    return Slab;
//  }
//};
//
//namespace SlabAllocator::x64 {
//  class SlabAllocator {
//  public:
//    ::SlabAllocator::Slabs *AllocateSlab(::SlabAllocator::Slabs *Prev = nullptr);
//
//  private:
//  };
//
//  ::SlabAllocator::Slabs * SlabAllocator::AllocateSlab(::SlabAllocator::Slabs *Prev) {
//    void *Memory = mmap(nullptr, ::SlabAllocator::SLAB_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
//    if (Memory == (void*)~0ULL) {
//      return nullptr;
//    }
//    size_t SlabDataSize = AlignUp(sizeof(::SlabAllocator::Slabs), ::SlabAllocator::SLAB_SIZE) / ::SlabAllocator::SLAB_SIZE;
//
//    ::SlabAllocator::Slabs *NewSlab = new (Memory) ::SlabAllocator::Slabs();
//    NewSlab->Ptr = Memory;
//    NewSlab->Prev = Prev;
//    NewSlab->Next = nullptr;
//
//    if (Prev) {
//      Prev->Next = NewSlab;
//    }
//
//    for (size_t i = 0; i < SlabDataSize; ++i) {
//      NewSlab->UsedPages[i] = true;
//    }
//    return NewSlab;
//  }
//}
//
//namespace ArenaAllocator {
//  std::unique_ptr<SlabAllocator::x64::SlabAllocator> Slab;
//
//  struct ThreadLocalData {
//    bool Allocated{};
//    SlabAllocator::Slabs* Slabs{};
//    SlabAllocator::Slabs* LastSlab{};
//    size_t LastOffset{};
//  };
//
//  thread_local ThreadLocalData Local;
//
//  struct MetaDataInfo {
//    uintptr_t Ptr;
//    size_t Size;
//    uint8_t ALIGN[4096 - (16)];
//    char Allocated[];
//  };
//  static_assert(offsetof(MetaDataInfo, Allocated) == 4096, "What");
//
//  class Allocator {
//  public:
//    void *malloc(size_t Size);
//    void *realloc(void *ptr, size_t new_size);
//    void free(void *ptr);
//
//  private:
//    void AllocateSlab();
//  };
//
//  void Allocator::AllocateSlab() {
//    Local.Slabs = Slab->AllocateSlab(nullptr);
//    Local.Allocated = true;
//  }
//
//  void *Allocator::malloc(size_t Size) {
//    if (!Local.Allocated) {
//      AllocateSlab();
//    }
//
//    size_t TrueSize = Size + sizeof(MetaDataInfo);
//
//    if (TrueSize >= ::SlabAllocator::SLAB_SIZE) {
//      __builtin_trap();
//      return 0;
//    }
//
//    try_again:
//    // Walk our allocated slabs to find one that fits a region that we want
//    size_t PageSize = AlignUp(TrueSize, ::SlabAllocator::PAGE_SIZE) / ::SlabAllocator::PAGE_SIZE;
//    auto CurrentSlab = Local.LastSlab ? Local.LastSlab : Local.Slabs->begin();
//    auto PrevSlab = *CurrentSlab;
//    while (*CurrentSlab != nullptr) {
//      for (size_t i = Local.LastOffset; i < (::SlabAllocator::BITSET_SIZE - PageSize);) {
//        size_t Remaining = PageSize;
//
//        while (Remaining) {
//          if ((*CurrentSlab)->UsedPages[i + Remaining - 1]) {
//            // Already consumed, skip
//            break;
//          }
//          --Remaining;
//        }
//
//        if (Remaining == 0) {
//          // Found one!
//          uintptr_t NewPtr = (*CurrentSlab)->UPtr + (i * ::SlabAllocator::PAGE_SIZE);
//          MetaDataInfo *Info = reinterpret_cast<MetaDataInfo*>(NewPtr);
//          // Store their size
//          Info->Ptr = 0x4142434445464748ULL;
//          Info->Size = Size;
//          mprotect(Info, 4096, PROT_READ);
//
//          if ((i + PageSize) > ::SlabAllocator::BITSET_SIZE) {
//            __builtin_trap();
//          }
//
//          for (size_t j = 0; j < PageSize; ++j) {
//            (*CurrentSlab)->UsedPages[i + j] = true;
//          }
//          Local.LastSlab = *CurrentSlab;
//          Local.LastOffset = i;
//          return reinterpret_cast<void*>(Info->Allocated);
//        }
//        else {
//          // Didn't find an allocation in this range
//          i += (PageSize - Remaining) + 1;
//        }
//      }
//
//      PrevSlab = *CurrentSlab;
//      ++CurrentSlab;
//      Local.LastOffset = 0;
//    }
//
//    if (Local.LastSlab) {
//      Local.LastSlab = nullptr;
//      Local.LastOffset = 0;
//      goto try_again;
//    }
//
//    auto NewSlab = Slab->AllocateSlab(PrevSlab);
//    if (NewSlab) {
//      Local.LastSlab = NewSlab;
//      Local.LastOffset = 0;
//      goto try_again;
//    }
//    return 0;
//  }
//
//  void Allocator::free(void *ptr) {
//    if (!ptr) {
//      return;
//    }
//
//    MetaDataInfo *Info = reinterpret_cast<MetaDataInfo*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(MetaDataInfo));
//    if (0x4142434445464748ULL != Info->Ptr) {
//      __builtin_trap();
//    }
//
//    mprotect(Info, 4096, PROT_READ | PROT_WRITE);
//
//    // Find the slab allocator with this memory range
//    auto CurrentSlab = Local.Slabs->begin();
//    while (*CurrentSlab != nullptr) {
//      uintptr_t SlabBegin = (*CurrentSlab)->UPtr;
//      uintptr_t SlabEnd = SlabBegin + SlabAllocator::SLAB_SIZE;
//      if (SlabBegin <= reinterpret_cast<uintptr_t>(ptr) &&
//          SlabEnd >= reinterpret_cast<uintptr_t>(ptr)) {
//        // Clear the allocated region
//        size_t RealSize = Info->Size + sizeof(MetaDataInfo);
//        size_t PageSize = AlignUp(RealSize, ::SlabAllocator::PAGE_SIZE) / ::SlabAllocator::PAGE_SIZE;
//        size_t PageOffset = (reinterpret_cast<uintptr_t>(ptr) - SlabBegin) / ::SlabAllocator::PAGE_SIZE;
//        for (size_t j = 0; j < PageSize; ++j) {
//          (*CurrentSlab)->UsedPages[PageOffset + j] = false;
//        }
//        return;
//      }
//      ++CurrentSlab;
//    }
//  }
//
//  void *Allocator::realloc(void *ptr, size_t new_size) {
//    if (!ptr) {
//      return this->malloc(new_size);
//    }
//
//    printf("REALLOC\n");
//    MetaDataInfo *Info = reinterpret_cast<MetaDataInfo*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(MetaDataInfo));
//    if (0x4142434445464748ULL != Info->Ptr) {
//      __builtin_trap();
//    }
//
//    // Do nothing in dumb case
//    if (new_size == Info->Size) {
//      return ptr;
//    }
//
//    mprotect(Info, 4096, PROT_READ | PROT_WRITE);
//
//    // Find the slab allocator with this memory range
//    auto CurrentSlab = Local.Slabs->begin();
//    while (*CurrentSlab != nullptr) {
//      uintptr_t SlabBegin = (*CurrentSlab)->UPtr;
//      uintptr_t SlabEnd = SlabBegin + SlabAllocator::SLAB_SIZE;
//      if (SlabBegin <= reinterpret_cast<uintptr_t>(ptr) &&
//          SlabEnd >= reinterpret_cast<uintptr_t>(ptr)) {
//
//        // Can we allocate forward?
//        size_t RealSize = Info->Size + sizeof(MetaDataInfo);
//        size_t NewRealSize = new_size + sizeof(MetaDataInfo);
//        size_t PageSize = AlignUp(RealSize, ::SlabAllocator::PAGE_SIZE) / ::SlabAllocator::PAGE_SIZE;
//        size_t NewPageSize = AlignUp(NewRealSize, ::SlabAllocator::PAGE_SIZE) / ::SlabAllocator::PAGE_SIZE;
//        size_t PageOffset = (reinterpret_cast<uintptr_t>(ptr) - SlabBegin) / ::SlabAllocator::PAGE_SIZE;
//
//        bool CanFit{true};
//        for (size_t i = PageOffset + PageSize;
//             i < ::SlabAllocator::BITSET_SIZE && i < (PageOffset + NewPageSize);
//             ++i) {
//          if ((*CurrentSlab)->UsedPages[i]) {
//            CanFit = false;
//            break;
//          }
//        }
//
//        if (CanFit) {
//          // XXX: Support
//          //__builtin_trap();
//          break;
//        }
//      }
//      ++CurrentSlab;
//    }
//
//    // Careful dance around new allocations
//    void *NewAlloc = this->malloc(new_size);
//    memcpy(NewAlloc, ptr, Info->Size);
//    this->free(ptr);
//    return NewAlloc;
//  }
//
//  std::unique_ptr<Allocator> Alloc;
//
//  void *my_malloc(size_t size) {
//    return Alloc->malloc(size);
//  }
//
//  void *my_realloc(void *ptr, size_t size) {
//    return Alloc->realloc(ptr, size);
//  }
//
//  void *my_memalign(size_t alignment, size_t size) {
//    __builtin_trap();
//  }
//
//  void my_free(void *ptr) {
//    Alloc->free(ptr);
//  }
//
//  void InitializeAllocator() {
//    Slab = std::make_unique<SlabAllocator::x64::SlabAllocator>();
//    Alloc = std::make_unique<Allocator>();
//    current_malloc = my_malloc;
//    current_realloc = my_realloc;
//    current_memalign = my_memalign;
//    current_free = my_free;
//  }
//}
//#endif
