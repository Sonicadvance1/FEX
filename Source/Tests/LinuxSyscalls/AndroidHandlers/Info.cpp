/*
$info$
tags: LinuxSyscalls|android-syscalls-shared
$end_info$
*/

#include "Tests/LinuxSyscalls/AndroidHandlers/Handlers.h"
#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

#include <FEXCore/IR/IR.h>
#include <FEXCore/Debug/InternalThreadState.h>

namespace FEX::HLE::Android {
  void RegisterInfo() {
    using namespace FEXCore::IR;
    REGISTER_SYSCALL_NO_PERM(syslog);
    REGISTER_SYSCALL_NO_PERM(kcmp);
  }
}
