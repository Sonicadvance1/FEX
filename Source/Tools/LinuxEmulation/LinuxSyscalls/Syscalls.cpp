// SPDX-License-Identifier: MIT
/*
$info$
category: LinuxSyscalls ~ Linux syscall emulation, marshaling and passthrough
tags: LinuxSyscalls|common
desc: Glue logic, brk allocations
$end_info$
*/

#include "CodeLoader.h"

#include "Linux/Utils/ELFContainer.h"
#include "Linux/Utils/ELFParser.h"

#include "LinuxSyscalls/LinuxAllocator.h"
#include "LinuxSyscalls/SignalDelegator.h"
#include "LinuxSyscalls/Syscalls.h"
#include "LinuxSyscalls/Syscalls/Thread.h"
#include "LinuxSyscalls/Utils/Threads.h"
#include "LinuxSyscalls/x32/Syscalls.h"
#include "LinuxSyscalls/x64/Syscalls.h"
#include "LinuxSyscalls/x32/Types.h"
#include "LinuxSyscalls/x64/Types.h"

#include <FEXCore/Config/Config.h>
#include <FEXCore/Core/Context.h>
#include <FEXCore/Core/CoreState.h>
#include <FEXCore/Debug/InternalThreadState.h>
#include <FEXCore/HLE/Linux/ThreadManagement.h>
#include <FEXCore/HLE/SyscallHandler.h>
#include <FEXCore/Utils/Allocator.h>
#include <FEXCore/Utils/CompilerDefs.h>
#include <FEXCore/Utils/LogManager.h>
#include <FEXCore/Utils/MathUtils.h>
#include <FEXCore/Utils/FileLoading.h>
#include <FEXCore/fextl/fmt.h>
#include <FEXCore/fextl/sstream.h>
#include <FEXCore/fextl/string.h>
#include <FEXCore/fextl/vector.h>
#include <FEXHeaderUtils/Filesystem.h>
#include <FEXHeaderUtils/Syscalls.h>

#include <algorithm>
#include <alloca.h>
#include <charconv>
#include <functional>
#include <linux/audit.h>
#include <linux/seccomp.h>
#include <memory>
#include <regex>
#include <sched.h>
#include <span>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <signal.h>
#include <system_error>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace FEX::HLE {
class SignalDelegator;
SyscallHandler* _SyscallHandler {};

template<bool IncrementOffset, typename T>
uint64_t GetDentsEmulation(int fd, T* dirp, uint32_t count) {
  uint64_t Result = syscall(SYSCALL_DEF(getdents64), static_cast<uint64_t>(fd), dirp, static_cast<uint64_t>(count));

  // Now copy back in to the array we were given
  if (Result != -1) {
    // If the outgoing d_ino is smaller than the incoming d_ino from the kernel
    // Then we need to check for overflow before writing any of the data back
    if constexpr (sizeof(decltype(FEX::HLE::x64::linux_dirent_64::d_ino)) > sizeof(decltype(T::d_ino))) {
      uint64_t TmpOffset = 0;
      while (TmpOffset < Result) {
        FEX::HLE::x64::linux_dirent_64* Tmp = (FEX::HLE::x64::linux_dirent_64*)(reinterpret_cast<uint64_t>(dirp) + TmpOffset);
        decltype(T::d_ino) Result_d_ino = Tmp->d_ino;

        if (Result_d_ino != Tmp->d_ino) {
          // The resulting d_ino truncated, return error
          return -EOVERFLOW;
        }
        TmpOffset += Tmp->d_reclen;
      }
    }

    uint64_t Offset = 0;
    uint64_t TmpOffset = 0;
    size_t OffsetIndex = 1;
    // With how the emulation occurs we will always return a smaller buffer than what was given to us.
    // We need to be careful with the in-place translation that occurs here, the data returning to the guest is guaranteed to be smaller
    // than the data returned by getdents64.
    // This means FEX is guaranteed to /never/ fill the full getdents buffer to the guest, but we may temporarily use it all.
    while (TmpOffset < Result) {
      T* Outgoing = (T*)(reinterpret_cast<uint64_t>(dirp) + Offset);
      FEX::HLE::x64::linux_dirent_64* Tmp = (FEX::HLE::x64::linux_dirent_64*)(reinterpret_cast<uint64_t>(dirp) + TmpOffset);

      if (!Tmp->d_reclen) {
        break;
      }

      size_t NewRecLen = FEXCore::AlignUp(Tmp->d_reclen - (sizeof(std::remove_reference<decltype(*Tmp)>::type) - sizeof(*Outgoing)),
                                          alignof(decltype(Tmp->d_ino)));
      Outgoing->d_ino = Tmp->d_ino;

      // 32-bit getdents can't safely handle d_off
      // A safe way of emulating this is to just use an incrementing offset from 1
      Outgoing->d_off = IncrementOffset ? OffsetIndex : Tmp->d_off;
      size_t OffsetOfName = offsetof(std::remove_reference<decltype(*Tmp)>::type, d_name);
      Outgoing->d_reclen = NewRecLen;

      // Copies null character as well
      size_t NameLength = Tmp->d_reclen - OffsetOfName - 1;
      memmove(Outgoing->d_name, Tmp->d_name, NameLength);

      // Copy the hidden d_type flag
      Outgoing->d_name[Outgoing->d_reclen - offsetof(T, d_name) - 1] = Tmp->d_type;

      TmpOffset += Tmp->d_reclen;
      // Outgoing is 5 bytes smaller
      Offset += NewRecLen;

      ++OffsetIndex;
    }
    Result = Offset;
  }
  SYSCALL_ERRNO();
}

template uint64_t GetDentsEmulation<false>(int, FEX::HLE::x64::linux_dirent*, uint32_t);

template uint64_t GetDentsEmulation<true>(int, FEX::HLE::x32::linux_dirent_32*, uint32_t);

static bool IsShebangFile(std::span<char> Data) {
  // File isn't large enough to even contain a shebang.
  if (Data.size() <= 2) {
    return false;
  }

  // Handle shebang files.
  if (Data[0] == '#' && Data[1] == '!') {
    fextl::string InterpreterLine {Data.begin() + 2, // strip off "#!" prefix
                                   std::find(Data.begin(), Data.end(), '\n')};
    fextl::vector<fextl::string> ShebangArguments {};

    // Shebang line can have a single argument
    fextl::istringstream InterpreterSS(InterpreterLine);
    fextl::string Argument;
    while (std::getline(InterpreterSS, Argument, ' ')) {
      if (Argument.empty()) {
        continue;
      }
      ShebangArguments.push_back(std::move(Argument));
    }

    // Executable argument
    fextl::string& ShebangProgram = ShebangArguments[0];

    // If the filename is absolute then prepend the rootfs
    // If it is relative then don't append the rootfs
    if (ShebangProgram[0] == '/') {
      ShebangProgram = FEX::HLE::_SyscallHandler->RootFSPath() + ShebangProgram;
    }

    return FHU::Filesystem::Exists(ShebangProgram);
  }

  return false;
}

