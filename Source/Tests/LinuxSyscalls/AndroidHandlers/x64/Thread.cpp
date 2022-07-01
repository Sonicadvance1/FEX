/*
$info$
tags: LinuxSyscalls|android-syscalls-x64
$end_info$
*/

#include "Tests/LinuxSyscalls/AndroidHandlers/Handlers.h"
#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "x64/SyscallsEnum.h"

#include <FEXCore/IR/IR.h>
#include <FEXCore/Debug/InternalThreadState.h>

namespace FEX::HLE::Android::x64 {
  void RegisterThread() {
    using namespace FEXCore::IR;

    REGISTER_SYSCALL_IMPL_X64_FLAGS(set_robust_list, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, struct robust_list_head *head, size_t len) -> uint64_t {
      auto Thread = Frame->Thread;
      Thread->ThreadManager.robust_list_head = reinterpret_cast<uint64_t>(head);
      // Termux/Android doesn't support `set_robust_list` syscall.
      // The seccomp filter that the OS installs explicitly blocks this syscall from working
      // glibc uses this syscall for tls and thread data so almost every application uses it
      // Return success since we have stored the pointer ourselves.
      return 0;
    });

    REGISTER_SYSCALL_IMPL_X64_FLAGS(get_robust_list, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, int pid, struct robust_list_head **head, size_t *len_ptr) -> uint64_t {
      auto Thread = Frame->Thread;

      *head = reinterpret_cast<struct robust_list_head*>(Thread->ThreadManager.robust_list_head);
      *len_ptr = sizeof(struct robust_list_head);

      return 0;
    });
  }
}
