/*
$info$
tags: LinuxSyscalls|android-syscalls-shared
$end_info$
*/

#include "Tests/LinuxSyscalls/AndroidHandlers/Handlers.h"
#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

namespace FEX::HLE::Android {
  void RegisterFS() {
    using namespace FEX::HLE;
    REGISTER_SYSCALL_NO_PERM(chroot);
    REGISTER_SYSCALL_NOT_IMPL(acct);
    REGISTER_SYSCALL_NO_PERM(mount);
    REGISTER_SYSCALL_NO_PERM(umount2);
    REGISTER_SYSCALL_NO_PERM(swapon);
    REGISTER_SYSCALL_NO_PERM(swapoff);
  }
}