static bool IsShebangFD(int FD) {
  // We don't know the state of the FD coming in since this might be a guest tracked FD.
  // Need to be extra careful here not to adjust file offsets and status flags.
  //
  // Can't use dup since that makes the FD have the same file description backing both FDs.

  // The maximum length of the shebang line is `#!` + 255 chars
  std::array<char, 257> Header;
  const auto ChunkSize = 257l;
  const auto ReadSize = pread(FD, &Header.at(0), ChunkSize, 0);

  return IsShebangFile(std::span<char>(Header.data(), ReadSize));
}

static bool IsShebangFilename(const fextl::string& Filename) {
  // Open the Filename to determine if it is a shebang file.
  int FD = open(Filename.c_str(), O_RDONLY | O_CLOEXEC);
  if (FD == -1) {
    return false;
  }

  bool IsShebang = IsShebangFD(FD);
  close(FD);
  return IsShebang;
}

uint64_t ExecveHandler(FEXCore::Core::CpuStateFrame* Frame, const char* pathname, char* const* argv, char* const* envp, ExecveAtArgs Args) {
  auto SyscallHandler = FEX::HLE::_SyscallHandler;
  fextl::string Filename {};

  fextl::string RootFS = SyscallHandler->RootFSPath();
  ELFLoader::ELFContainer::ELFType Type {};

  // AT_EMPTY_PATH is only used if the pathname is empty.
  const bool IsFDExec = (Args.flags & AT_EMPTY_PATH) && strlen(pathname) == 0;
  const bool SupportsProcFSInterpreter = SyscallHandler->FM.SupportsProcFSInterpreterPath();
  fextl::vector<const char*> EnvpArgs {};
  char* const* EnvpPtr = envp;
  bool AlreadyCopiedEnvp {};
  fextl::string FDExecEnv;
  fextl::string FDSeccompEnv;
  fextl::string FDServerEnv;

  bool IsShebang {};

  if (IsFDExec) {
    Type = ELFLoader::ELFContainer::GetELFType(Args.dirfd);

    IsShebang = IsShebangFD(Args.dirfd);
  } else {
    // For absolute paths, check the rootfs first (if available)
    if (pathname[0] == '/') {
      auto Path = SyscallHandler->FM.GetEmulatedPath(pathname, true);
      if (!Path.empty() && FHU::Filesystem::Exists(Path)) {
        Filename = Path;
      } else {
        Filename = pathname;
      }
    } else {
      Filename = pathname;
    }

    bool exists = FHU::Filesystem::Exists(Filename);
    if (!exists) {
      return -ENOENT;
    }

    if (!SupportsProcFSInterpreter) {
      int pid = getpid();

      char PidSelfPath[50];
      snprintf(PidSelfPath, 50, "/proc/%i/exe", pid);

      if (strcmp(pathname, "/proc/self/exe") == 0 || strcmp(pathname, "/proc/thread-self/exe") == 0 || strcmp(pathname, PidSelfPath) == 0) {
        // If the application is trying to execve `/proc/self/exe` or its variants,
        // then we need to redirect this path to the true application path.
        // This is because this path is a symlink to the executing application, which is always `FEXInterpreter` or `FEXLoader`.
        // ex: JRE and shapez.io do this self-execution.
        Filename = SyscallHandler->Filename();
      }
    }

    Type = ELFLoader::ELFContainer::GetELFType(Filename);

    IsShebang = IsShebangFilename(Filename);
  }

  if (!IsShebang && Type == ELFLoader::ELFContainer::ELFType::TYPE_NONE) {
    // If our interpeter doesn't support this file format AND ELF format is NONE then ENOEXEC
    // binfmt_misc could end up handling this case but we can't know that without parsing binfmt_misc ourselves
    // Return -ENOEXEC until proven otherwise
    return -ENOEXEC;
  }

  auto SeccompFD = SyscallHandler->SeccompEmulator.SerializeFilters(Frame);
  const auto HasSeccomp = SeccompFD.has_value() && *SeccompFD != -1;
  auto AppendEnvpVariables = [&EnvpArgs, &AlreadyCopiedEnvp, envp, &EnvpPtr]() {
    if (AlreadyCopiedEnvp) {
      return;
    }

    if (envp) {
      auto OldEnvp = envp;
      while (*OldEnvp) {
        EnvpArgs.emplace_back(*OldEnvp);
        ++OldEnvp;
      }
    }
    EnvpPtr = const_cast<char* const*>(EnvpArgs.data());

    AlreadyCopiedEnvp = true;
  };

  auto AppendFinalEnvp = [&EnvpArgs, &AlreadyCopiedEnvp]() {
    if (!AlreadyCopiedEnvp) {
      return;
    }

    // Emplace nullptr at the end to stop
    EnvpArgs.emplace_back(nullptr);
  };

  auto CloseSeccompFD = [&HasSeccomp, &SeccompFD]() {
    if (HasSeccomp) {
      close(*SeccompFD);
    }
  };

  if (HasSeccomp) {
    AppendEnvpVariables();

    // Create the environment variable to pass the FD to our FEX.
    // Needs to stick around until execveat completes.
    FDSeccompEnv = fextl::fmt::format("FEX_SECCOMPFD={}", *SeccompFD);

    // Insert the FD for FEX to track.
    EnvpArgs.emplace_back(FDSeccompEnv.data());
  }

  fextl::fmt::print("Execve, do we have new net? {}\n", FEX::HLE::_SyscallHandler->GetFEXServerFromFD() ? "Yes" : "No");
  if (FEX::HLE::_SyscallHandler->GetFEXServerFromFD()) {
    LogMan::Msg::DFmt("NewNet execve");
    AppendEnvpVariables();

    // If the application set up a new network namespace then pass FEXServer's FD through.
    // Duplicate the FD since this socket is opened with SOCK_CLOEXEC.
    FDServerEnv = fextl::fmt::format("FEX_SERVERFD={}", dup(FEXServerClient::RequestNewConnectionFD(FEXServerClient::GetServerFD())));

    // Insert the FD for FEX to track.
    EnvpArgs.emplace_back(FDServerEnv.data());
  }

  // If we don't have the interpreter installed we need to be extra careful for ENOEXEC
  // Reasoning is that if we try executing a file from FEXLoader then this process loses the ENOEXEC flag
  // Kernel does its own checks for file format support for this
  // We can only call execve directly if we both have an interpreter installed AND were ran with the interpreter
  // If the user ran FEX through FEXLoader then we must go down the emulated path
  uint64_t Result {};
  if (SyscallHandler->IsInterpreterInstalled() && SyscallHandler->IsInterpreter() &&
      (Type == ELFLoader::ELFContainer::ELFType::TYPE_X86_32 || Type == ELFLoader::ELFContainer::ELFType::TYPE_X86_64)) {
    AppendFinalEnvp();
    // If the FEX interpreter is installed then just execve the ELF file
    // This will stay inside of our emulated environment since binfmt_misc will capture it
    Result = ::syscall(SYS_execveat, Args.dirfd, Filename.c_str(), argv, EnvpPtr, Args.flags);
    CloseSeccompFD();
    SYSCALL_ERRNO();
  }

  if (Type == ELFLoader::ELFContainer::ELFType::TYPE_OTHER_ELF) {
    AppendFinalEnvp();
    // We are trying to execute an ELF of a different architecture
    // We can't know if we can support this without architecture specific checks and binfmt_misc parsing
    // Just execve it and let the kernel handle the process
    Result = ::syscall(SYS_execveat, Args.dirfd, Filename.c_str(), argv, EnvpPtr, Args.flags);
    CloseSeccompFD();
    SYSCALL_ERRNO();
  }

  // We don't have an interpreter installed or we are executing a non-ELF executable
  // We now need to munge the arguments
  fextl::vector<const char*> ExecveArgs {};
  const char NullString[] = "";
  SyscallHandler->GetCodeLoader()->GetExecveArguments(&ExecveArgs);
  if (!SyscallHandler->IsInterpreter()) {
    // If we were launched from FEXLoader then we need to make sure to split arguments from FEXLoader and guest
    ExecveArgs.emplace_back("--");
  }

  if (argv) {
    // Overwrite the filename with the new one we are redirecting to
    ExecveArgs.emplace_back(Filename.c_str());

    auto OldArgv = argv;

    // It is valid to provide nullptr first argument.
    if (*OldArgv) {
      // Skip filename argument
      ++OldArgv;
      while (*OldArgv) {
        // Append the arguments together
        ExecveArgs.emplace_back(*OldArgv);
        ++OldArgv;
      }
    } else {
      // Linux kernel will stick an empty argument in to the argv list if none are provided.
      ExecveArgs.emplace_back(NullString);
    }

    // Emplace nullptr at the end to stop
    ExecveArgs.emplace_back(nullptr);
  }

  bool IsFDCopy {};
  if (IsFDExec) {
    AppendEnvpVariables();

    int Flags = fcntl(Args.dirfd, F_GETFD);
    if (Flags & FD_CLOEXEC) {
      // FEX needs the FD to live past execve when binfmt_misc isn't used,
      // so duplicate the FD if FD_CLOEXEC is set
      Args.dirfd = dup(Args.dirfd);
      IsFDCopy = true;
    }

    // Remove AT_EMPTY_PATH flag now.
    // We need to emulate this flag with `FEX_EXECVEFD` environment variable.
    // If we passed this flag through to the real `execveat` then the target FD wouldn't get emulated by FEX.
    Args.flags &= ~AT_EMPTY_PATH;

    // Create the environment variable to pass the FD to our FEX.
    // Needs to stick around until execveat completes.
    FDExecEnv = fextl::fmt::format("FEX_EXECVEFD={}", Args.dirfd);

    // Insert the FD for FEX to track.
    EnvpArgs.emplace_back(FDExecEnv.data());
  }

  // Emplace nullptr at the end to stop
  AppendFinalEnvp();

  const char* InterpreterPath = SupportsProcFSInterpreter ? "/proc/self/interpreter" : "/proc/self/exe";
  Result = ::syscall(SYS_execveat, Args.dirfd, InterpreterPath, const_cast<char* const*>(ExecveArgs.data()), EnvpPtr, Args.flags);
  CloseSeccompFD();
  if (IsFDCopy) {
    ///< Had to make a copy, close it now.
    close(Args.dirfd);
  }

  SYSCALL_ERRNO();
}

