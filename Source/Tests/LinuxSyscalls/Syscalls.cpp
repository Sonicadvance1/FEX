#include <FEXCore/Utils/LogManager.h>
#include "Common/MathUtils.h"

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

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

namespace FEX::HLE {
SyscallHandler *_SyscallHandler{};

uint64_t SyscallHandler::HandleBRK(FEXCore::Core::InternalThreadState *Thread, void *Addr) {
  std::lock_guard<std::mutex> lk(MMapMutex);

  uint64_t Result;

  if (Addr == nullptr) { // Just wants to get the location of the program break atm
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
      uint64_t NewSizeAligned = AlignUp(NewSize, 4096);

      if (NewSizeAligned < DataSpaceMaxSize) {
        // If we are shrinking the brk then munmap the ranges
        // That way we gain the memory back and also give the application zero pages if it allocates again
        // DataspaceMaxSize is always page aligned

        uint64_t RemainingSize = DataSpaceMaxSize - NewSizeAligned;
        // We have pages we can unmap
        munmap((void*)(DataSpace + NewSizeAligned), RemainingSize);
        DataSpaceMaxSize = NewSizeAligned;
      }
      else if (NewSize >= DataSpaceMaxSize) {
        constexpr static uint64_t SizeAlignment = 8 * 1024 * 1024;
        uint64_t AllocateNewSize = AlignUp(NewSize, SizeAlignment) - DataSpaceMaxSize;
        if (!Is64BitMode() &&
          (DataSpace + DataSpaceMaxSize + AllocateNewSize > 0x1'0000'0000ULL)) {
          // If we are 32bit and we tried going about the 32bit limit then out of memory
          return DataSpace + DataSpaceSize;
        }

        uint64_t NewBRK = (uint64_t)mmap((void*)(DataSpace + DataSpaceMaxSize), AllocateNewSize, PROT_READ | PROT_WRITE, MAP_FIXED_NOREPLACE | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (NewBRK == ~0ULL) {
          // If we couldn't allocate a new region then out of memory
          return DataSpace + DataSpaceSize;
        }
        else {
          // Increase our BRK size
          DataSpaceMaxSize += AllocateNewSize;
        }
      }

      DataSpaceSize = NewSize;
    }
    Result = DataSpace + DataSpaceSize;
  }
  return Result;
}

void SyscallHandler::DefaultProgramBreak(uint64_t Base, uint64_t Size) {
  if (Base == 0) {
    constexpr size_t BRK_SIZE = 8 * 1024 * 1024;
  //#define MAP_32BIT 0
    // XXX: If MAP_32BIT then steam breaks....?
    if (Is64BitMode()) {
      DataSpace = (uint64_t)mmap(nullptr, BRK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }
    else {
      DataSpace = (uint64_t)mmap((void*)0x1000, BRK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      LogMan::Msg::D("32bit BRK: 0x%lx'%lx", DataSpace >> 32, DataSpace & ~0U);
    }

    if (DataSpace == ~0ULL) {
      ERROR_AND_DIE("Couldn't allocate");
    }
    if (DataSpace >> 32 && !Is64BitMode()) {
      ERROR_AND_DIE("BRK in upper 32bits");
    }
    DataSpaceMaxSize = BRK_SIZE;
  }
  else {
    DataSpace = Base;
    DataSpaceMaxSize = Size;
    if (Is64BitMode()) {
    }
    else {
      LogMan::Msg::D("32bit BRK: 0x%lx'%lx", DataSpace >> 32, DataSpace & ~0U);
    }

  }
}

SyscallHandler::SyscallHandler(FEXCore::Context::Context *ctx, FEX::HLE::SignalDelegator *_SignalDelegation)
  : FM {ctx}
  , SignalDelegation {_SignalDelegation} {
  FEX::HLE::_SyscallHandler = this;
  CalculateHostKernelVersion();
}

void SyscallHandler::CalculateHostKernelVersion() {
  struct utsname buf{};
  if (uname(&buf) == -1) {
    return;
  }

  int32_t Major{};
  int32_t Minor{};
  int32_t Patch{};
  char Tmp{};
  std::istringstream ss{buf.release};
  ss >> Major;
  ss.read(&Tmp, 1);
  ss >> Minor;
  ss.read(&Tmp, 1);
  ss >> Patch;
  HostKernelVersion = (Major << 24) | (Minor << 16) | Patch;
}

uint64_t SyscallHandler::HandleSyscall(FEXCore::Core::InternalThreadState *Thread, FEXCore::HLE::SyscallArguments *Args) {
  auto &Def = Definitions[Args->Argument[0]];
  uint64_t Result{};
  switch (Def.NumArgs) {
  case 0: Result = std::invoke(Def.Ptr0, Thread); break;
  case 1: Result = std::invoke(Def.Ptr1, Thread, Args->Argument[1]); break;
  case 2: Result = std::invoke(Def.Ptr2, Thread, Args->Argument[1], Args->Argument[2]); break;
  case 3: Result = std::invoke(Def.Ptr3, Thread, Args->Argument[1], Args->Argument[2], Args->Argument[3]); break;
  case 4: Result = std::invoke(Def.Ptr4, Thread, Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4]); break;
  case 5: Result = std::invoke(Def.Ptr5, Thread, Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5]); break;
  case 6: Result = std::invoke(Def.Ptr6, Thread, Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5], Args->Argument[6]); break;
  // for missing syscalls
  case 255: return std::invoke(Def.Ptr1, Thread, Args->Argument[0]);
  default:
    LogMan::Msg::A("Unhandled syscall: %d", Args->Argument[0]);
    return -1;
  break;
  }
#ifdef DEBUG_STRACE
  Strace(Args, Result);
#endif
  return Result;
}

