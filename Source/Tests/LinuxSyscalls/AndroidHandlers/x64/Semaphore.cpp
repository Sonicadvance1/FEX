/*
$info$
tags: LinuxSyscalls|syscalls-x86-64
$end_info$
*/

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Types.h"

#include <FEXHeaderUtils/Syscalls.h>

#include <linux/sem.h>
#include <stddef.h>
#include <stdint.h>

namespace FEXCore::Core {
  struct CpuStateFrame;
}

ARG_TO_STR(FEX::HLE::x64::semun, "%lx")

namespace FEX::HLE::x64 {
  void RegisterSemaphore() {
   REGISTER_SYSCALL_IMPL_X64_PASS(semop, [](FEXCore::Core::CpuStateFrame *Frame, int semid, struct sembuf *sops, size_t nsops) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X64_PASS(semtimedop, [](FEXCore::Core::CpuStateFrame *Frame, int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X64(semctl, [](FEXCore::Core::CpuStateFrame *Frame, int semid, int semnum, int cmd, FEX::HLE::x64::semun semun) -> uint64_t {
      return -ENOSYS;
    });
  }
}
