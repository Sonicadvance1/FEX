/*
$info$
tags: LinuxSyscalls|android-syscalls-shared
$end_info$
*/

#include "FEXCore/IR/IR.h"

#include "Tests/LinuxSyscalls/AndroidHandlers/Handlers.h"
#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

namespace FEX::HLE::Android {
  void RegisterThread() {
    using namespace FEXCore::IR;

    REGISTER_SYSCALL_NO_PERM(setregid);
    REGISTER_SYSCALL_NO_PERM(setgid);
    REGISTER_SYSCALL_NO_PERM(setreuid);
    REGISTER_SYSCALL_NO_PERM(setuid);
    REGISTER_SYSCALL_NO_PERM(setresgid);

    REGISTER_SYSCALL_IMPL_FLAGS(setfsuid, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, uid_t fsuid) -> uint64_t {

      // Returns previous UID of the caller on both success and failure.
      // Just return uid
      uint64_t Result = ::getuid();
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_FLAGS(setfsgid, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, uid_t fsgid) -> uint64_t {
      // Returns previous UID of the caller on both success and failure.
      // Just return gid
      uint64_t Result = ::getgid();
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_NO_PERM(setgroups);
  }
}
