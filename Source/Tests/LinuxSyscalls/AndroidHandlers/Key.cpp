/*
$info$
tags: LinuxSyscalls|syscalls-shared
$end_info$
*/

#include "Tests/LinuxSyscalls/Syscalls.h"
#include "Tests/LinuxSyscalls/Types.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"

#include <FEXCore/IR/IR.h>

#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace FEX::HLE::Android {
  void RegisterKey() {
    using namespace FEXCore::IR;

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(add_key, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, const char *type, const char *description, const void *payload, size_t plen, key_serial_t keyring) -> uint64_t {
      return -EACCES;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(request_key, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, const char *type, const char *description, const char *callout_info, key_serial_t dest_keyring) -> uint64_t {
      return -EACCES;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(keyctl, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, int operation, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) -> uint64_t {
      return -EACCES;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(pkey_mprotect, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, void *addr, size_t len, int prot, int pkey) -> uint64_t {
      return -EINVAL;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(pkey_alloc, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, unsigned int flags, unsigned int access_rights) -> uint64_t {
      return -ENOSPC;
    });

    REGISTER_SYSCALL_IMPL_PASS_FLAGS(pkey_free, SyscallFlags::OPTIMIZETHROUGH | SyscallFlags::NOSYNCSTATEONENTRY,
      [](FEXCore::Core::CpuStateFrame *Frame, int pkey) -> uint64_t {
      return -EINVAL;
    });
  }
}