static bool AnyFlagsSet(uint64_t Flags, uint64_t Mask) {
  return (Flags & Mask) != 0;
}

static bool AllFlagsSet(uint64_t Flags, uint64_t Mask) {
  return (Flags & Mask) == Mask;
}

struct StackFrameData {
  FEX::HLE::ThreadStateObject* Thread {};
  FEXCore::Context::Context* CTX {};
  FEXCore::Core::CpuStateFrame NewFrame {};
  FEX::HLE::clone3_args GuestArgs {};
  bool NewNetNamespace{};
  int NewServerFD{};
};

struct StackFramePlusRet {
  uint64_t Ret;
  StackFrameData Data;
  uint64_t Pad;
};

[[noreturn]]
static void CloneBody(StackFrameData* Data, bool NeedsDataFree) {
  if (Data->NewNetNamespace) {
    LogMan::Msg::DFmt("Clone3: NewClone after ServerFD\n");
    close(FEXServerClient::GetServerFD());
    FEXServerClient::SetServerFD(Data->NewServerFD);
    FEX::HLE::_SyscallHandler->SetFEXServerFromFD();
  }

  uint64_t Result = FEX::HLE::HandleNewClone(Data->Thread, Data->CTX, &Data->NewFrame, &Data->GuestArgs);
  auto Stack = Data->GuestArgs.NewStack;
  if (NeedsDataFree) {
    FEXCore::Allocator::free(Data);
  }

  FEX::LinuxEmulation::Threads::DeallocateStackObjectAndExit(Stack, Result);
  FEX_UNREACHABLE;
}

[[noreturn]]
static void Clone3HandlerRet() {
  StackFrameData* Data = (StackFrameData*)alloca(0);
  CloneBody(Data, false);
}

static int Clone2HandlerRet(void* arg) {
  StackFrameData* Data = (StackFrameData*)arg;
  CloneBody(Data, true);
}

// Clone3 flags
#ifndef CLONE_CLEAR_SIGHAND
#define CLONE_CLEAR_SIGHAND 0x100000000ULL
#endif
#ifndef CLONE_INTO_CGROUP
#define CLONE_INTO_CGROUP 0x200000000ULL
#endif
#ifndef CLONE_NEWTIME
// Overlaps CSIGNAL, can only be used with clone3 and not clone2
#define CLONE_NEWTIME 0x00000080ULL
#endif

