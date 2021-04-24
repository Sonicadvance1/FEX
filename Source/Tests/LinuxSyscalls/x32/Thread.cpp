/*
$info$
tags: LinuxSyscalls|syscalls-x86-32
$end_info$
*/

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/Syscalls/Thread.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Thread.h"

#include <FEXCore/Core/Context.h>
#include <FEXCore/Debug/InternalThreadState.h>

#include <stdint.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>

ARG_TO_STR(FEX::HLE::x32::compat_ptr<FEX::HLE::x32::stack_t32>, "%x")

namespace FEX::HLE::x32 {
  uint64_t SetThreadArea(FEXCore::Core::CpuStateFrame *Frame, void *tls) {
    struct x32::user_desc* u_info = reinterpret_cast<struct x32::user_desc*>(tls);

    static bool Initialized = false;
    if (Initialized == true && u_info->entry_number == -1) {
      LogMan::Msg::A("Trying to load a new GDT");
    }
    if (u_info->entry_number == -1) {
      u_info->entry_number = 12; // Sure?
      Initialized = true;
    }
    // Now we need to update the thread's GDT to handle this change
    auto GDT = &Frame->State.gdt[u_info->entry_number];
    GDT->base = u_info->base_addr;
    return 0;
  }

  void AdjustRipForNewThread(FEXCore::Core::CpuStateFrame *Frame) {
    Frame->State.rip += 2;
  }

  static bool AnyFlagsSet(uint64_t Flags, uint64_t Mask) {
    return (Flags & Mask) != 0;
  }

  static bool AllFlagsSet(uint64_t Flags, uint64_t Mask) {
    return (Flags & Mask) == Mask;
  }

