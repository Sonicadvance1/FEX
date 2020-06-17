#include "Common/MathUtils.h"

#include "Interface/Context/Context.h"
#include "Interface/Core/InternalThreadState.h"
#include "Interface/Core/ELFMapper.h"
#include "Interface/HLE/Syscalls.h"
#include "Interface/HLE/x64/Syscalls.h"
#include "Interface/HLE/x32/Syscalls.h"

#include "LogManager.h"

#include <FEXCore/Core/X86Enums.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/futex.h>
#include <numaif.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/random.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <unistd.h>

constexpr uint64_t PAGE_SIZE = 4096;

namespace FEXCore {
uint64_t SyscallHandler::HandleBRK(FEXCore::Core::InternalThreadState *Thread, void *Addr) {
  std::lock_guard<std::mutex> lk(MMapMutex);
  uint64_t Result;
  if (Addr == nullptr) { // Just wants to get the location of the program break atm
    if (DataSpace == 0) {
      // XXX: We need to setup our default BRK space first
      DefaultProgramBreak(Thread, 0xd000'0000);
    }
    Result = DataSpace + DataSpaceSize;
  }
  else {
    // Allocating out data space
    uint64_t NewEnd = reinterpret_cast<uint64_t>(Addr);
    if (NewEnd < DataSpace) {
      // Not allowed to move brk end below original start
      // Set the size to zero
      DataSpaceSize = 0;
    }
    else {
      uint64_t NewSize = NewEnd - DataSpace;
      DataSpaceSize = NewSize;
    }
    Result = DataSpace + DataSpaceSize;
  }
  return Result;
}

uint64_t SyscallHandler::HandleMMAP(FEXCore::Core::InternalThreadState *Thread, void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
  if (flags & MAP_FIXED) {
    uint64_t Result = reinterpret_cast<uint64_t>(::mmap(addr, length, prot, flags, fd, offset));
    if (fd != -1 && offset == 0) {
      auto Name = FM.FindFDName(fd);
      if (Name && Result) {
        LogMan::Msg::D("Mapping File to [0x%lx, 0x%lx) -> '%s' -> %p", Result, Result + length, Name->c_str(), Result);
        FEXCore::ELFMapper::AddELFToList(Name, Result, length);
      }
    }
    SYSCALL_ERRNO();
  }
  else {
    uint64_t Result = reinterpret_cast<uint64_t>(::mmap(addr, length, prot, flags, fd, offset));
    if (fd != -1) {
      auto Name = FM.FindFDName(fd);
      if (Name && Result) {
        LogMan::Msg::D("Mapping File to [0x%lx, 0x%lx) -> '%s' -> %p", Result, Result + length, Name->c_str(), Result);
        FEXCore::ELFMapper::AddELFToList(Name, Result, length);
      }
    }

    SYSCALL_ERRNO();
  }
}

void SyscallHandler::DefaultProgramBreak(FEXCore::Core::InternalThreadState *Thread, uint64_t Addr) {
  DataSpaceSize = 0;
  // Just allocate 1GB of data memory past the default program break location at this point
  CTX->MapRegion(Thread, Addr, 0x1000'0000, true);

  Addr += CTX->MemoryMapper.GetBaseOffset<uint64_t>(0);

  DataSpace = Addr;
  DefaultProgramBreakAddress = Addr;
}

SyscallHandler::SyscallHandler(FEXCore::Context::Context *ctx)
  : FM {ctx}
  , CTX {ctx} {
}

SyscallHandler *CreateHandler(Context::OperatingMode Mode, FEXCore::Context::Context *ctx) {
  if (Mode == Context::MODE_64BIT) {
    return FEXCore::HLE::x64::CreateHandler(ctx);
  }
  else {
    return FEXCore::HLE::x32::CreateHandler(ctx);
  }
}

uint64_t HandleSyscall(SyscallHandler *Handler, FEXCore::Core::InternalThreadState *Thread, FEXCore::HLE::SyscallArguments *Args) {
  uint64_t Result{};
  Result = Handler->HandleSyscall(Thread, Args);
#ifdef DEBUG_STRACE
  Handler->Strace(Args, Result);
#endif
  return Result;
}

}
