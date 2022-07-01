#pragma once

#define REGISTER_SYSCALL_NOT_IMPL(name) REGISTER_SYSCALL_IMPL(name, [](FEXCore::Core::CpuStateFrame *Frame) -> uint64_t { \
  LogMan::Msg::DFmt("Using deprecated/removed syscall: " #name); \
  return -ENOSYS; \
});


#define REGISTER_SYSCALL_NO_PERM(name) REGISTER_SYSCALL_IMPL(name, [](FEXCore::Core::CpuStateFrame *Frame) -> uint64_t { \
  return -EPERM; \
});

#define REGISTER_SYSCALL_NO_PERM_X32(name) REGISTER_SYSCALL_IMPL_X32(name, [](FEXCore::Core::CpuStateFrame *Frame) -> uint64_t { \
  return -EPERM; \
});

namespace FEX::HLE::Android {
  void RegisterFS();
  void RegisterInfo();
  void RegisterTime();
  void RegisterThread();

  namespace x32 {
    void RegisterFS();
    void RegisterThread();
  }
  namespace x64 {
    void RegisterThread();
  }

}
