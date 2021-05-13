/*
$info$
tags: LinuxSyscalls|syscalls-x86-32
$end_info$
*/

#include "Common/MathUtils.h"

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/IoctlEmulation.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

#include <FEXCore/Debug/InternalThreadState.h>
#include <FEXCore/Utils/LogManager.h>

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/file.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/statfs.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#include <sys/uio.h>
#include <sys/vfs.h>
#include <unistd.h>

ARG_TO_STR(FEX::HLE::x32::compat_ptr<FEX::HLE::x32::sigset_argpack32>, "%lx")

namespace FEX::HLE::x32 {
#ifdef _M_X86_64
  uint32_t ioctl_32(int fd, uint32_t cmd, uint32_t args) {
    uint32_t Result{};
    __asm volatile("int $0x80;"
        : "=a" (Result)
        : "a" (SYSCALL_x86_ioctl)
        , "b" (fd)
        , "c" (cmd)
        , "d" (args)
        : "memory");
    return Result;
  }
#else
	constexpr uint32_t arm_SYS_ioctl =  54;
	__attribute__((naked))
	__attribute__((noinline))
	uint32_t ioctl_32(FEXCore::Core::CpuStateFrame*, int fd, uint32_t request, uint32_t args) {
	__asm volatile(
			R"(
				uxtw x0, w1;
				uxtw x1, w2;
				uxtw x2, w3;
				movz w8, %[Syscall];
				orr w8, w8, #0x80000000;
				svc 0x0;
				ret;
				)"
			:: [Syscall] "i" (arm_SYS_ioctl)
			: "memory");
	}
#endif
  using fd_set32 = uint32_t;
#ifdef _M_X86_64
  uint32_t ioctl_32(FEXCore::Core::CpuStateFrame*, int fd, uint32_t cmd, uint32_t args) {
    uint32_t Result{};
    __asm volatile("int $0x80;"
        : "=a" (Result)
        : "a" (SYSCALL_x86_ioctl)
        , "b" (fd)
        , "c" (cmd)
        , "d" (args)
        : "memory");
    return Result;
  }
