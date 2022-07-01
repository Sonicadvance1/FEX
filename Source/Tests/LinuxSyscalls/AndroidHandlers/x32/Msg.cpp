/*
$info$
tags: LinuxSyscalls|android-syscalls-x86-32
$end_info$
*/

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Types.h"

#include "Tests/LinuxSyscalls/x64/Syscalls.h"

#include <stdint.h>
#include <syscall.h>
#include <time.h>
#include <unistd.h>

ARG_TO_STR(FEX::HLE::x32::compat_ptr<FEX::HLE::x32::mq_attr32>, "%lx")
ARG_TO_STR(FEX::HLE::x32::compat_ptr<FEX::HLE::x32::sigevent32>, "%lx")

namespace FEX::HLE::Android::x32 {
  void RegisterMsg() {
using namespace FEX::HLE::x32;
    REGISTER_SYSCALL_IMPL_X32(mq_timedsend, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec32 *abs_timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X32(mq_timedreceive, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio, const struct timespec32 *abs_timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X32(mq_timedsend_time64, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *abs_timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X32(mq_timedreceive_time64, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X32(mq_open, [](FEXCore::Core::CpuStateFrame *Frame, const char *name, int oflag, mode_t mode, compat_ptr<FEX::HLE::x32::mq_attr32> attr) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X32(mq_notify, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, const compat_ptr<FEX::HLE::x32::sigevent32> sevp) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X32(mq_getsetattr, [](FEXCore::Core::CpuStateFrame *Frame, FEX::HLE::mqd_t mqdes, compat_ptr<FEX::HLE::x32::mq_attr32> newattr, compat_ptr<FEX::HLE::x32::mq_attr32> oldattr) -> uint64_t {
      return -ENOSYS;
    });
  }
}