static void PrintFlags(uint64_t Flags) {
#define FLAGPRINT(x, y) \
  if (Flags & (y)) LogMan::Msg::IFmt("\tFlag: " #x)
  FLAGPRINT(CSIGNAL, 0x000000FF);
  FLAGPRINT(CLONE_VM, 0x00000100);
  FLAGPRINT(CLONE_FS, 0x00000200);
  FLAGPRINT(CLONE_FILES, 0x00000400);
  FLAGPRINT(CLONE_SIGHAND, 0x00000800);
  FLAGPRINT(CLONE_PTRACE, 0x00002000);
  FLAGPRINT(CLONE_VFORK, 0x00004000);
  FLAGPRINT(CLONE_PARENT, 0x00008000);
  FLAGPRINT(CLONE_THREAD, 0x00010000);
  FLAGPRINT(CLONE_NEWNS, 0x00020000);
  FLAGPRINT(CLONE_SYSVSEM, 0x00040000);
  FLAGPRINT(CLONE_SETTLS, 0x00080000);
  FLAGPRINT(CLONE_PARENT_SETTID, 0x00100000);
  FLAGPRINT(CLONE_CHILD_CLEARTID, 0x00200000);
  FLAGPRINT(CLONE_DETACHED, 0x00400000);
  FLAGPRINT(CLONE_UNTRACED, 0x00800000);
  FLAGPRINT(CLONE_CHILD_SETTID, 0x01000000);
  FLAGPRINT(CLONE_NEWCGROUP, 0x02000000);
  FLAGPRINT(CLONE_NEWUTS, 0x04000000);
  FLAGPRINT(CLONE_NEWIPC, 0x08000000);
  FLAGPRINT(CLONE_NEWUSER, 0x10000000);
  FLAGPRINT(CLONE_NEWPID, 0x20000000);
  FLAGPRINT(CLONE_NEWNET, 0x40000000);
  FLAGPRINT(CLONE_IO, 0x80000000);
  FLAGPRINT(CLONE_PIDFD, 0x00001000);
#undef FLAGPRINT
};

static uint64_t Clone2Handler(FEXCore::Core::CpuStateFrame* Frame, FEX::HLE::clone3_args* args, int *NewServerFD) {
  StackFrameData* Data = (StackFrameData*)FEXCore::Allocator::malloc(sizeof(StackFrameData));
  Data->Thread = static_cast<FEX::HLE::ThreadStateObject*>(Frame->Thread->FrontendPtr);
  Data->CTX = Frame->Thread->CTX;
  Data->GuestArgs = *args;

  Data->NewNetNamespace = (args->args.flags & CLONE_NEWNET) == CLONE_NEWNET;
  if (Data->NewNetNamespace) {
    // If the clone is setting up a new network namespace then it won't be able to find FEXServer on execve
    // Flag that we are under a NEWNET namespace so if execve happens we can inherit the FD.
    LogMan::Msg::DFmt("NewNet, Enabling net namespace handling");
    Data->NewServerFD = FEXServerClient::RequestNewConnectionFD(FEXServerClient::GetServerFD());
    *NewServerFD = Data->NewServerFD;
  }

  // Create a copy of the parent frame
  memcpy(&Data->NewFrame, Frame, sizeof(FEXCore::Core::CpuStateFrame));

  // Remove flags that will break us
  constexpr uint64_t INVALID_FOR_HOST = CLONE_SETTLS;
  uint64_t Flags = args->args.flags & ~INVALID_FOR_HOST;
  uint64_t Result = ::clone(Clone2HandlerRet,                                    // To be called function
                            (void*)((uint64_t)args->NewStack + args->StackSize), // Stack
                            Flags,                                               // Flags
                            Data,                                                // Argument
                            (pid_t*)args->args.parent_tid,                       // parent_tid
                            0,                                                   // XXX: What is correct for this? tls
                            (pid_t*)args->args.child_tid);                       // child_tid

  // Only parent will get here
  SYSCALL_ERRNO();
}

static uint64_t Clone3Handler(FEXCore::Core::CpuStateFrame* Frame, FEX::HLE::clone3_args* args, int *NewServerFD) {
  constexpr size_t Offset = sizeof(StackFramePlusRet);
  StackFramePlusRet* Data = (StackFramePlusRet*)(reinterpret_cast<uint64_t>(args->NewStack) + args->StackSize - Offset);
  Data->Ret = (uint64_t)Clone3HandlerRet;
  Data->Data.Thread = static_cast<FEX::HLE::ThreadStateObject*>(Frame->Thread->FrontendPtr);
  Data->Data.CTX = Frame->Thread->CTX;
  Data->Data.GuestArgs = *args;

  Data->Data.NewNetNamespace = (args->args.flags & CLONE_NEWNET) == CLONE_NEWNET;
  if (Data->Data.NewNetNamespace) {
    // If the clone is setting up a new network namespace then it won't be able to find FEXServer on execve
    // Flag that we are under a NEWNET namespace so if execve happens we can inherit the FD.
    LogMan::Msg::DFmt("NewNet, Enabling net namespace handling");
    Data->Data.NewServerFD = FEXServerClient::RequestNewConnectionFD(FEXServerClient::GetServerFD());
    *NewServerFD = Data->Data.NewServerFD;
  }

  FEX::HLE::kernel_clone3_args HostArgs {};
  HostArgs.flags = args->args.flags;
  HostArgs.pidfd = args->args.pidfd;
  HostArgs.child_tid = args->args.child_tid;
  HostArgs.parent_tid = args->args.parent_tid;
  HostArgs.exit_signal = args->args.exit_signal;
  // Host stack is always created
  HostArgs.stack = reinterpret_cast<uint64_t>(args->NewStack);
  HostArgs.stack_size = args->StackSize - Offset; // Needs to be 16 byte aligned
  HostArgs.tls = 0;                               // XXX: What is correct for this?
  HostArgs.set_tid = args->args.set_tid;
  HostArgs.set_tid_size = args->args.set_tid_size;
  HostArgs.cgroup = args->args.cgroup;

  // Create a copy of the parent frame
  memcpy(&Data->Data.NewFrame, Frame, sizeof(FEXCore::Core::CpuStateFrame));
  uint64_t Result = ::syscall(SYSCALL_DEF(clone3), &HostArgs, sizeof(HostArgs));

  // Only parent will get here
  SYSCALL_ERRNO();
};

uint64_t CloneHandler(FEXCore::Core::CpuStateFrame* Frame, FEX::HLE::clone3_args* args) {
  uint64_t flags = args->args.flags;

  if (flags & CLONE_CLEAR_SIGHAND) {
    // CLONE_CLEAR_SIGHAND was added in kernel 5.5. FEX doesn't properly support this.
    // glibc started using this flag in 2.38 as an optimization for posix_spawn.
    // If clone returns EINVAL or ENOSYS then it will fallback to the non-optimized path.
    LogMan::Msg::IFmt("CLONE_CLEAR_SIGHAND passed to clone3. Returning EINVAL.");
    return -EINVAL;
  }

  auto HasUnhandledFlags = [](FEX::HLE::clone3_args* args) -> bool {
    constexpr uint64_t UNHANDLED_FLAGS = CLONE_NEWNS |
                                         // CLONE_UNTRACED |
                                         CLONE_NEWCGROUP | CLONE_NEWUTS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWUSER | CLONE_NEWPID |
                                         CLONE_NEWNET | CLONE_IO | CLONE_CLEAR_SIGHAND | CLONE_INTO_CGROUP;

    if ((args->args.flags & UNHANDLED_FLAGS) != 0) {
      // Basic unhandled flags
      return true;
    }

    if (args->args.set_tid_size > 0) {
      // set_tid isn't exposed through anything other than clone3
      return true;
    }

    if (args->Type == TypeOfClone::TYPE_CLONE3) {
      if (AnyFlagsSet(args->args.flags, CLONE_NEWTIME)) {
        // New time namespace overlaps with CSIGNAL, only available in clone3
        return true;
      }
    }

    if (AnyFlagsSet(args->args.flags, CLONE_THREAD)) {
      if (!AllFlagsSet(args->args.flags, CLONE_SYSVSEM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND)) {
        LogMan::Msg::IFmt("clone: CLONE_THREAD: Unsupported flags w/ CLONE_THREAD (Shared Resources), {:X}", args->args.flags);
        return false;
      }
    } else {
      if (AnyFlagsSet(args->args.flags, CLONE_SYSVSEM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM)) {
        // CLONE_VM is particularly nasty here
        // Memory regions at the point of clone(More similar to a fork) are shared
        LogMan::Msg::IFmt("clone: Unsupported flags w/o CLONE_THREAD (Shared Resources), {:X}", args->args.flags);
        return false;
      }
    }

    // We support everything here
    return false;
  };

  if (flags & CLONE_VM) {
    Frame->Thread->CTX->MarkMemoryShared(Frame->Thread);
  }

  // If there are flags that can't be handled regularly then we need to hand off to the true clone handler
  if (HasUnhandledFlags(args)) {
    if (!AnyFlagsSet(flags, CLONE_THREAD)) {
      // Has an unsupported flag
      // Fall to a handler that can handle this case

      args->SignalMask = ~0ULL;
      ::syscall(SYS_rt_sigprocmask, SIG_SETMASK, &args->SignalMask, &args->SignalMask, sizeof(args->SignalMask));

      // Need to create a stack for the host thread.
      // LockBeforeFork grabs the allocator mutex to block allocations temporarily, so this must be allocated before
      args->StackSize = FEX::LinuxEmulation::Threads::STACK_SIZE;
      args->NewStack = FEX::LinuxEmulation::Threads::AllocateStackObject();

      FEX::HLE::_SyscallHandler->LockBeforeFork(Frame->Thread);

      uint64_t Result {};
      int NewServerFD{-1};
      if (args->Type == TYPE_CLONE2) {
        Result = Clone2Handler(Frame, args, &NewServerFD);
      } else {
        Result = Clone3Handler(Frame, args, &NewServerFD);
      }

      if (NewServerFD != -1) {
        ///< Close the new serverFD
        close(NewServerFD);
      }

      if (Result != 0) {
        // Parent
        // Unlock the mutexes on both sides of the fork
        FEX::HLE::_SyscallHandler->UnlockAfterFork(Frame->Thread, false);

        ::syscall(SYS_rt_sigprocmask, SIG_SETMASK, &args->SignalMask, nullptr, sizeof(args->SignalMask));
      }
      return Result;
    } else {
      LogMan::Msg::IFmt("Unsupported flag with CLONE_THREAD. This breaks TLS, falling down classic thread path");
      PrintFlags(flags);
    }
  }

  constexpr uint64_t TASK_MAX = (1ULL << 48); // 48-bits until we can query the host side VA sanely. AArch64 doesn't expose this in cpuinfo
  if (args->args.tls && args->args.tls >= TASK_MAX) {
    return -EPERM;
  }

  auto Thread = Frame->Thread;

  if (AnyFlagsSet(flags, CLONE_PTRACE)) {
    PrintFlags(flags);
    LogMan::Msg::DFmt("clone: Ptrace* not supported");
  }

  if (!(flags & CLONE_THREAD)) {
    // CLONE_PARENT is ignored (Implied by CLONE_THREAD)
    return FEX::HLE::ForkGuest(Thread, Frame, flags, reinterpret_cast<void*>(args->args.stack), args->args.stack_size,
                               reinterpret_cast<pid_t*>(args->args.parent_tid), reinterpret_cast<pid_t*>(args->args.child_tid),
                               reinterpret_cast<void*>(args->args.tls));
  } else {
    auto NewThread = FEX::HLE::CreateNewThread(Thread->CTX, Frame, args);

    // Return the new threads TID
    uint64_t Result = NewThread->Thread->ThreadManager.GetTID();

    // Actually start the thread
    FEX::HLE::_SyscallHandler->TM.RunThread(NewThread);

    if (flags & CLONE_VFORK) {
      // If VFORK is set then the calling process is suspended until the thread exits with execve or exit
      NewThread->Thread->ExecutionThread->join(nullptr);

      // Normally a thread cleans itself up on exit. But because we need to join, we are now responsible
      FEX::HLE::_SyscallHandler->TM.DestroyThread(NewThread);
    }

    SYSCALL_ERRNO();
  }
};

uint64_t SyscallHandler::HandleBRK(FEXCore::Core::CpuStateFrame* Frame, void* Addr) {
  std::lock_guard<std::mutex> lk(MMapMutex);

  uint64_t Result;

  if (Addr == nullptr) { // Just wants to get the location of the program break atm
    Result = DataSpace + DataSpaceSize;
  } else {
    // Allocating out data space
    uint64_t NewEnd = reinterpret_cast<uint64_t>(Addr);
    if (NewEnd < DataSpace) {
      // Not allowed to move brk end below original start
      // Set the size to zero
      DataSpaceSize = 0;
    } else {
      uint64_t NewSize = NewEnd - DataSpace;
      uint64_t NewSizeAligned = FEXCore::AlignUp(NewSize, 4096);

      if (NewSizeAligned < DataSpaceMaxSize) {
        // If we are shrinking the brk then munmap the ranges
        // That way we gain the memory back and also give the application zero pages if it allocates again
        // DataspaceMaxSize is always page aligned

        uint64_t RemainingSize = DataSpaceMaxSize - NewSizeAligned;
        // We have pages we can unmap
        [[maybe_unused]] auto ok = GuestMunmap(Frame->Thread, reinterpret_cast<void*>(DataSpace + NewSizeAligned), RemainingSize);
        LOGMAN_THROW_A_FMT(ok != -1, "Munmap failed");

        DataSpaceMaxSize = NewSizeAligned;
      } else if (NewSize > DataSpaceMaxSize) {
        constexpr static uint64_t SizeAlignment = 8 * 1024 * 1024;
        uint64_t AllocateNewSize = FEXCore::AlignUp(NewSize, SizeAlignment) - DataSpaceMaxSize;
        if (!Is64BitMode() && (DataSpace + DataSpaceMaxSize + AllocateNewSize > 0x1'0000'0000ULL)) {
          // If we are 32bit and we tried going about the 32bit limit then out of memory
          return DataSpace + DataSpaceSize;
        }

        uint64_t NewBRK {};
        NewBRK = (uint64_t)GuestMmap(Frame->Thread, (void*)(DataSpace + DataSpaceMaxSize), AllocateNewSize, PROT_READ | PROT_WRITE,
                                     MAP_FIXED_NOREPLACE | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);


        if (NewBRK != ~0ULL && NewBRK != (DataSpace + DataSpaceMaxSize)) {
          // Couldn't allocate that the region we wanted
          // Can happen if MAP_FIXED_NOREPLACE isn't understood by the kernel
          [[maybe_unused]] int ok = GuestMunmap(Frame->Thread, reinterpret_cast<void*>(NewBRK), AllocateNewSize);
          LOGMAN_THROW_A_FMT(ok != -1, "Munmap failed");
          NewBRK = ~0ULL;
        }

        if (NewBRK == ~0ULL) {
          // If we couldn't allocate a new region then out of memory
          return DataSpace + DataSpaceSize;
        } else {
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
  DataSpace = Base;
  DataSpaceMaxSize = Size;
  DataSpaceStartingSize = Size;
}

SyscallHandler::SyscallHandler(FEXCore::Context::Context* _CTX, FEX::HLE::SignalDelegator* _SignalDelegation)
  : TM {_CTX, _SignalDelegation}
  , SeccompEmulator {this}
  , FM {_CTX}
  , CTX {_CTX}
  , SignalDelegation {_SignalDelegation} {
  FEX::HLE::_SyscallHandler = this;
  HostKernelVersion = CalculateHostKernelVersion();
  GuestKernelVersion = CalculateGuestKernelVersion();
  Alloc32Handler = FEX::HLE::Create32BitAllocator();

  SignalDelegation->RegisterHostSignalHandler(SIGSEGV, HandleSegfault, true);
}

SyscallHandler::~SyscallHandler() {
  FEXCore::Allocator::munmap(reinterpret_cast<void*>(DataSpace), DataSpaceMaxSize);
}

uint32_t SyscallHandler::CalculateHostKernelVersion() {
  struct utsname buf {};
  if (uname(&buf) == -1) {
    return 0;
  }

  uint32_t Major {};
  uint32_t Minor {};
  uint32_t Patch {};

  // Parse kernel version in the form of `<Major>.<Minor>.<Patch>[Optional Data]`
  const auto End = buf.release + sizeof(buf.release);
  auto Results = std::from_chars(buf.release, End, Major, 10);
  Results = std::from_chars(Results.ptr + 1, End, Minor, 10);
  Results = std::from_chars(Results.ptr + 1, End, Patch, 10);

  return (Major << 24) | (Minor << 16) | Patch;
}

uint32_t SyscallHandler::CalculateGuestKernelVersion() {
  // We currently only emulate a kernel between the ranges of Kernel 5.0.0 and 6.8.0
  return std::max(KernelVersion(5, 0), std::min(KernelVersion(6, 8), GetHostKernelVersion()));
}

uint64_t SyscallHandler::HandleSyscall(FEXCore::Core::CpuStateFrame* Frame, FEXCore::HLE::SyscallArguments* Args) {
  // Need to reconstruct RIP from the return address. Which will be inside the FEX JIT.
  const uint64_t RIP =
    CTX->RestoreRIPFromHostPC(Frame->Thread, reinterpret_cast<uint64_t>(__builtin_extract_return_addr(__builtin_return_address(0))));

  const auto SeccompResult = SeccompEmulator.ExecuteFilter(Frame, RIP, Args);
  const auto ActionMasked = SeccompResult.SeccompResult & SECCOMP_RET_ACTION_FULL;
  const auto DataMasked = SeccompResult.SeccompResult & SECCOMP_RET_DATA;

  const auto Arch = Is64BitMode() ? AUDIT_ARCH_X86_64 : AUDIT_ARCH_I386;

  // Logging rules
  // - Log if explicitly returning SECCOMP_RET_LOG
  // - Log if the filter enabled the logging flag and the action is something other than SECCOMP_RET_ALLOW.
  if ((SeccompResult.ShouldLog && ActionMasked != SECCOMP_RET_ALLOW) || ActionMasked == SECCOMP_RET_LOG) {
    int Signal = 0;
    switch (ActionMasked) {
    case SECCOMP_RET_KILL_PROCESS:
    case SECCOMP_RET_KILL_THREAD: Signal = SeccompEmulator.GetKillSignal(); break;
    case SECCOMP_RET_TRAP: Signal = SIGSYS; break;
    default: break;
    }

    // Logs in to dmesg. Log through FEX
    // ex: `[13572.669277] audit: type=1326 audit(1715469332.533:62): auid=1000 uid=1000 gid=1000 ses=2 subj=unconfined pid=52546 comm="seccomp_bpf"
    // exe="/mnt/Work/Projects/work/linux/tools/testing/selftests/seccomp/seccomp_bpf" sig=0 arch=c000003e syscall=39 compat=0 ip=0x7d789352725d code=0x7ffc0000`
    timespec tp {};
    clock_gettime(CLOCK_MONOTONIC, &tp);
    LogMan::Msg::IFmt("audit: type={} audit({}.{:03}:{}): uid={} gid={} pid={} comm={} sig={} arch={:x} syscall={} ip=0x{:x} code=0x{:x}",
                      AUDIT_SECCOMP, tp.tv_sec, tp.tv_nsec / 1'000'000, AuditSerialIncrement(), ::getuid(), ::getgid(), ::getpid(),
                      Filename(), Signal, Arch, Args->Argument[0], RIP, SeccompResult.SeccompResult);
  }

  switch (ActionMasked) {
  ///< Unknown actions behave like RET_KILL_PROCESS.
  default:
  case SECCOMP_RET_KILL_PROCESS: {
    LogMan::Msg::DFmt("[Seccomp] Kill Process!\n");
    const int KillSignal = SeccompEmulator.GetKillSignal();
    // Ignores signal handler and sigmask
    uint64_t Mask = 1 << (KillSignal - 1);
    SignalDelegation->GuestSigProcMask(SIG_UNBLOCK, &Mask, nullptr);
    SignalDelegation->UninstallHostHandler(KillSignal);
    kill(0, KillSignal);
    break;
  }
  case SECCOMP_RET_KILL_THREAD: {
    LogMan::Msg::DFmt("[Seccomp] Kill Thread!\n");
    // Ignores signal handler and sigmask
    siginfo_t Info {
      .si_signo = SIGSYS,
      .si_errno = static_cast<int32_t>(DataMasked),
      .si_code = 1, ///< SYS_SECCOMP
    };

    Info.si_call_addr = reinterpret_cast<void*>(RIP);
    Info.si_syscall = Args->Argument[0];
    Info.si_arch = Arch;

    SignalDelegation->QueueSignal(::getpid(), ::gettid(), SIGSYS, &Info, true);
    break;
  }
  case SECCOMP_RET_TRAP: {
    LogMan::Msg::DFmt("[Seccomp] Trap!\n");
    siginfo_t Info {
      .si_signo = SIGSYS,
      .si_errno = static_cast<int32_t>(DataMasked),
      .si_code = 1, ///< SYS_SECCOMP
    };

    Info.si_call_addr = reinterpret_cast<void*>(RIP);
    Info.si_syscall = Args->Argument[0];
    Info.si_arch = Arch;

    SignalDelegation->QueueSignal(::getpid(), ::gettid(), SIGSYS, &Info, true);
    break;
  }
  case SECCOMP_RET_ERRNO: {
    LogMan::Msg::DFmt("[Seccomp] ERRNO!\n");
    ///< errno return is clamped.
    return -(std::min<uint64_t>(DataMasked, 4095));
  }
  case SECCOMP_RET_USER_NOTIF: LogMan::Msg::DFmt("[Seccomp] User notify!\n"); break;
  case SECCOMP_RET_TRACE: {
    LogMan::Msg::DFmt("[Seccomp] Tracer!");
    // When no tracer attached, behave like RET_ERRNO returning ENOSYS.
    // TODO: Implement once FEX supports tracing.
    return -ENOSYS;
  }
  case SECCOMP_RET_LOG:
  case SECCOMP_RET_ALLOW: break;
  }

  if (Args->Argument[0] >= Definitions.size()) {
    return -ENOSYS;
  }

  auto& Def = Definitions[Args->Argument[0]];
  uint64_t Result {};
  switch (Def.NumArgs) {
  case 0: Result = std::invoke(Def.Ptr0, Frame); break;
  case 1: Result = std::invoke(Def.Ptr1, Frame, Args->Argument[1]); break;
  case 2: Result = std::invoke(Def.Ptr2, Frame, Args->Argument[1], Args->Argument[2]); break;
  case 3: Result = std::invoke(Def.Ptr3, Frame, Args->Argument[1], Args->Argument[2], Args->Argument[3]); break;
  case 4: Result = std::invoke(Def.Ptr4, Frame, Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4]); break;
  case 5:
    Result = std::invoke(Def.Ptr5, Frame, Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5]);
    break;
  case 6:
    Result = std::invoke(Def.Ptr6, Frame, Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5],
                         Args->Argument[6]);
    break;
  // for missing syscalls
  case 255: return std::invoke(Def.Ptr1, Frame, Args->Argument[0]);
  default:
    LOGMAN_MSG_A_FMT("Unhandled syscall: {}", Args->Argument[0]);
    return -1;
    break;
  }
#ifdef DEBUG_STRACE
  Strace(Args, Result);
#endif
  return Result;
}

#ifdef DEBUG_STRACE
void SyscallHandler::Strace(FEXCore::HLE::SyscallArguments* Args, uint64_t Ret) {
  auto& Def = Definitions[Args->Argument[0]];
  switch (Def.NumArgs) {
  case 0: LogMan::Msg::D(Def.StraceFmt.c_str(), Ret); break;
  case 1: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Ret); break;
  case 2: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Ret); break;
  case 3: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Ret); break;
  case 4: LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Ret); break;
  case 5:
    LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5], Ret);
    break;
  case 6:
    LogMan::Msg::D(Def.StraceFmt.c_str(), Args->Argument[1], Args->Argument[2], Args->Argument[3], Args->Argument[4], Args->Argument[5],
                   Args->Argument[6], Ret);
    break;
  default: break;
  }
}
#endif

