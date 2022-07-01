/*
$info$
tags: LinuxSyscalls|android-syscalls-shared
$end_info$
*/

#include "Tests/LinuxSyscalls/AndroidHandlers/Handlers.h"
#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"
#include "x64/SyscallsEnum.h"

namespace FEX::HLE::Android {
  void RegisterTime() {
    REGISTER_SYSCALL_NO_PERM(clock_settime);
    REGISTER_SYSCALL_NO_PERM(settimeofday);
    REGISTER_SYSCALL_NO_PERM(adjtimex);
  }
}