#ifdef DEBUG_STRACE
void SyscallHandler::Strace(FEXCore::HLE::SyscallArguments *Args, uint64_t Ret) {
  //if (Args->Argument[0] == FEX::HLE::x32::SYSCALL_x86_clock_gettime64 ||
  //    Args->Argument[0] == FEX::HLE::x32::SYSCALL_x86_time ||
  //    Args->Argument[0] == FEX::HLE::x32::SYSCALL_x86_socketcall ||
  //    Args->Argument[0] == FEX::HLE::x32::SYSCALL_x86__newselect ||
  //    Args->Argument[0] == FEX::HLE::x32::SYSCALL_x86_gettid ||
  //    Args->Argument[0] == FEX::HLE::x32::SYSCALL_x86_poll ||
  //    Args->Argument[0] == FEX::HLE::x32::SYSCALL_x86_ppoll ||
  //    0
  //    ) {
  //  return;
  //}
  auto &Def = Definitions[Args->Argument[0]];
  switch (Def.NumArgs) {
    case 0: LogMan::Msg::D(Def.StraceFmt.c_str(), Ret); break;
    case 1: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Ret); break;
    case 2: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Ret); break;
    case 3: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Ret); break;
    case 4: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Ret); break;
    case 5: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5], Ret); break;
    case 6: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5], Args->Argument[6], Ret); break;
    default: break;
  }
  if (Ret > -4096) {
    LogMan::Msg::D("\tError: %d '%s'", -Ret, strerror(-Ret));
  }
}
#endif

FEX::HLE::SyscallHandler *CreateHandler(FEXCore::Context::OperatingMode Mode,
  FEXCore::Context::Context *ctx,
  FEX::HLE::SignalDelegator *_SignalDelegation,
  FEXCore::CodeLoader *Loader) {

  FEX::HLE::SyscallHandler *Result{};
  if (Mode == FEXCore::Context::MODE_64BIT) {
    Result = FEX::HLE::x64::CreateHandler(ctx, _SignalDelegation);
  }
  else {
    Result = FEX::HLE::x32::CreateHandler(ctx, _SignalDelegation);
  }

  Result->SetCodeLoader(Loader);
  return Result;
}

}