uint64_t UnimplementedSyscall(FEXCore::Core::CpuStateFrame* Frame, uint64_t SyscallNumber) {
  ERROR_AND_DIE_FMT("Unhandled system call: {}", SyscallNumber);
  return -ENOSYS;
}

uint64_t UnimplementedSyscallSafe(FEXCore::Core::CpuStateFrame* Frame, uint64_t SyscallNumber) {
  return -ENOSYS;
}

void SyscallHandler::LockBeforeFork(FEXCore::Core::InternalThreadState* Thread) {
  Thread->CTX->LockBeforeFork(Thread);
  VMATracking.Mutex.lock();
}

void SyscallHandler::UnlockAfterFork(FEXCore::Core::InternalThreadState* LiveThread, bool Child) {
  if (Child) {
    VMATracking.Mutex.StealAndDropActiveLocks();
  } else {
    VMATracking.Mutex.unlock();
  }

  CTX->UnlockAfterFork(LiveThread, Child);

  // Clear all the other threads that are being tracked
  TM.UnlockAfterFork(LiveThread, Child);
}

static bool isHEX(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

fextl::unique_ptr<FEXCore::HLE::SourcecodeMap>
SyscallHandler::GenerateMap(const std::string_view& GuestBinaryFile, const std::string_view& GuestBinaryFileId) {

  ELFParser GuestELF;

  if (!GuestELF.ReadElf(fextl::string(GuestBinaryFile))) {
    LogMan::Msg::DFmt("GenerateMap: '{}' is not an elf file?", GuestBinaryFile);
    return {};
  }

  struct stat GuestBinaryFileStat;

  if (stat(GuestBinaryFile.data(), &GuestBinaryFileStat)) {
    LogMan::Msg::DFmt("GenerateMap: failed to stat '{}'", GuestBinaryFile);
    return {};
  }

  const auto FexSrcPath = fextl::fmt::format("{}/fexsrc", FEXCore::Config::GetDataDirectory());

  if (!FHU::Filesystem::CreateDirectories(FexSrcPath)) {
    LogMan::Msg::DFmt("GenerateMap: failed to create_directories '{}'", FexSrcPath);
    return {};
  }

  const auto GuestSourceFile = fextl::fmt::format("{}/{}.src", FexSrcPath, GuestBinaryFileId);

  struct stat GuestSourceFileStat;

  if (stat(GuestSourceFile.data(), &GuestSourceFileStat) != 0 || GuestBinaryFileStat.st_mtime > GuestSourceFileStat.st_mtime) {
    LogMan::Msg::DFmt("GenerateMap: Generating source for '{}'", GuestBinaryFile);
    auto command = fextl::fmt::format("x86_64-linux-gnu-objdump -SC \'{}\' > '{}'", GuestBinaryFile, GuestSourceFile);
    if (system(command.c_str()) != 0) {
      LogMan::Msg::DFmt("GenerateMap: '{}' failed", command);
      return {};
    }
  }

  const auto GuestIndexFile = fextl::fmt::format("{}/{}.idx", FexSrcPath, GuestBinaryFileId);
  struct stat GuestIndexFileStat;

  bool GenerateIndex = stat(GuestIndexFile.data(), &GuestIndexFileStat) != 0 || GuestSourceFileStat.st_mtime > GuestIndexFileStat.st_mtime;

  constexpr char SrcHeaderString[] = "fexsrcindex0";
  if (!GenerateIndex) {
    // Index file de-serialization
    LogMan::Msg::DFmt("GenerateMap: Reading index '{}'", GuestIndexFile);

    int FD = ::open(GuestIndexFile.c_str(), O_RDONLY | O_CLOEXEC);

    if (FD == -1) {
      LogMan::Msg::DFmt("GenerateMap: Failed to open '{}'", GuestIndexFile);
      goto DoGenerate;
    }

    //"fexsrcindex0"
    char filemagic[12];
    ::read(FD, filemagic, sizeof(filemagic));
    if (memcmp(filemagic, SrcHeaderString, sizeof(filemagic)) != 0) {
      LogMan::Msg::DFmt("GenerateMap: '{}' has invalid magic '{}'", GuestIndexFile, filemagic);
      close(FD);
      goto DoGenerate;
    }

    auto rv = fextl::make_unique<FEXCore::HLE::SourcecodeMap>();

    {
      auto len = rv->SourceFile.size();
      ::read(FD, (char*)&len, sizeof(len));
      rv->SourceFile.resize(len);
      ::read(FD, rv->SourceFile.data(), len);
    }

    {
      auto len = rv->SortedLineMappings.size();

      ::read(FD, (char*)&len, sizeof(len));

      rv->SortedLineMappings.resize(len);

      for (auto& Mapping : rv->SortedLineMappings) {
        ::read(FD, (char*)&Mapping.FileGuestBegin, sizeof(Mapping.FileGuestBegin));
        ::read(FD, (char*)&Mapping.FileGuestEnd, sizeof(Mapping.FileGuestEnd));
        ::read(FD, (char*)&Mapping.LineNumber, sizeof(Mapping.LineNumber));
      }
    }

    {
      auto len = rv->SortedSymbolMappings.size();

      ::read(FD, (char*)&len, sizeof(len));

      rv->SortedSymbolMappings.resize(len);

      for (auto& Mapping : rv->SortedSymbolMappings) {
        ::read(FD, (char*)&Mapping.FileGuestBegin, sizeof(Mapping.FileGuestBegin));
        ::read(FD, (char*)&Mapping.FileGuestEnd, sizeof(Mapping.FileGuestEnd));

        {
          auto len = Mapping.Name.size();
          ::read(FD, (char*)&len, sizeof(len));
          Mapping.Name.resize(len);
          ::read(FD, Mapping.Name.data(), len);
        }
      }
    }

    LogMan::Msg::DFmt("GenerateMap: Finished reading index");
    close(FD);
    return rv;
  } else {
// objdump output parsing,  index generation, index file serialization
DoGenerate:
    LogMan::Msg::DFmt("GenerateMap: Generating index for '{}'", GuestSourceFile);
    int StreamFD = ::open(GuestSourceFile.c_str(), O_RDONLY | O_CLOEXEC);

    if (StreamFD == -1) {
      LogMan::Msg::DFmt("GenerateMap: Failed to open '{}'", GuestSourceFile);
      return {};
    }

    fextl::string SourceData;
    if (!FEXCore::FileLoading::LoadFile(SourceData, GuestSourceFile)) {
      return {};
    }
    fextl::istringstream Stream(SourceData);

    constexpr int USER_PERMS = S_IRWXU | S_IRWXG | S_IRWXO;
    int IndexStream = ::open(GuestIndexFile.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC, USER_PERMS);

    if (IndexStream == -1) {
      LogMan::Msg::DFmt("GenerateMap: Failed to open '{}' for writing", GuestIndexFile);
      return {};
    }

    ::write(IndexStream, SrcHeaderString, strlen(SrcHeaderString));

    // objdump parsing
    fextl::string Line;
    int LineNum = 0;

    bool PreviousLineWasEmpty = false;

    uintptr_t LastSymbolOffset {};
    uintptr_t CurrentSymbolOffset {};
    fextl::string LastSymbolName;

    uintptr_t LastOffset {};
    uintptr_t CurrentOffset {};
    int LastOffsetLine;

    auto rv = fextl::make_unique<FEXCore::HLE::SourcecodeMap>();

    rv->SourceFile = GuestSourceFile;

    auto EndSymbol = [&] {
      if (LastSymbolOffset) {
        rv->SortedSymbolMappings.push_back({LastSymbolOffset, CurrentSymbolOffset, LastSymbolName});

        // LogMan::Msg::DFmt("Ended Symbol {} - {:x}...{:x}", LastSymbolName, LastSymbolOffset, CurrentSymbolOffset);
      }
      LastSymbolOffset = {};
    };

    auto EndLine = [&] {
      if (LastOffset) {
        rv->SortedLineMappings.push_back({LastOffset, CurrentOffset, LastOffsetLine});

        // LogMan::Msg::DFmt("Ended Line {} - {:x}...{:x}", LastOffsetLine, LastOffset, CurrentOffset);
      }
      LastOffset = {};
    };

    while (std::getline(Stream, Line)) {
      LineNum++;

      auto LineIsEmpty = Line.empty();

      if (LineIsEmpty) {
        PreviousLineWasEmpty = true;
      } else {

        // LogMan::Msg::DFmt("Line: '{}'", Line);

        if (isHEX(Line[0])) {
          fextl::string addr;
          int offs = 1;
          for (; !isspace(Line[offs]) && offs < Line.size(); offs++)
            ;

          if (offs == Line.size()) {
            continue;
          }
          if (offs != 8 && offs != 16) {
            continue;
          }

          auto VAOffset = std::strtoul(Line.substr(0, offs).c_str(), nullptr, 16);

          auto FileOffset = GuestELF.VAToFile(VAOffset);

          if (FileOffset == 0) {
            LogMan::Msg::EFmt("File Offset {:x} did not map to file?! {}", VAOffset, Line);
          }

          CurrentSymbolOffset = FileOffset;

          if (PreviousLineWasEmpty) {
            EndSymbol();
          }
          LastSymbolOffset = CurrentSymbolOffset;

          for (; Line[offs] != '<' && offs < Line.size(); offs++)
            ;

          if (offs == Line.size()) {
            continue;
          }

          offs++;

          LastSymbolName = Line.substr(offs, Line.size() - 2 - offs);

          // LogMan::Msg::DFmt("Symbol {} @ {:x} -> Line {}", LastSymbolName, LastSymbolOffset, LineNum);
        } else if (isspace(Line[0])) {
          int offs = 1;
          for (; isspace(Line[offs]) && offs < Line.size(); offs++)
            ;

          if (offs == Line.size()) {
            continue;
          }

          int start = offs;

          for (; Line[offs] != ':' && offs < Line.size(); offs++)
            ;

          if (offs == Line.size()) {
            continue;
          }

          if (Line[offs + 1] == '\t') {
            auto VAOffsetStr = Line.substr(start, offs - start);
            auto VAOffset = std::strtoul(VAOffsetStr.c_str(), nullptr, 16);
            auto FileOffset = GuestELF.VAToFile(VAOffset);
            if (FileOffset == 0) {
              LogMan::Msg::EFmt("File Offset {:x} did not map to file?! {}", VAOffset, Line);
            } else {
              if (LastOffset > FileOffset) {
                LogMan::Msg::EFmt("File Offset {:x} less than previous {:} ?!  {}", FileOffset, LastOffset, Line);
              }
              CurrentOffset = FileOffset;

              EndLine();

              LastOffset = CurrentOffset;
              LastOffsetLine = LineNum;
            }
          }
        }
        // something else -- keep going
      }
    }

    CurrentOffset = LastOffset + 4;
    CurrentSymbolOffset = CurrentOffset;

    EndSymbol();
    EndLine();

    // Index post processing - entires are sorted for faster lookups

    std::sort(rv->SortedLineMappings.begin(), rv->SortedLineMappings.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.FileGuestEnd <= rhs.FileGuestBegin; });

    std::sort(rv->SortedSymbolMappings.begin(), rv->SortedSymbolMappings.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.FileGuestEnd <= rhs.FileGuestBegin; });

    // Index serialization
    {
      auto len = rv->SourceFile.size();
      ::write(IndexStream, (const char*)&len, sizeof(len));
      ::write(IndexStream, rv->SourceFile.c_str(), len);
    }

    {
      auto len = rv->SortedLineMappings.size();

      ::write(IndexStream, (const char*)&len, sizeof(len));

      for (const auto& Mapping : rv->SortedLineMappings) {
        ::write(IndexStream, (const char*)&Mapping.FileGuestBegin, sizeof(Mapping.FileGuestBegin));
        ::write(IndexStream, (const char*)&Mapping.FileGuestEnd, sizeof(Mapping.FileGuestEnd));
        ::write(IndexStream, (const char*)&Mapping.LineNumber, sizeof(Mapping.LineNumber));
      }
    }

    {
      auto len = rv->SortedSymbolMappings.size();

      ::write(IndexStream, (char*)&len, sizeof(len));

      for (const auto& Mapping : rv->SortedSymbolMappings) {
        ::write(IndexStream, (const char*)&Mapping.FileGuestBegin, sizeof(Mapping.FileGuestBegin));
        ::write(IndexStream, (const char*)&Mapping.FileGuestEnd, sizeof(Mapping.FileGuestEnd));

        {
          auto len = Mapping.Name.size();
          ::write(IndexStream, (const char*)&len, sizeof(len));
          ::write(IndexStream, Mapping.Name.c_str(), len);
        }
      }
    }

    if (StreamFD != -1) {
      close(StreamFD);
    }

    if (IndexStream != -1) {
      close(IndexStream);
    }

    LogMan::Msg::DFmt("GenerateMap: Finished generating index", GuestIndexFile);
    return rv;
  }
}

} // namespace FEX::HLE
