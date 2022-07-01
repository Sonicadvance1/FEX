/*
$info$
tags: LinuxSyscalls|android-syscalls-shared
$end_info$
*/

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

#include <FEXCore/IR/IR.h>

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace FEX::HLE::Android {
  void RegisterMsg() {
    using namespace FEXCore::IR;

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(msgget, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, key_t key, int msgflg) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(msgsnd, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, int msqid, const void *msgp, size_t msgsz, int msgflg) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(msgrcv, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(msgctl, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, int msqid, int cmd, struct msqid_ds *buf) -> uint64_t {
      return -ENOSYS;
    });

    // last two parameters are optional
    REGISTER_SYSCALL_IMPL_PASS_FLAGS(mq_unlink, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, const char *name) -> uint64_t {
      return -ENOSYS;
    });
  }
}
