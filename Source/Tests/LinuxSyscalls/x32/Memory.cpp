#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

#include "Common/MathUtils.h"
#include <FEXCore/Utils/LogManager.h>

#include <bitset>
#include <map>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace FEX::HLE::x32 {
class MemAllocator {
private:
  static constexpr uint64_t PAGE_SHIFT = 12;
  static constexpr uint64_t PAGE_SIZE = 1 << PAGE_SHIFT;
  static constexpr uint64_t PAGE_MASK = (1 << PAGE_SHIFT) - 1;
  static constexpr uint64_t BASE_KEY = 16;
  const uint64_t TOP_KEY = 0xFFFF'F000ULL >> PAGE_SHIFT;

public:
  MemAllocator() {
    // First 16 pages are taken by the Linux kernel
    for (size_t i = 0; i < 16; ++i) {
      MappedPages.set(i);
    }
    // Take the top page as well
    MappedPages.set(TOP_KEY);
    if (SearchDown) {
      LastScanLocation = TOP_KEY;
      LastKeyLocation = TOP_KEY;
      FindPageRangePtr = &MemAllocator::FindPageRange_TopDown;
    }
    else {
      LastScanLocation = BASE_KEY;
      LastKeyLocation = BASE_KEY;
      FindPageRangePtr = &MemAllocator::FindPageRange;
    }
  }
  void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
  int munmap(void *addr, size_t length);
  void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, void *new_address);
  uint64_t Shmat(int shmid, const void* shmaddr, int shmflg, uint32_t *ResultAddress);
  uint64_t Shmdt(const void* shmaddr);
  static constexpr bool SearchDown = true;

  // PageAddr is a page already shifted to page index
  // PagesLength is the number of pages
  void SetUsedPages(uint64_t PageAddr, size_t PagesLength) {
    // Set the range as mapped
    for (size_t i = 0; i < PagesLength; ++i) {
      MappedPages.set(PageAddr + i);
    }
  }

  // PageAddr is a page already shifted to page index
  // PagesLength is the number of pages
  void SetFreePages(uint64_t PageAddr, size_t PagesLength) {
    // Set the range as unused
    for (size_t i = 0; i < PagesLength; ++i) {
      MappedPages.reset(PageAddr + i);
    }
  }

private:
  // Set that contains 4k mapped pages
  // This is the full 32bit memory range
  std::bitset<0x10'0000> MappedPages;
  std::map<uint32_t, int> PageToShm{};
  uint64_t LastScanLocation{};
  uint64_t LastKeyLocation{};
  std::mutex AllocMutex{};
  uint64_t FindPageRange(uint64_t Start, size_t Pages);
  uint64_t FindPageRange_TopDown(uint64_t Start, size_t Pages);
  using FindHandler = uint64_t(MemAllocator::*)(uint64_t Start, size_t Pages);
  FindHandler FindPageRangePtr{};
};

uint64_t MemAllocator::FindPageRange(uint64_t Start, size_t Pages) {
  // Linear range scan
  while (Start != TOP_KEY) {
    bool Free = true;
    if ((Start + Pages) > TOP_KEY) {
      return 0;
    }
    uint64_t Offset = 0;
    for (; Offset < Pages; ++Offset) {
      if (MappedPages.test(Start + Offset)) {
        Free = false;
        break;
      }
    }

    if (Free) {
      return Start;
    }
    Start += Offset + 1;
  }

  return 0;
}

uint64_t MemAllocator::FindPageRange_TopDown(uint64_t Start, size_t Pages) {
  // Linear range scan
  while (Start >= BASE_KEY &&
         Start <= TOP_KEY) {
    bool Free = true;

    uint64_t Offset = 0;
    for (; Offset < Pages; ++Offset) {
      if (MappedPages.test(Start - Offset)) {
        Free = false;
        break;
      }
    }

    if (Free) {
      //LogMan::Msg::D("Allocated pages at: [0x%lx, 0x%lx)", (Start - Offset) << PAGE_SHIFT, (Start - Offset + Pages) << PAGE_SHIFT);
      return Start - Offset;
    }
    Start -= Offset + 1;
  }

  return 0;
}