  void RegisterThread() {
    REGISTER_SYSCALL_IMPL_X32(clone, [](FEXCore::Core::CpuStateFrame *Frame, uint32_t flags, void *stack, pid_t *parent_tid, void *tls, pid_t *child_tid) -> uint64_t {
    #define FLAGPRINT(x, y) if (flags & (y)) LogMan::Msg::I("\tFlag: " #x)
      FLAGPRINT(CSIGNAL,              0x000000FF);
      FLAGPRINT(CLONE_VM,             0x00000100);
      FLAGPRINT(CLONE_FS,             0x00000200);
      FLAGPRINT(CLONE_FILES,          0x00000400);
      FLAGPRINT(CLONE_SIGHAND,        0x00000800);
      FLAGPRINT(CLONE_PTRACE,         0x00002000);
      FLAGPRINT(CLONE_VFORK,          0x00004000);
      FLAGPRINT(CLONE_PARENT,         0x00008000);
      FLAGPRINT(CLONE_THREAD,         0x00010000);
      FLAGPRINT(CLONE_NEWNS,          0x00020000);
      FLAGPRINT(CLONE_SYSVSEM,        0x00040000);
      FLAGPRINT(CLONE_SETTLS,         0x00080000);
      FLAGPRINT(CLONE_PARENT_SETTID,  0x00100000);
      FLAGPRINT(CLONE_CHILD_CLEARTID, 0x00200000);
      FLAGPRINT(CLONE_DETACHED,       0x00400000);
      FLAGPRINT(CLONE_UNTRACED,       0x00800000);
      FLAGPRINT(CLONE_CHILD_SETTID,   0x01000000);
      FLAGPRINT(CLONE_NEWCGROUP,      0x02000000);
      FLAGPRINT(CLONE_NEWUTS,         0x04000000);
      FLAGPRINT(CLONE_NEWIPC,         0x08000000);
      FLAGPRINT(CLONE_NEWUSER,        0x10000000);
      FLAGPRINT(CLONE_NEWPID,         0x20000000);
      FLAGPRINT(CLONE_NEWNET,         0x40000000);
      FLAGPRINT(CLONE_IO,             0x80000000);

      auto Thread = Frame->Thread;

      if (AnyFlagsSet(flags, CLONE_UNTRACED | CLONE_PTRACE)) {
        LogMan::Msg::D("clone: Ptrace* not supported");
      }

      if (AnyFlagsSet(flags, CLONE_NEWNS | CLONE_NEWCGROUP | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWNET)) {
        // NEWUSER doesn't need any privileges from 3.8 onward
        // We just don't support it yet
        LogMan::Msg::I("Unconditionally returning EPERM on clone namespace");
        return -EPERM;
      }

      if (!(flags & CLONE_THREAD)) {

        if (flags & CLONE_VFORK) {
          flags &= ~CLONE_VFORK;
          flags &= ~CLONE_VM;
          LogMan::Msg::D("clone: WARNING: CLONE_VFORK w/o CLONE_THREAD");
        }

        if (AnyFlagsSet(flags, CLONE_SYSVSEM | CLONE_FS |  CLONE_FILES | CLONE_SIGHAND | CLONE_VM)) {
          LogMan::Msg::I("clone: Unsuported flags w/o CLONE_THREAD (Shared Resources), %X", flags);
        }

        // CLONE_PARENT is ignored (Implied by CLONE_THREAD)
        return FEX::HLE::ForkGuest(Thread, Frame, flags, stack, parent_tid, child_tid, tls);
      } else {

        if (!AllFlagsSet(flags, CLONE_SYSVSEM | CLONE_FS |  CLONE_FILES | CLONE_SIGHAND)) {
          LogMan::Msg::I("clone: CLONE_THREAD: Unsuported flags w/ CLONE_THREAD (Shared Resources), %X", flags);
        }

        auto NewThread = FEX::HLE::CreateNewThread(Thread->CTX, Frame, flags, stack, parent_tid, child_tid, tls);

        // Return the new threads TID
        uint64_t Result = NewThread->ThreadManager.GetTID();

        if (flags & CLONE_VFORK) {
          NewThread->DestroyedByParent = true;
        }

        // Actually start the thread
        FEXCore::Context::RunThread(Thread->CTX, NewThread);

        if (flags & CLONE_VFORK) {
          // If VFORK is set then the calling process is suspended until the thread exits with execve or exit
          NewThread->ExecutionThread->join(nullptr);

          // Normally a thread cleans itself up on exit. But because we need to join, we are now responsible 
          FEXCore::Context::DestroyThread(Thread->CTX, NewThread);
        }
        SYSCALL_ERRNO();
      }
    });

    REGISTER_SYSCALL_IMPL_X32(waitpid, [](FEXCore::Core::CpuStateFrame *Frame, pid_t pid, int32_t *status, int32_t options) -> uint64_t {
      uint64_t Result = ::waitpid(pid, status, options);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(nice, [](FEXCore::Core::CpuStateFrame *Frame, int inc) -> uint64_t {
      uint64_t Result = ::nice(inc);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(set_thread_area, [](FEXCore::Core::CpuStateFrame *Frame, struct user_desc *u_info) -> uint64_t {
      return SetThreadArea(Frame, u_info);
    });

    REGISTER_SYSCALL_IMPL_X32(set_robust_list, [](FEXCore::Core::CpuStateFrame *Frame, struct robust_list_head *head, size_t len) -> uint64_t {
      auto Thread = Frame->Thread;
      // Retain the robust list head but don't give it to the kernel
      // The kernel would break if it tried parsing a 32bit robust list from a 64bit process
      Thread->ThreadManager.robust_list_head = reinterpret_cast<uint64_t>(head);
      return 0;
    });

    REGISTER_SYSCALL_IMPL_X32(get_robust_list, [](FEXCore::Core::CpuStateFrame *Frame, int pid, struct robust_list_head **head, uint32_t *len_ptr) -> uint64_t {
      auto Thread = Frame->Thread;
      // Give the robust list back to the application
      // Steam specifically checks to make sure the robust list is set
      *(uint32_t**)head = (uint32_t*)Thread->ThreadManager.robust_list_head;
      *len_ptr = 12;
      return 0;
    });

    REGISTER_SYSCALL_IMPL_X32(futex, [](FEXCore::Core::CpuStateFrame *Frame, int *uaddr, int futex_op, int val, const timespec32 *timeout, int *uaddr2, uint32_t val3) -> uint64_t {
      void* timeout_ptr = (void*)timeout;
      struct timespec tp64{};
      int cmd = futex_op & FUTEX_CMD_MASK;
      if (timeout &&
          (cmd == FUTEX_WAIT ||
           cmd == FUTEX_LOCK_PI ||
           cmd == FUTEX_WAIT_BITSET ||
           cmd == FUTEX_WAIT_REQUEUE_PI)) {
        // timeout argument is only handled as timespec in these cases
        // Otherwise just an integer
        tp64 = *timeout;
        timeout_ptr = &tp64;
      }

      uint64_t Result = syscall(SYS_futex,
        uaddr,
        futex_op,
        val,
        timeout_ptr,
        uaddr2,
        val3);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(getuid32, [](FEXCore::Core::CpuStateFrame *Frame) -> uint64_t {
      uint64_t Result = ::getuid();
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(getgid32, [](FEXCore::Core::CpuStateFrame *Frame) -> uint64_t {
      uint64_t Result = ::getgid();
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(setuid32, [](FEXCore::Core::CpuStateFrame *Frame, uid_t uid) -> uint64_t {
      uint64_t Result = ::setuid(uid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(setgid32, [](FEXCore::Core::CpuStateFrame *Frame, gid_t gid) -> uint64_t {
      uint64_t Result = ::setgid(gid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(geteuid32, [](FEXCore::Core::CpuStateFrame *Frame) -> uint64_t {
      uint64_t Result = ::geteuid();
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(getegid32, [](FEXCore::Core::CpuStateFrame *Frame) -> uint64_t {
      uint64_t Result = ::getegid();
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(setreuid32, [](FEXCore::Core::CpuStateFrame *Frame, uid_t ruid, uid_t euid) -> uint64_t {
      uint64_t Result = ::setreuid(ruid, euid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(setresuid32, [](FEXCore::Core::CpuStateFrame *Frame, uid_t ruid, uid_t euid, uid_t suid) -> uint64_t {
      uint64_t Result = ::setresuid(ruid, euid, suid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(getresuid32, [](FEXCore::Core::CpuStateFrame *Frame, uid_t *ruid, uid_t *euid, uid_t *suid) -> uint64_t {
      uint64_t Result = ::getresuid(ruid, euid, suid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(setresgid32, [](FEXCore::Core::CpuStateFrame *Frame, gid_t rgid, gid_t egid, gid_t sgid) -> uint64_t {
      uint64_t Result = ::setresgid(rgid, egid, sgid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(getresgid32, [](FEXCore::Core::CpuStateFrame *Frame, gid_t *rgid, gid_t *egid, gid_t *sgid) -> uint64_t {
      uint64_t Result = ::getresgid(rgid, egid, sgid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(setregid32, [](FEXCore::Core::CpuStateFrame *Frame, gid_t rgid, gid_t egid) -> uint64_t {
      uint64_t Result = ::setregid(rgid, egid);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(sigaltstack, [](FEXCore::Core::CpuStateFrame *Frame, const compat_ptr<stack_t32> ss, compat_ptr<stack_t32> old_ss) -> uint64_t {
      stack_t ss64{};
      stack_t old64{};

      stack_t *ss64_ptr{};
      stack_t *old64_ptr{};

      if (ss64_ptr) {
        ss64 = *ss;
        ss64_ptr = &ss64;
      }

      if (old_ss) {
        old64 = *old_ss;
        old64_ptr = &old64;
      }
      uint64_t Result = FEX::HLE::_SyscallHandler->GetSignalDelegator()->RegisterGuestSigAltStack(ss64_ptr, old64_ptr);

      if (Result == 0 && old_ss) {
        *old_ss = old64;
      }
      return Result;
    });

    // launch a new process under fex
    // currently does not propagate argv[0] correctly
    REGISTER_SYSCALL_IMPL_X32(execve, [](FEXCore::Core::CpuStateFrame *Frame, const char *pathname, uint32_t *argv, uint32_t *envp) -> uint64_t {
      std::vector<const char*> Args;
      std::vector<const char*> Envp;

      for (int i = 0; argv[i]; i++) {
        Args.push_back(reinterpret_cast<const char*>(static_cast<uintptr_t>(argv[i])));
      }

      Args.push_back(nullptr);

      for (int i = 0; envp[i]; i++) {
        Envp.push_back(reinterpret_cast<const char*>(static_cast<uintptr_t>(envp[i])));
      }
      Envp.push_back(nullptr);

      return FEX::HLE::ExecveHandler(pathname, Args, Envp);
    });

    REGISTER_SYSCALL_IMPL_X32(wait4, [](FEXCore::Core::CpuStateFrame *Frame, pid_t pid, int *wstatus, int options, struct rusage_32 *rusage) -> uint64_t {
      struct rusage usage64{};
      struct rusage *usage64_p{};

      if (rusage) {
        usage64 = *rusage;
        usage64_p = &usage64;
      }
      uint64_t Result = ::wait4(pid, wstatus, options, usage64_p);
      if (rusage) {
        *rusage = usage64;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(futex_time64, [](FEXCore::Core::CpuStateFrame *Frame, int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, uint32_t val3) -> uint64_t {
      uint64_t Result = syscall(SYS_futex,
        uaddr,
        futex_op,
        val,
        timeout,
        uaddr2,
        val3);
      SYSCALL_ERRNO();
    });
  }
}
