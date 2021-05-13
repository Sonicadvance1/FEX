/*
$info$
tags: LinuxSyscalls|syscalls-shared
$end_info$
*/

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

namespace FEX::HLE {
  void RegisterSemaphore() {
    REGISTER_SYSCALL_IMPL(semget, [](FEXCore::Core::CpuStateFrame *Frame, key_t key, int nsems, int semflg) -> uint64_t {
      uint64_t Result = ::semget(key, nsems, semflg);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL(semctl, [](FEXCore::Core::CpuStateFrame *Frame, int semid, int semnum, int cmd, void* semun) -> uint64_t {
      // semun struct doesn't match between x86-64 and aarch64
      uint64_t Result{};
      switch (cmd) {
        case IPC_SET: {
          break;
        }
        case IPC_STAT: {
          break;
        }
        case IPC_INFO: {
          break;
        }
        case SEM_INFO: {
          break;
        }
        case SEM_STAT: {
          break;
        }
        case SEM_STAT_ANY: {
          break;
        }
        case GETALL: {
          break;
        }
        case GETNCNT: {
          break;
        }
        case GETPID: {
          break;
        }
        case GETVAL: {
          break;
        }
        case GETZCNT: {
          break;
        }
        case SETALL: {
          break;
        }
        case SETVAL: {
          break;
        }
        // Forward
        case IPC_RMID: {
          Result = ::semctl(semid, semnum, cmd, semun);
          break;
        }
        default: LOGMAN_MSG_A("Unhandled shmctl cmd: %d", cmd); return -EINVAL; break;
      }
      SYSCALL_ERRNO();
    });
  }
}
