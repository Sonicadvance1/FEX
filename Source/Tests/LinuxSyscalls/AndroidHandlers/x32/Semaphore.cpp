/*
$info$
tags: LinuxSyscalls|android-syscalls-x86-32
$end_info$
*/

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Types.h"

#include "Tests/LinuxSyscalls/x64/Syscalls.h"

#include <FEXCore/Utils/LogManager.h>
#include <FEXHeaderUtils/Syscalls.h>

#include <cstdint>
#include <errno.h>
#include <limits>
#include <string.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include <type_traits>
#include <vector>

namespace FEXCore::Core {
  struct CpuStateFrame;
}

namespace FEX::HLE::x32 {
  // Define the IPC ops
  enum IPCOp {
    OP_SEMOP      = 1,
    OP_SEMGET     = 2,
    OP_SEMCTL     = 3,
    OP_SEMTIMEDOP = 4,
    OP_MSGSND     = 11,
    OP_MSGRCV     = 12,
    OP_MSGGET     = 13,
    OP_MSGCTL     = 14,
    OP_SHMAT      = 21,
    OP_SHMDT      = 22,
    OP_SHMGET     = 23,
    OP_SHMCTL     = 24,
  };

  struct msgbuf_32 {
    compat_long_t mtype;
    char mtext[1];
  };

  union semun_32 {
    int32_t val;      // Value for SETVAL
    compat_ptr<semid_ds_32> buf32; // struct semid_ds* - Buffer ptr for IPC_STAT, IPC_SET
    compat_ptr<semid_ds_64> buf64; // struct semid_ds* - Buffer ptr for IPC_STAT, IPC_SET
    uint32_t array;   // uint16_t array for GETALL, SETALL
    compat_ptr<struct fex_seminfo> __buf;   // struct seminfo * - Buffer for IPC_INFO
  };

  union msgun_32 {
    int32_t val;      // Value for SETVAL
    compat_ptr<msqid_ds_32> buf32; // struct msgid_ds* - Buffer ptr for IPC_STAT, IPC_SET
    compat_ptr<msqid_ds_64> buf64; // struct msgid_ds* - Buffer ptr for IPC_STAT, IPC_SET
    uint32_t array;   // uint16_t array for GETALL, SETALL
    compat_ptr<struct msginfo> __buf;   // struct msginfo * - Buffer for IPC_INFO
  };

  union shmun_32 {
    int32_t val;      // Value for SETVAL
    compat_ptr<shmid_ds_32> buf32; // struct shmid_ds* - Buffer ptr for IPC_STAT, IPC_SET
    compat_ptr<shmid_ds_64> buf64; // struct shmid_ds* - Buffer ptr for IPC_STAT, IPC_SET
    uint32_t array;   // uint16_t array for GETALL, SETALL
    compat_ptr<struct shminfo_32> __buf32;   // struct shminfo * - Buffer for IPC_INFO
    compat_ptr<struct shminfo_64> __buf64;   // struct shminfo * - Buffer for IPC_INFO

    compat_ptr<struct shm_info_32> __buf_info_32;   // struct shm_info * - Buffer for SHM_INFO
  };

  union semun {
    int val;                   /* value for SETVAL */
    struct semid_ds_32 *buf;   /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;     /* array for GETALL & SETALL */
    struct fex_seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
  };