void *MemAllocator::mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
  std::scoped_lock<std::mutex> lk{AllocMutex};
  size_t PagesLength = AlignUp(length, PAGE_SIZE) >> PAGE_SHIFT;

  uintptr_t Addr = reinterpret_cast<uintptr_t>(addr);
  uintptr_t PageAddr = Addr >> PAGE_SHIFT;

  uintptr_t PageEnd = PageAddr + PagesLength;

  bool Fixed = ((flags & MAP_FIXED) ||
      (flags & MAP_FIXED_NOREPLACE));

  // Both Addr and length must be page aligned
  if (Addr & PAGE_MASK) {
    return (void*)-EINVAL;
  }

  // If we do have an fd then offset must be page aligned
  if (fd != -1 &&
      offset & PAGE_MASK) {
    return (void*)-EINVAL;
  }

  if (Addr + length > std::numeric_limits<uint32_t>::max()) {
    return (void*)-EOVERFLOW;
  }

  // Check reserved range
  if (Fixed && PageAddr < 16) {
    return (void*)-EINVAL;
  }

  if (!Fixed) {
    // If we aren't mapping fixed the ignore the address input
    Addr = 0;
    PageAddr = 0;
    PageEnd = PagesLength;
  }

  // Find a region that fits our address
  if (Addr == 0) {
    bool Wrapped = false;
    uint64_t BottomPage = LastScanLocation;
restart:
    {
      // Linear range scan
      uint64_t LowerPage = (this->*FindPageRangePtr)(BottomPage, PagesLength);
      if (LowerPage == 0) {
        // Try again but this time from the start
        BottomPage = LastKeyLocation;
        LowerPage = (this->*FindPageRangePtr)(BottomPage, PagesLength);
      }

      uint64_t UpperPage = LowerPage + PagesLength;
      if (LowerPage == 0) {
        LogMan::Msg::E("@@@@@ mmap no mem!");
        return (void*)(uintptr_t)-ENOMEM;
      }
      {
        // Try and map the range
        void *MappedPtr = ::mmap(
          reinterpret_cast<void*>(LowerPage<< PAGE_SHIFT),
          length,
          prot,
          flags | MAP_FIXED_NOREPLACE,
          fd,
          offset);

        if (MappedPtr == MAP_FAILED) {
          if (UpperPage == TOP_KEY) {
            BottomPage = BASE_KEY;
            Wrapped = true;
            goto restart;
          }
          else if (Wrapped &&
            LowerPage >= LastScanLocation) {
            // We linear scanned the entire memory range. Give up
            return (void*)(uintptr_t)-errno;
          }
          else {
            // Try again
            if (SearchDown) {
              BottomPage -= PagesLength;
            }
            else {
              BottomPage += PagesLength;
            }
            goto restart;
          }
        }
        else {
          if (SearchDown) {
            LastScanLocation = LowerPage;
          }
          else {
            LastScanLocation = UpperPage;
          }
          SetUsedPages(LowerPage, PagesLength);
          return MappedPtr;
        }
      }
    }
  }
  else {
    void *MappedPtr = ::mmap(
      reinterpret_cast<void*>(PageAddr << PAGE_SHIFT),
      PagesLength << PAGE_SHIFT,
      prot,
      flags,
      fd,
      offset);

    if (MappedPtr != MAP_FAILED) {
      SetUsedPages(PageAddr, PagesLength);
      return MappedPtr;
    }
    else {
      return (void*)(uintptr_t)-errno;
    }
  }
  return 0;
}

int MemAllocator::munmap(void *addr, size_t length) {
  std::scoped_lock<std::mutex> lk{AllocMutex};
  size_t PagesLength = AlignUp(length, PAGE_SIZE) >> PAGE_SHIFT;

  uintptr_t Addr = reinterpret_cast<uintptr_t>(addr);
  uintptr_t PageAddr = Addr >> PAGE_SHIFT;

  uintptr_t PageEnd = PageAddr + PagesLength;

  // Both Addr and length must be page aligned
  if (Addr & PAGE_MASK) {
    return -EINVAL;
  }

  if (length & PAGE_MASK) {
    return -EINVAL;
  }

  if (Addr + length > std::numeric_limits<uint32_t>::max()) {
    return -EOVERFLOW;
  }

  // Check reserved range
  if (PageAddr < 16) {
    // Return success for these
    return 0;
  }

  while (PageAddr != PageEnd) {
    // Always pass to munmap, it may be something allocated we aren't tracking
    int Result = ::munmap(reinterpret_cast<void*>(PageAddr << PAGE_SHIFT), PAGE_SIZE);
    if (Result != 0) {
      return -errno;
    }

    if (MappedPages.test(PageAddr)) {
      MappedPages.reset(PageAddr);
    }

    ++PageAddr;
  }

  return 0;
}

