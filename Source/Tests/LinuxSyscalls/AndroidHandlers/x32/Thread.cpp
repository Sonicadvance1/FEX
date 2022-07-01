/*
$info$
tags: LinuxSyscalls|android-syscalls-x32
$end_info$
*/

#include "Tests/LinuxSyscalls/AndroidHandlers/Handlers.h"
#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"
#include "x32/SyscallsEnum.h"

#include <FEXCore/Debug/InternalThreadState.h>

namespace FEX::HLE::Android::x32 {
  void RegisterThread() {
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
  }
}
