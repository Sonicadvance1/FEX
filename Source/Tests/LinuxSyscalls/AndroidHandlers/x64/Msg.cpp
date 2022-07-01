/*
$info$
tags: LinuxSyscalls|android-syscalls-x86-64
$end_info$
*/

#include "Tests/LinuxSyscalls/Types.h"
#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"

#include <stddef.h>
#include <stdint.h>
#include <syscall.h>
#include <unistd.h>

namespace FEXCore::Core {
  struct CpuStateFrame;
}

namespace FEX::HLE::Android::x64 {
  void RegisterMsg() {
    REGISTER_SYSCALL_IMPL_X64(mq_timedsend, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *abs_timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X64(mq_timedreceive, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X64(mq_open, [](FEXCore::Core::CpuStateFrame *Frame, const char *name, int oflag, mode_t mode, struct mq_attr *attr) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X64(mq_notify, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, const struct sigevent *sevp) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X64(mq_getsetattr, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, struct mq_attr *newattr, struct mq_attr *oldattr) -> uint64_t {
      return -ENOSYS;
    });
  }
}