#endif

  void RegisterFD() {
    REGISTER_SYSCALL_IMPL_X32(poll, [](FEXCore::Core::CpuStateFrame *Frame, struct pollfd *fds, nfds_t nfds, int timeout) -> uint64_t {
      uint64_t Result = ::poll(fds, nfds, timeout);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(ppoll, [](FEXCore::Core::CpuStateFrame *Frame, struct pollfd *fds, nfds_t nfds, timespec32 *timeout_ts, const uint64_t *sigmask, size_t sigsetsize) -> uint64_t {
      // sigsetsize is unused here since it is currently a constant and not exposed through glibc
      struct timespec tp64{};
      struct timespec *timed_ptr{};
      if (timeout_ts) {
        tp64 = *timeout_ts;
        timed_ptr = &tp64;
      }

      sigset_t HostSet{};

      if (sigmask) {
        sigemptyset(&HostSet);

        for (int32_t i = 0; i < (sigsetsize * 8); ++i) {
          if (*sigmask & (1ULL << i)) {
            sigaddset(&HostSet, i + 1);
          }
        }
      }

      uint64_t Result = ppoll(
        fds,
        nfds,
        timed_ptr,
        sigmask ? &HostSet : nullptr);

      if (timeout_ts) {
        *timeout_ts = tp64;
      }

      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(ppoll_time64, [](FEXCore::Core::CpuStateFrame *Frame, struct pollfd *fds, nfds_t nfds, struct timespec *timeout_ts, const uint64_t *sigmask, size_t sigsetsize) -> uint64_t {
      // sigsetsize is unused here since it is currently a constant and not exposed through glibc
      sigset_t HostSet{};

      if (sigmask) {
        sigemptyset(&HostSet);

        for (int32_t i = 0; i < (sigsetsize * 8); ++i) {
          if (*sigmask & (1ULL << i)) {
            sigaddset(&HostSet, i + 1);
          }
        }
      }

      uint64_t Result = ppoll(
        fds,
        nfds,
        timeout_ts,
        sigmask ? &HostSet : nullptr);

      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(_llseek, [](FEXCore::Core::CpuStateFrame *Frame, uint32_t fd, uint32_t offset_high, uint32_t offset_low, loff_t *result, uint32_t whence) -> uint64_t {
      uint64_t Offset = offset_high;
      Offset <<= 32;
      Offset |= offset_low;
      uint64_t Result = lseek(fd, Offset, whence);
      if (Result != -1) {
        *result = Result;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(readv, [](FEXCore::Core::CpuStateFrame *Frame, int fd, const struct iovec32 *iov, int iovcnt) -> uint64_t {
      std::vector<iovec> Host_iovec(iovcnt);
      for (int i = 0; i < iovcnt; ++i) {
        Host_iovec[i] = iov[i];
      }

      uint64_t Result = ::readv(fd, &Host_iovec.at(0), iovcnt);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(writev, [](FEXCore::Core::CpuStateFrame *Frame, int fd, const struct iovec32 *iov, int iovcnt) -> uint64_t {
      std::vector<iovec> Host_iovec(iovcnt);
      for (int i = 0; i < iovcnt; ++i) {
        Host_iovec[i] = iov[i];
      }
      uint64_t Result = ::writev(fd, &Host_iovec.at(0), iovcnt);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(stat, [](FEXCore::Core::CpuStateFrame *Frame, const char *pathname, stat32 *buf) -> uint64_t {
      struct stat host_stat;
      uint64_t Result = FEX::HLE::_SyscallHandler->FM.Stat(pathname, &host_stat);
      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(fstat, [](FEXCore::Core::CpuStateFrame *Frame, int fd, stat32 *buf) -> uint64_t {
      struct stat host_stat;
      uint64_t Result = ::fstat(fd, &host_stat);
      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(lstat, [](FEXCore::Core::CpuStateFrame *Frame, const char *path, stat32 *buf) -> uint64_t {
      struct stat host_stat;
      uint64_t Result = FEX::HLE::_SyscallHandler->FM.Lstat(path, &host_stat);
      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(stat64, [](FEXCore::Core::CpuStateFrame *Frame, const char *pathname, stat64_32 *buf) -> uint64_t {
      struct stat host_stat;
      uint64_t Result = FEX::HLE::_SyscallHandler->FM.Stat(pathname, &host_stat);
      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(lstat64, [](FEXCore::Core::CpuStateFrame *Frame, const char *path, stat64_32 *buf) -> uint64_t {
      struct stat host_stat;
      uint64_t Result = FEX::HLE::_SyscallHandler->FM.Lstat(path, &host_stat);

      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(fstat64, [](FEXCore::Core::CpuStateFrame *Frame, int fd, stat64_32 *buf) -> uint64_t {
      struct stat64 host_stat;
      uint64_t Result = ::fstat64(fd, &host_stat);
      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(fstatfs64, [](FEXCore::Core::CpuStateFrame *Frame, int fd, size_t sz, struct statfs64_32 *buf) -> uint64_t {
      LOGMAN_THROW_A(sz == sizeof(struct statfs64_32), "This needs to match");

      struct statfs64 host_stat;
      uint64_t Result = ::fstatfs64(fd, &host_stat);
      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(statfs64, [](FEXCore::Core::CpuStateFrame *Frame, const char *path, size_t sz, struct statfs64_32 *buf) -> uint64_t {
      LOGMAN_THROW_A(sz == sizeof(struct statfs64_32), "This needs to match");

      struct statfs host_stat;
      uint64_t Result = FEX::HLE::_SyscallHandler->FM.Statfs(path, &host_stat);
      if (Result != -1) {
        *buf = host_stat;
      }

      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(fcntl64, [](FEXCore::Core::CpuStateFrame *Frame, int fd, int cmd, uint64_t arg) -> uint64_t {
      // fcntl64 struct directly matches the 64bit fcntl op
      // cmd just needs to be fixed up
      // These are redefined to be their non-64bit tagged value on x86-64
      constexpr int OP_GETLK64_32 = 12;
      constexpr int OP_SETLK64_32 = 13;
      constexpr int OP_SETLKW64_32 = 14;

      void *lock_arg = (void*)arg;
      struct flock tmp{};

      switch (cmd) {
        case OP_GETLK64_32: {
          cmd = F_GETLK;
          lock_arg = (void*)&tmp;
          tmp = *reinterpret_cast<flock64_32*>(arg);
          break;
        }
        case OP_SETLK64_32: {
          cmd = F_SETLK;
          lock_arg = (void*)&tmp;
          tmp = *reinterpret_cast<flock64_32*>(arg);
          break;
        }
        case OP_SETLKW64_32: {
          cmd = F_SETLKW;
          lock_arg = (void*)&tmp;
          tmp = *reinterpret_cast<flock64_32*>(arg);
          break;
        }
        case F_OFD_SETLK:
        case F_OFD_GETLK:
        case F_OFD_SETLKW: {
          lock_arg = (void*)&tmp;
          tmp = *reinterpret_cast<flock64_32*>(arg);
          break;
        }
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW: {
          lock_arg = (void*)&tmp;
          tmp = *reinterpret_cast<flock_32*>(arg);
          break;
        }

        case F_SETFL:
          lock_arg = (void*)FEX::HLE::RemapFromX86Flags(arg);
          break;
        // Maps directly
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_GETFD:
        case F_SETFD:
        case F_GETFL:
          break;

        default: LOGMAN_MSG_A("Unhandled fcntl64: 0x%x", cmd); break;
      }

      uint64_t Result = ::fcntl(fd, cmd, lock_arg);

      if (Result != -1) {
        switch (cmd) {
          case OP_GETLK64_32: {
            *reinterpret_cast<flock64_32*>(arg) = tmp;
            break;
          }
          case F_OFD_GETLK: {
            *reinterpret_cast<flock64_32*>(arg) = tmp;
            break;
          }
          case F_GETLK: {
            *reinterpret_cast<flock_32*>(arg) = tmp;
            break;
          }
          break;
          case F_DUPFD:
          case F_DUPFD_CLOEXEC:
            FEX::HLE::x32::CheckAndAddFDDuplication(fd, Result);
            break;
          case F_GETFL: {
            Result = FEX::HLE::RemapToX86Flags(Result);
            break;
          }
          default: break;
        }
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(dup, [](FEXCore::Core::CpuStateFrame *Frame, int oldfd) -> uint64_t {
      uint64_t Result = ::dup(oldfd);
      if (Result != -1) {
        CheckAndAddFDDuplication(oldfd, Result);
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(dup2, [](FEXCore::Core::CpuStateFrame *Frame, int oldfd, int newfd) -> uint64_t {
      uint64_t Result = ::dup2(oldfd, newfd);
      if (Result != -1) {
        CheckAndAddFDDuplication(oldfd, newfd);
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(preadv, [](FEXCore::Core::CpuStateFrame *Frame, int fd, const struct iovec32 *iov, int iovcnt, off_t offset) -> uint64_t {
      std::vector<iovec> Host_iovec(iovcnt);
      for (int i = 0; i < iovcnt; ++i) {
        Host_iovec[i] = iov[i];
      }

      uint64_t Result = ::preadv(fd, &Host_iovec.at(0), iovcnt, offset);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(pwritev, [](FEXCore::Core::CpuStateFrame *Frame, int fd, const struct iovec32 *iov, int iovcnt, off_t offset) -> uint64_t {
      std::vector<iovec> Host_iovec(iovcnt);
      for (int i = 0; i < iovcnt; ++i) {
        Host_iovec[i] = iov[i];
      }

      uint64_t Result = ::pwritev(fd, &Host_iovec.at(0), iovcnt, offset);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(process_vm_readv, [](FEXCore::Core::CpuStateFrame *Frame, pid_t pid, const struct iovec32 *local_iov, unsigned long liovcnt, const struct iovec32 *remote_iov, unsigned long riovcnt, unsigned long flags) -> uint64_t {
      std::vector<iovec> Host_local_iovec(liovcnt);
      std::vector<iovec> Host_remote_iovec(riovcnt);

      for (int i = 0; i < liovcnt; ++i) {
        Host_local_iovec[i] = local_iov[i];
      }

      for (int i = 0; i < riovcnt; ++i) {
        Host_remote_iovec[i] = remote_iov[i];
      }

      uint64_t Result = ::process_vm_readv(pid, &Host_local_iovec.at(0), liovcnt, &Host_remote_iovec.at(0), riovcnt, flags);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(process_vm_writev, [](FEXCore::Core::CpuStateFrame *Frame, pid_t pid, const struct iovec32 *local_iov, unsigned long liovcnt, const struct iovec32 *remote_iov, unsigned long riovcnt, unsigned long flags) -> uint64_t {
      std::vector<iovec> Host_local_iovec(liovcnt);
      std::vector<iovec> Host_remote_iovec(riovcnt);

      for (int i = 0; i < liovcnt; ++i) {
        Host_local_iovec[i] = local_iov[i];
      }

      for (int i = 0; i < riovcnt; ++i) {
        Host_remote_iovec[i] = remote_iov[i];
      }

      uint64_t Result = ::process_vm_writev(pid, &Host_local_iovec.at(0), liovcnt, &Host_remote_iovec.at(0), riovcnt, flags);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(preadv2, [](FEXCore::Core::CpuStateFrame *Frame, int fd, const struct iovec32 *iov, int iovcnt, off_t offset, int flags) -> uint64_t {
      std::vector<iovec> Host_iovec(iovcnt);
      for (int i = 0; i < iovcnt; ++i) {
        Host_iovec[i] = iov[i];
      }

      uint64_t Result = ::preadv2(fd, &Host_iovec.at(0), iovcnt, offset, flags);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(pwritev2, [](FEXCore::Core::CpuStateFrame *Frame, int fd, const struct iovec32 *iov, int iovcnt, off_t offset, int flags) -> uint64_t {
      std::vector<iovec> Host_iovec(iovcnt);
      for (int i = 0; i < iovcnt; ++i) {
        Host_iovec[i] = iov[i];
      }

      uint64_t Result = ::pwritev2(fd, &Host_iovec.at(0), iovcnt, offset, flags);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(fstatat64, [](FEXCore::Core::CpuStateFrame *Frame, int dirfd, const char *pathname, stat64_32 *buf, int flag) -> uint64_t {
      struct stat64 host_stat;
      uint64_t Result = FEX::HLE::_SyscallHandler->FM.NewFSStatAt64(dirfd, pathname, &host_stat, flag);
      if (Result != -1) {
        *buf = host_stat;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(ioctl, ioctl32);

    REGISTER_SYSCALL_IMPL_X32(getdents, [](FEXCore::Core::CpuStateFrame *Frame, int fd, void *dirp, uint32_t count) -> uint64_t {
#ifdef SYS_getdents
      std::vector<uint8_t> TmpVector(count);
      void *TmpPtr = reinterpret_cast<void*>(&TmpVector.at(0));

      // Copy the incoming structures to our temporary array
      for (uint64_t Offset = 0, TmpOffset = 0;
          Offset < count;) {
        linux_dirent_32 *Incoming = (linux_dirent_32*)(reinterpret_cast<uint64_t>(dirp) + Offset);
        linux_dirent *Tmp = (linux_dirent*)(reinterpret_cast<uint64_t>(TmpPtr) + TmpOffset);

        if (!Incoming->d_reclen ||
            (Offset + Incoming->d_reclen) > count) {
          break;
        }

        size_t NewRecLen = Incoming->d_reclen + (sizeof(linux_dirent) - sizeof(linux_dirent_32));
        Tmp->d_ino    = Incoming->d_ino;
        Tmp->d_off    = Incoming->d_off;
        Tmp->d_reclen = NewRecLen;

        // This actually copies two more bytes than the string of d_name
        // Copies a null byte for the string
        // Copies a d_type flag that lives after the name
        size_t CopySize = std::clamp<uint32_t>(Incoming->d_reclen - offsetof(linux_dirent_32, d_name), 0U, count - Offset);
        memcpy(Tmp->d_name, Incoming->d_name, CopySize);

        // We take up 8 more bytes of space
        TmpOffset += NewRecLen;
        Offset += Incoming->d_reclen;
      }

      uint64_t Result = syscall(SYS_getdents,
        static_cast<uint64_t>(fd),
        TmpPtr,
        static_cast<uint64_t>(count));

      // Now copy back in to the array we were given
      if (Result != -1) {
        uint64_t Offset = 0;
        // With how the emulation occurs we will always return a smaller buffer than what was given to us
        for (uint64_t TmpOffset = 0, num = 0; TmpOffset < Result; ++num) {
          linux_dirent_32 *Outgoing = (linux_dirent_32*)(reinterpret_cast<uint64_t>(dirp) + Offset);
          linux_dirent *Tmp = (linux_dirent*)(reinterpret_cast<uint64_t>(TmpPtr) + TmpOffset);

          if (!Tmp->d_reclen) {
            break;
          }

          size_t NewRecLen = Tmp->d_reclen - (sizeof(std::remove_reference<decltype(*Tmp)>::type) - sizeof(*Outgoing));
          Outgoing->d_ino = Tmp->d_ino;
          // If we pass d_off directly then we seem to encounter issues?
          Outgoing->d_off = num; //Tmp->d_off;
          size_t OffsetOfName = offsetof(std::remove_reference<decltype(*Tmp)>::type, d_name);
          Outgoing->d_reclen = NewRecLen;

          // Copies null character and d_type flag as well
          memcpy(Outgoing->d_name, Tmp->d_name, Tmp->d_reclen - OffsetOfName);

          TmpOffset += Tmp->d_reclen;
          // Outgoing is 8 bytes smaller
          Offset += NewRecLen;
        }
        Result = Offset;
      }
      SYSCALL_ERRNO();
#else
      // XXX: Emulate
      return -ENOSYS;
#endif
    });

    REGISTER_SYSCALL_IMPL_X32(getdents64, [](FEXCore::Core::CpuStateFrame *Frame, int fd, void *dirp, uint32_t count) -> uint64_t {
      uint64_t Result = ::syscall(SYS_getdents64,
        static_cast<uint64_t>(fd),
        dirp,
        static_cast<uint64_t>(count));
      if (Result != -1) {
        // Walk each offset
        // if we are passing the full d_off to the 32bit application then it seems to break things?
        for (size_t i = 0, num = 0; i < Result; ++num) {
          linux_dirent_64 *Incoming = (linux_dirent_64*)(reinterpret_cast<uint64_t>(dirp) + i);
          Incoming->d_off = num;
          i += Incoming->d_reclen;
        }
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(_newselect, [](FEXCore::Core::CpuStateFrame *Frame, int nfds, fd_set32 *readfds, fd_set32 *writefds, fd_set32 *exceptfds, struct timeval32 *timeout) -> uint64_t {
      struct timeval tp64{};
      if (timeout) {
        tp64 = *timeout;
      }

      fd_set Host_readfds;
      fd_set Host_writefds;
      fd_set Host_exceptfds;
      FD_ZERO(&Host_readfds);
      FD_ZERO(&Host_writefds);
      FD_ZERO(&Host_exceptfds);

      // Round up to the full 32bit word
      uint32_t NumWords = AlignUp(nfds, 32) / 4;

      if (readfds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = readfds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_readfds);
            }
          }
        }
      }

      if (writefds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = writefds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_writefds);
            }
          }
        }
      }

      if (exceptfds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = exceptfds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_exceptfds);
            }
          }
        }
      }

      uint64_t Result = ::select(nfds,
        readfds ? &Host_readfds : nullptr,
        writefds ? &Host_writefds : nullptr,
        exceptfds ? &Host_exceptfds : nullptr,
        timeout ? &tp64 : nullptr);
      if (readfds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_readfds)) {
            readfds[i/32] |= 1 << (i & 31);
          } else {
            readfds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (writefds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_writefds)) {
            writefds[i/32] |= 1 << (i & 31);
          } else {
            writefds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (exceptfds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_exceptfds)) {
            exceptfds[i/32] |= 1 << (i & 31);
          } else {
            exceptfds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (timeout) {
        *timeout = tp64;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(pselect6, [](FEXCore::Core::CpuStateFrame *Frame, int nfds, fd_set32 *readfds, fd_set32 *writefds, fd_set32 *exceptfds, timespec32 *timeout, compat_ptr<sigset_argpack32> sigmaskpack) -> uint64_t {
      struct timespec tp64{};
      if (timeout) {
        tp64 = *timeout;
      }

      fd_set Host_readfds;
      fd_set Host_writefds;
      fd_set Host_exceptfds;
      sigset_t HostSet{};

      FD_ZERO(&Host_readfds);
      FD_ZERO(&Host_writefds);
      FD_ZERO(&Host_exceptfds);
      sigemptyset(&HostSet);

      // Round up to the full 32bit word
      uint32_t NumWords = AlignUp(nfds, 32) / 4;

      if (readfds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = readfds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_readfds);
            }
          }
        }
      }

      if (writefds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = writefds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_writefds);
            }
          }
        }
      }

      if (exceptfds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = exceptfds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_exceptfds);
            }
          }
        }
      }

      if (sigmaskpack) {
        uint64_t *sigmask = sigmaskpack->sigset;
        size_t sigsetsize = sigmaskpack->size;
        for (int32_t i = 0; i < (sigsetsize * 8); ++i) {
          if (*sigmask & (1ULL << i)) {
            sigaddset(&HostSet, i + 1);
          }
        }
      }

      uint64_t Result = ::pselect(nfds,
        readfds ? &Host_readfds : nullptr,
        writefds ? &Host_writefds : nullptr,
        exceptfds ? &Host_exceptfds : nullptr,
        timeout ? &tp64 : nullptr,
        &HostSet);

      if (readfds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_readfds)) {
            readfds[i/32] |= 1 << (i & 31);
          } else {
            readfds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (writefds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_writefds)) {
            writefds[i/32] |= 1 << (i & 31);
          } else {
            writefds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (exceptfds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_exceptfds)) {
            exceptfds[i/32] |= 1 << (i & 31);
          } else {
            exceptfds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (timeout) {
        *timeout = tp64;
      }
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(fadvise64_64, [](FEXCore::Core::CpuStateFrame *Frame, int32_t fd, uint32_t offset_low, uint32_t offset_high, uint32_t len_low, uint32_t len_high, int advice) -> uint64_t {
      uint64_t Offset = offset_high;
      Offset <<= 32;
      Offset |= offset_low;
      uint64_t Len = len_high;
      Len <<= 32;
      Len |= len_low;
      uint64_t Result = ::posix_fadvise64(fd, Offset, Len, advice);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(timerfd_settime64, [](FEXCore::Core::CpuStateFrame *Frame, int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value) -> uint64_t {
      uint64_t Result = ::timerfd_settime(fd, flags, new_value, old_value);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(timerfd_gettime64, [](FEXCore::Core::CpuStateFrame *Frame, int fd, struct itimerspec *curr_value) -> uint64_t {
      uint64_t Result = ::timerfd_gettime(fd, curr_value);
      SYSCALL_ERRNO();
    });

    REGISTER_SYSCALL_IMPL_X32(pselect6_time64, [](FEXCore::Core::CpuStateFrame *Frame, int nfds, fd_set32 *readfds, fd_set32 *writefds, fd_set32 *exceptfds, struct timespec *timeout, compat_ptr<sigset_argpack32> sigmaskpack) -> uint64_t {
      fd_set Host_readfds;
      fd_set Host_writefds;
      fd_set Host_exceptfds;
      sigset_t HostSet{};

      FD_ZERO(&Host_readfds);
      FD_ZERO(&Host_writefds);
      FD_ZERO(&Host_exceptfds);
      sigemptyset(&HostSet);

      // Round up to the full 32bit word
      uint32_t NumWords = AlignUp(nfds, 32) / 4;

      if (readfds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = readfds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_readfds);
            }
          }
        }
      }

      if (writefds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = writefds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_writefds);
            }
          }
        }
      }

      if (exceptfds) {
        for (int i = 0; i < NumWords; ++i) {
          uint32_t FD = exceptfds[i];
          int32_t Rem = nfds - (i * 32);
          for (int j = 0; j < 32 && j < Rem; ++j) {
            if ((FD >> j) & 1) {
              FD_SET(i * 32 + j, &Host_exceptfds);
            }
          }
        }
      }

      if (sigmaskpack) {
        uint64_t *sigmask = sigmaskpack->sigset;
        size_t sigsetsize = sigmaskpack->size;
        for (int32_t i = 0; i < (sigsetsize * 8); ++i) {
          if (*sigmask & (1ULL << i)) {
            sigaddset(&HostSet, i + 1);
          }
        }
      }

      uint64_t Result = ::pselect(nfds,
        readfds ? &Host_readfds : nullptr,
        writefds ? &Host_writefds : nullptr,
        exceptfds ? &Host_exceptfds : nullptr,
        timeout,
        &HostSet);

      if (readfds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_readfds)) {
            readfds[i/32] |= 1 << (i & 31);
          } else {
            readfds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (writefds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_writefds)) {
            writefds[i/32] |= 1 << (i & 31);
          } else {
            writefds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      if (exceptfds) {
        for (int i = 0; i < nfds; ++i) {
          if (FD_ISSET(i, &Host_exceptfds)) {
            exceptfds[i/32] |= 1 << (i & 31);
          } else {
            exceptfds[i/32] &= ~(1 << (i & 31));
          }
        }
      }

      SYSCALL_ERRNO();
    });
  }

  REGISTER_SYSCALL_IMPL_X32(sendfile, [](FEXCore::Core::CpuStateFrame *Frame, int out_fd, int in_fd, compat_off_t *offset, size_t count) -> uint64_t {
    off_t Local{};
    off_t *Local_p{};
    if (offset) {
      Local_p = &Local;
      Local = *offset;
    }
    uint64_t Result = ::sendfile(out_fd, in_fd, Local_p, count);
    SYSCALL_ERRNO();
  });


  REGISTER_SYSCALL_IMPL_X32(sendfile64, [](FEXCore::Core::CpuStateFrame *Frame, int out_fd, int in_fd, off_t *offset, compat_size_t count) -> uint64_t {
    // Linux definition for this is a bit confusing
    // Defines offset as compat_loff_t* but loads loff_t worth of data
    // count is defined as compat_size_t still
    uint64_t Result = ::sendfile(out_fd, in_fd, offset, count);
    SYSCALL_ERRNO();
  });
}