  uint64_t _ipc(FEXCore::Core::CpuStateFrame *Frame, uint32_t call, uint32_t first, uint32_t second, uint32_t third, uint32_t ptr, uint32_t fifth) {
    uint64_t Result{};

    switch (static_cast<IPCOp>(call)) {
      case OP_SEMOP: {
        return -ENOSYS;
        break;
      }
      case OP_SEMGET: {
        return -ENOSYS;
        break;
      }
      case OP_SEMCTL: {
        return -ENOSYS;
        break;
      }
      case OP_SEMTIMEDOP: {
        return -ENOSYS;
        break;
      }
      case OP_MSGSND: {
        return -ENOSYS;
        break;
      }
      case OP_MSGRCV: {
        return -ENOSYS;
        break;
      }
      case OP_MSGGET: {
        return -ENOSYS;
        break;
      }
      case OP_MSGCTL: {
        return -ENOSYS;
        break;
      }
      case OP_SHMAT: {
        // also implemented in memory:shmat
        Result = static_cast<FEX::HLE::x32::x32SyscallHandler*>(FEX::HLE::_SyscallHandler)->GetAllocator()->
          Shmat(first, reinterpret_cast<const void*>(ptr), second, reinterpret_cast<uint32_t*>(third));
        if (!FEX::HLE::HasSyscallError(Result)) {
          FEX::HLE::_SyscallHandler->TrackShmat(first, *reinterpret_cast<uint32_t*>(third), second);
        }
        break;
      }
      case OP_SHMDT: {
        // also implemented in memory:shmdt
        Result = static_cast<FEX::HLE::x32::x32SyscallHandler*>(FEX::HLE::_SyscallHandler)->GetAllocator()->
          Shmdt(reinterpret_cast<void*>(ptr));
        if (!FEX::HLE::HasSyscallError(Result)) {
          FEX::HLE::_SyscallHandler->TrackShmdt(ptr);
        }
        break;
      }
      case OP_SHMGET: {
        Result = ::shmget(first, second, third);
        break;
      }
      case OP_SHMCTL: {
        int32_t shmid = first;
        int32_t shmcmd = second;
        int32_t cmd = shmcmd & 0xFF;
        bool IPC64 = shmcmd & 0x100;
        shmun_32 shmun{};
        shmun.val = reinterpret_cast<uint32_t>(ptr);

        switch (cmd) {
          case IPC_SET: {
            struct shmid64_ds buf{};
            if (IPC64) {
              buf = *shmun.buf64;
            }
            else {
              buf = *shmun.buf32;
            }
            Result = ::shmctl(shmid, cmd, &buf);
            // IPC_SET sets the internal data structure that the kernel uses
            // No need to writeback
            break;
          }
          case SHM_STAT:
          case SHM_STAT_ANY:
          case IPC_STAT: {
            struct shmid64_ds buf{};
            Result = ::shmctl(shmid, cmd, &buf);
            if (Result != -1) {
              if (IPC64) {
                *shmun.buf64 = buf;
              }
              else {
                *shmun.buf32 = buf;
              }
            }
            break;
          }
          case IPC_INFO: {
            struct shminfo si{};
            Result = ::shmctl(shmid, cmd, reinterpret_cast<struct shmid_ds*>(&si));
            if (Result != -1) {
              if (IPC64) {
                *shmun.__buf64 = si;
              }
              else {
                *shmun.__buf32 = si;
              }
            }
            break;
          }
          case SHM_INFO: {
            struct shm_info si{};
            Result = ::shmctl(shmid, cmd, reinterpret_cast<struct shmid_ds*>(&si));
            if (Result != -1) {
              // SHM_INFO doesn't follow IPC64 behaviour
              *shmun.__buf_info_32 = si;
            }
            break;
          }
          case SHM_LOCK:
            Result = ::shmctl(shmid, cmd, nullptr);
            break;
          case SHM_UNLOCK:
            Result = ::shmctl(shmid, cmd, nullptr);
            break;
          case IPC_RMID:
            Result = ::shmctl(shmid, cmd, nullptr);
            break;

          default:
            LOGMAN_MSG_A_FMT("Unhandled shmctl cmd: {}", cmd);
            return -EINVAL;
        }
        break;
      }

      default: return -ENOSYS;
    }
    SYSCALL_ERRNO();
  }
  void RegisterSemaphore() {
    REGISTER_SYSCALL_IMPL_X32(ipc, _ipc);

    REGISTER_SYSCALL_IMPL_X32_PASS_MANUAL(semtimedop_time64, semtimedop, [](FEXCore::Core::CpuStateFrame *Frame, int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout) -> uint64_t {
      return -ENOSYS;
    });

    REGISTER_SYSCALL_IMPL_X32(semctl, [](FEXCore::Core::CpuStateFrame *Frame, int semid, int semnum, int cmd, semun_32 *semun) -> uint64_t {
      return -ENOSYS;
    });
  }
}