void *MemAllocator::mremap(void *old_address, size_t old_size, size_t new_size, int flags, void *new_address) {
  size_t OldPagesLength = AlignUp(old_size, PAGE_SIZE) >> PAGE_SHIFT;
  size_t NewPagesLength = AlignUp(new_size, PAGE_SIZE) >> PAGE_SHIFT;

  {
    std::scoped_lock<std::mutex> lk{AllocMutex};
    if (flags & MREMAP_FIXED) {
      void *MappedPtr = ::mremap(old_address, old_size, new_size, flags, new_address);

      if (MappedPtr != MAP_FAILED) {
        if (!(flags & MREMAP_DONTUNMAP)) {
          // Unmap the old location
          uintptr_t OldAddr = reinterpret_cast<uintptr_t>(old_address);
          SetFreePages(OldAddr >> PAGE_SHIFT, OldPagesLength);
        }

        // Map the new pages
        uintptr_t NewAddr = reinterpret_cast<uintptr_t>(MappedPtr);
        SetUsedPages(NewAddr >> PAGE_SHIFT, NewPagesLength);
      }
      else {
        LogMan::Msg::E("@@@@@ %d mremap failed! '%s'", __LINE__, strerror(-errno));
        return (void*)(uintptr_t)-errno;
      }
    }
    else {
      uintptr_t OldAddr = reinterpret_cast<uintptr_t>(old_address);
      uintptr_t OldPageAddr = OldAddr >> PAGE_SHIFT;

      if (NewPagesLength < OldPagesLength) {
        void *MappedPtr = ::mremap(old_address, old_size, new_size, flags & ~MREMAP_MAYMOVE);

        if (MappedPtr != MAP_FAILED) {
          // Clear the pages that we just shrunk
          size_t NewPagesLength = AlignUp(new_size, PAGE_SIZE) >> PAGE_SHIFT;
          uintptr_t NewPageAddr = reinterpret_cast<uintptr_t>(MappedPtr) >> PAGE_SHIFT;
          SetFreePages(NewPageAddr + NewPagesLength, OldPagesLength - NewPagesLength);
          return MappedPtr;
        }
        else {
          LogMan::Msg::E("@@@@@ %d mremap failed! '%s'", __LINE__, strerror(-errno));
          return (void*)(uintptr_t)-errno;
        }
      }
      else {
        // Scan the region forward from our first region's endd to see if it can be extended
        bool CanExtend{true};

        for (size_t i = OldPagesLength; i < NewPagesLength; ++i) {
          if (MappedPages[OldPageAddr + i]) {
            CanExtend = false;
            break;
          }
        }

        if (CanExtend) {
          void *MappedPtr = ::mremap(old_address, old_size, new_size, flags & ~MREMAP_MAYMOVE);

          if (MappedPtr != MAP_FAILED) {
            // Map the new pages
            size_t NewPagesLength = AlignUp(new_size, PAGE_SIZE) >> PAGE_SHIFT;
            uintptr_t NewAddr = reinterpret_cast<uintptr_t>(MappedPtr);
            SetUsedPages(NewAddr >> PAGE_SHIFT, NewPagesLength);
            return MappedPtr;
          }
          else if (!(flags & MREMAP_MAYMOVE)) {
            // We have one more chance if MAYMOVE is specified
            LogMan::Msg::E("@@@@@ %d mremap failed! '%s'", __LINE__, strerror(-errno));
            return (void*)(uintptr_t)-errno;
          }
        }
      }
    }
  }

  // Flags can not contain MREMAP_FIXED at this point
  // Flags might contain MREMAP_MAYMOVE and/or MREMAP_DONTUNMAP
  // New Size is >= old size

  // First, try and allocate a region the size of the new size
  void *MappedPtr = this->mmap(nullptr, new_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  std::scoped_lock<std::mutex> lk{AllocMutex};
  if (reinterpret_cast<uintptr_t>(MappedPtr) > -4096) {
    // Couldn't find a region that fit our space
    LogMan::Msg::E("@@@@@ %d mremap failed! '%s'", __LINE__, strerror(-(uint64_t)MappedPtr));
    return MappedPtr;
  }

  // Good news, we found a region
  // This will overwrite the previous mmap if it succeeds
  MappedPtr = ::mremap(old_address, old_size, new_size, flags | MREMAP_FIXED | MREMAP_MAYMOVE, MappedPtr);

  if (MappedPtr != MAP_FAILED) {
    if (!(flags & MREMAP_DONTUNMAP) &&
        MappedPtr != old_address) {
      // If we have both MREMAP_DONTUNMAP not set and the new pointer is at a new location
      // Make sure to clear the old mapping
      uintptr_t OldAddr = reinterpret_cast<uintptr_t>(old_address);
      SetFreePages(OldAddr >> PAGE_SHIFT , OldPagesLength);
    }

    // Map the new pages
    size_t NewPagesLength = AlignUp(new_size, PAGE_SIZE) >> PAGE_SHIFT;
    uintptr_t NewAddr = reinterpret_cast<uintptr_t>(MappedPtr);
    SetUsedPages(NewAddr >> PAGE_SHIFT, NewPagesLength);
    return MappedPtr;
  }
  LogMan::Msg::E("@@@@@ %d mremap failed! '%s'", __LINE__, strerror(-errno));

  // Failed
  return (void*)(uintptr_t)-errno;
}

uint64_t MemAllocator::Shmat(int shmid, const void* shmaddr, int shmflg, uint32_t *ResultAddress) {
  std::scoped_lock<std::mutex> lk{AllocMutex};

  if (shmaddr != nullptr) {
    // shmaddr must be valid
    uint64_t Result = reinterpret_cast<uint64_t>(shmat(shmid, shmaddr, shmflg));
    if (Result != -1) {
      uint32_t SmallRet = Result >> 32;
      if (!(SmallRet == 0 ||
            SmallRet == ~0U)) {
        LogMan::Msg::A("Syscall returning something with data in the upper 32bits! BUG!");
        return -ENOMEM;
      }

      uintptr_t NewAddr = reinterpret_cast<uintptr_t>(Result);
      uintptr_t NewPageAddr = NewAddr >> PAGE_SHIFT;

      // Add to the map
      PageToShm[NewPageAddr] = shmid;

      *ResultAddress = Result;

      // We must get the shm size and track it
      struct shmid_ds buf{};

      if (shmctl(shmid, IPC_STAT, &buf) == 0) {
        // Map the new pages
        size_t NewPagesLength = buf.shm_segsz >> PAGE_SHIFT;
        SetUsedPages(NewPageAddr, NewPagesLength);
      }

      // Zero on working result
      Result = 0;
    }
    else {
      Result = -errno;
    }
    return Result;
  }
  else {
    // We must get the shm size and track it
    struct shmid_ds buf{};
    uint64_t PagesLength{};

    if (shmctl(shmid, IPC_STAT, &buf) == 0) {
      PagesLength = AlignUp(buf.shm_segsz, PAGE_SIZE) >> PAGE_SHIFT;
    }
    else {
      return -EINVAL;
    }

    bool Wrapped = false;
    uint64_t BottomPage = LastScanLocation;
restart:
    {
      // Linear range scan
      uint64_t LowerPage = (this->*FindPageRangePtr)(BottomPage, PagesLength);
      if (LowerPage == 0) {
        // Try again but this time from the start
        BottomPage = LastKeyLocation;
        LowerPage = (this->*FindPageRangePtr)(BottomPage, PagesLength);
      }

      uint64_t UpperPage = LowerPage + PagesLength;
      if (LowerPage == 0) {
        LogMan::Msg::E("@@@@@ shmat no mem!");
        return -ENOMEM;
      }
      {
        // Try and map the range
        void *MappedPtr = ::shmat(
          shmid,
          reinterpret_cast<const void*>(LowerPage << PAGE_SHIFT),
          shmflg);

        if (MappedPtr == MAP_FAILED) {
          if (UpperPage == TOP_KEY) {
            BottomPage = LastKeyLocation;
            Wrapped = true;
            goto restart;
          }
          else if (Wrapped &&
            LowerPage >= LastScanLocation) {
            // We linear scanned the entire memory range. Give up
            return -errno;
          }
          else {
            // Try again
            BottomPage += PagesLength;
            goto restart;
          }
        }
        else {
          if (SearchDown) {
            LastScanLocation = LowerPage;
          }
          else {
            LastScanLocation = UpperPage;
          }
          // Set the range as mapped
          SetUsedPages(LowerPage, PagesLength);

          *ResultAddress = reinterpret_cast<uint64_t>(MappedPtr);

          // Add to the map
          PageToShm[LowerPage] = shmid;

          // Zero on working result
          return 0;
        }
      }
    }
  }
}
uint64_t MemAllocator::Shmdt(const void* shmaddr) {
  uint32_t AddrPage = reinterpret_cast<uint64_t>(shmaddr) >> PAGE_SHIFT;
  auto it = PageToShm.find(AddrPage);

  if (it == PageToShm.end()) {
    // Page wasn't mapped
    return -EINVAL;
  }

  uint64_t Result = ::shmdt(shmaddr);
  PageToShm.erase(it);
  return Result;
}

#define _H (uint32_t)(uint64_t)

  static std::unique_ptr<MemAllocator> alloc{};

uint64_t Shmat(int shmid, const void* shmaddr, int shmflg, uint32_t *ResultAddress) {
  return alloc->Shmat(shmid, shmaddr, shmflg, ResultAddress);
}

uint64_t Shmdt(const void* shmaddr) {
  return alloc->Shmdt(shmaddr);
}

  void RegisterMemory(bool Has64BitAllocator) {
    if (Has64BitAllocator) {
      REGISTER_SYSCALL_IMPL_X32(mmap, [](FEXCore::Core::InternalThreadState *Thread, uint32_t addr, uint32_t length, int prot, int flags, int fd, int32_t offset) -> uint64_t {
        uint64_t Result = reinterpret_cast<uint64_t>(::mmap(reinterpret_cast<void*>(addr), length, prot, flags, fd, offset));
        SYSCALL_ERRNO();
      });

      REGISTER_SYSCALL_IMPL_X32(mmap2, [](FEXCore::Core::InternalThreadState *Thread, uint32_t addr, uint32_t length, int prot, int flags, int fd, uint32_t pgoffset) -> uint64_t {
        uint64_t Result = reinterpret_cast<uint64_t>(::mmap(reinterpret_cast<void*>(addr), length, prot, flags, fd, static_cast<uint64_t>(pgoffset) * 0x1000));
        SYSCALL_ERRNO();
      });

      REGISTER_SYSCALL_IMPL_X32(munmap, [](FEXCore::Core::InternalThreadState *Thread, void *addr, size_t length) -> uint64_t {
        uint64_t Result = ::munmap(addr, length);
        SYSCALL_ERRNO();
      });

      REGISTER_SYSCALL_IMPL_X32(mremap, [](FEXCore::Core::InternalThreadState *Thread, void *old_address, size_t old_size, size_t new_size, int flags, void *new_address) -> uint64_t {
        uint64_t Result = reinterpret_cast<uint64_t>(::mremap(old_address, old_size, new_size, flags, new_address));
        SYSCALL_ERRNO();
      });
    }
    else {
      alloc = std::make_unique<MemAllocator>();
      REGISTER_SYSCALL_IMPL_X32(mmap, [](FEXCore::Core::InternalThreadState *Thread, uint32_t addr, uint32_t length, int prot, int flags, int fd, int32_t offset) -> uint64_t {
        return (uint64_t)alloc->mmap(reinterpret_cast<void*>(addr), length, prot,flags, fd, offset);
      });

      REGISTER_SYSCALL_IMPL_X32(mmap2, [](FEXCore::Core::InternalThreadState *Thread, uint32_t addr, uint32_t length, int prot, int flags, int fd, uint32_t pgoffset) -> uint64_t {
        return (uint64_t)alloc->mmap(reinterpret_cast<void*>(addr), length, prot,flags, fd, (uint64_t)pgoffset * 0x1000);
      });

      REGISTER_SYSCALL_IMPL_X32(munmap, [](FEXCore::Core::InternalThreadState *Thread, void *addr, size_t length) -> uint64_t {
        return alloc->munmap(addr, length);
      });

      REGISTER_SYSCALL_IMPL_X32(mremap, [](FEXCore::Core::InternalThreadState *Thread, void *old_address, size_t old_size, size_t new_size, int flags, void *new_address) -> uint64_t {
        return reinterpret_cast<uint64_t>(alloc->mremap(old_address, old_size, new_size, flags, new_address));
      });
    }

    REGISTER_SYSCALL_IMPL_X32(mprotect, [](FEXCore::Core::InternalThreadState *Thread, void *addr, uint32_t len, int prot) -> uint64_t {
      uint64_t Result = ::mprotect(addr, len, prot);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(mlockall, [](FEXCore::Core::InternalThreadState *Thread, int flags) -> uint64_t {
      uint64_t Result = ::mlock2(reinterpret_cast<void*>(0x1'0000), 0x1'0000'0000ULL - 0x1'0000, flags);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(munlockall, [](FEXCore::Core::InternalThreadState *Thread) -> uint64_t {
      uint64_t Result = ::munlock(reinterpret_cast<void*>(0x1'0000), 0x1'0000'0000ULL - 0x1'0000);
      SYSCALL_ERRNO();
    });
  }

}
