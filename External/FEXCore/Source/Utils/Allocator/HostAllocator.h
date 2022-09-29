#pragma once

#include <FEXCore/Utils/Allocator.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

namespace Alloc {
  // HostAllocator is just a page pased slab allocator
  // Similar to mmap and munmap only mapping at the page level
  class HostAllocator {
    public:
      HostAllocator() = default;
      virtual ~HostAllocator() = default;
      virtual void *AllocateSlab(size_t Size) = 0;
      virtual void DeallocateSlab(void *Ptr, size_t Size) = 0;

      virtual void *Mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) { return nullptr; }
      virtual int Munmap(void *addr, size_t length) { return -1; }
  };

  class GlobalAllocator {
    public:
      HostAllocator *Alloc{};
      GlobalAllocator(HostAllocator *_Alloc)
        : Alloc {_Alloc} {}

      virtual ~GlobalAllocator() = default;
      virtual void *malloc(size_t Size) = 0;
      virtual void *calloc(size_t num, size_t size) = 0;
      virtual void *realloc(void *ptr, size_t size) = 0;
      virtual void *memalign(size_t alignment, size_t size) = 0;
      virtual void free(void *ptr) = 0;
  };
}

namespace Alloc::OSAllocator {
std::unique_ptr<Alloc::HostAllocator> Create64BitAllocator();
std::unique_ptr<Alloc::HostAllocator> Create64BitAllocatorWithMemoryRegions(std::vector<FEXCore::Allocator::MemoryRegion> *MemoryRegions);
}
