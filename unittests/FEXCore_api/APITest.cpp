#include "FEXCore/Core/SignalDelegator.h"
#include <FEXCore/Core/CodeLoader.h>
#include <FEXCore/Core/Context.h>
#include <FEXCore/Core/X86Enums.h>
#include <FEXCore/Debug/InternalThreadState.h>
#include <FEXCore/HLE/SyscallHandler.h>

#include <csetjmp>
#include <dlfcn.h>
#include <signal.h>

#define FEXCORE_FORWARDDECL_DEFINES
#define FEXCORE_IMPL_DEFINES
#define FEXCORE_LOADER
#include "LoaderAPI.h"

extern "C" {
// Needs to be inline to avoid overriding FEXCore symbols.
#ifdef ENABLE_JEMALLOC
#define JEMALLOC_NOTHROW __attribute__((nothrow))
  JEMALLOC_NOTHROW inline void *je_malloc(size_t size) { return FCL::je_malloc(size); }
  JEMALLOC_NOTHROW inline void *je_calloc(size_t n, size_t size) { return FCL::je_calloc(n, size); }
  JEMALLOC_NOTHROW inline void *je_memalign(size_t align, size_t s) { return FCL::je_memalign(align, s); }
  JEMALLOC_NOTHROW inline void *je_valloc(size_t size) { return FCL::je_valloc(size); }
  JEMALLOC_NOTHROW inline int je_posix_memalign(void** r, size_t a, size_t s) { return FCL::je_posix_memalign(r, a, s); }
  JEMALLOC_NOTHROW inline void *je_realloc(void* ptr, size_t size) { return FCL::je_realloc(ptr, size); }
  JEMALLOC_NOTHROW inline void je_free(void* ptr) { return FCL::je_free(ptr); }
  JEMALLOC_NOTHROW inline size_t je_malloc_usable_size(void *ptr) { return FCL::je_malloc_usable_size(ptr); }
  JEMALLOC_NOTHROW inline void *je_aligned_alloc(size_t a, size_t s) { return FCL::je_aligned_alloc(a, s); }
  JEMALLOC_NOTHROW inline void je_aligned_free(void* ptr) { return FCL::je_free(ptr); }
#endif
}

namespace {
static int OutputFD {STDERR_FILENO};

void MsgHandler(LogMan::DebugLevels Level, char const *Message) {
  const char *CharLevel{nullptr};

  switch (Level) {
  case LogMan::NONE:
    CharLevel = "NONE";
    break;
  case LogMan::ASSERT:
    CharLevel = "ASSERT";
    break;
  case LogMan::ERROR:
    CharLevel = "ERROR";
    break;
  case LogMan::DEBUG:
    CharLevel = "DEBUG";
    break;
  case LogMan::INFO:
    CharLevel = "Info";
    break;
  case LogMan::STDOUT:
    CharLevel = "STDOUT";
    break;
  case LogMan::STDERR:
    CharLevel = "STDERR";
    break;
  default:
    CharLevel = "???";
    break;
  }

  const auto Output = fmt::format("[{}] {}\n", CharLevel, Message);
  write(OutputFD, Output.c_str(), Output.size());
  fsync(OutputFD);
}

void AssertHandler(char const *Message) {
  const auto Output = fmt::format("[ASSERT] {}\n", Message);
  write(OutputFD, Output.c_str(), Output.size());
  fsync(OutputFD);
}

} // Anonymous namespace

namespace Msg {
template <typename... Args>
static inline void DFmt(const char* fmt, const Args&... args) {
  if (LogMan::MSG_LEVEL < LogMan::DEBUG) {
    return;
  }
  FCL::LogMan::Msg::MFmtImpl(LogMan::DEBUG, fmt, fmt::make_format_args(args...));
}


}

namespace LogMan::Msg {
FEX_DEFAULT_VISIBILITY void MFmtImpl(::LogMan::DebugLevels level, const char* fmt, const fmt::format_args& args) {
}
}

void *LoadSymbolFEXCore(void *UserPtr, const char* Name) {
  printf("Trying to load: %s\n", Name);
  void *Result = dlsym(UserPtr, Name);
  printf("\tResult: 0x%lx\n", (uint64_t)Result);
  return Result;
}

class DummySyscallHandler: public FEXCore::HLE::SyscallHandler, public FEXCore::Allocator::FEXAllocOperators {
  public:
  uint64_t HandleSyscall(FEXCore::Core::CpuStateFrame *Frame, FEXCore::HLE::SyscallArguments *Args) override {
    LOGMAN_MSG_A_FMT("Syscalls not implemented");
    return 0;
  }

  FEXCore::HLE::SyscallABI GetSyscallABI(uint64_t Syscall) override {
    LOGMAN_MSG_A_FMT("Syscalls not implemented");
    return {0, false, 0 };
  }

  // These are no-ops implementations of the SyscallHandler API
  std::shared_mutex StubMutex;
  FEXCore::HLE::AOTIRCacheEntryLookupResult LookupAOTIRCacheEntry(uint64_t GuestAddr) override {
    return {0, 0, FHU::ScopedSignalMaskWithSharedLock {StubMutex}};
  }
};

class DummySignalDelegator;
namespace GlobalSignal {
  static DummySignalDelegator *SignalDelegator;
}

class DummySignalDelegator final : public FEXCore::SignalDelegator, public FEXCore::Allocator::FEXAllocOperators {
public:
  // Returns true if the host handled the signal
  // Arguments are the same as sigaction handler
  DummySignalDelegator() {
    Handlers.resize(FEXCore::SignalDelegator::MAX_SIGNALS);
  }
  ~DummySignalDelegator() override {
  }
  /**
   * @brief Registers a signal handler for the host to handle a signal specifically for guest handling
   *
   * It's a process level signal handler so one must be careful
   */
  void RegisterHostSignalHandlerForGuest(int Signal, FEXCore::HostSignalDelegatorFunctionForGuest Func) override {
  }

  void CheckXIDHandler() override {}

  static void ExecuteHostSignalDelegator(int Signal, siginfo_t *info, void *context) {
    GlobalSignal::SignalDelegator->Handlers[Signal](nullptr, Signal, info, context);
  }

protected:
  // Called from the thunk handler to handle the signal
  void HandleGuestSignal(FEXCore::Core::InternalThreadState *Thread, int Signal, void *Info, void *UContext) override {}

  void RegisterFrontendTLSState(FEXCore::Core::InternalThreadState *Thread) override {}
  void UninstallFrontendTLSState(FEXCore::Core::InternalThreadState *Thread) override {}

  /**
   * @brief Registers a signal handler for the host to handle a signal
   *
   * It's a process level signal handler so one must be careful
   */
  void FrontendRegisterHostSignalHandler(int Signal, FEXCore::HostSignalDelegatorFunction Func, bool Required) override {}
  void FrontendRegisterFrontendHostSignalHandler(int Signal, FEXCore::HostSignalDelegatorFunction Func, bool Required) override {
    struct sigaction HostAction{};
    HostAction.sa_sigaction = DummySignalDelegator::ExecuteHostSignalDelegator;
    HostAction.sa_flags = SA_SIGINFO;

    const int Result = ::sigaction(Signal, &HostAction, nullptr);
    if (Result != 0) {
      fprintf(stderr, "Couldn't install signal handler");
    }
    Handlers[Signal] = Func;
  }

private:
  std::vector<FEXCore::HostSignalDelegatorFunction> Handlers;
};

class TestCodeLoader final : public FEXCore::CodeLoader {
  void *StackPointer{};
  const uint8_t Code[11] = {
    0x48, 0xb8, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, // mov rax, 0x4142434445464748
    0xf4, // hlt
  };
public:
    TestCodeLoader() {
      StackPointer = malloc(StackSize());
    }

    ~TestCodeLoader() {
      free(StackPointer);
    }

    uint64_t StackSize() const override {
      return 8 * 1024 * 1024;
    }

    uint64_t GetStackPointer() override {
      return reinterpret_cast<uint64_t>(StackPointer) + StackSize();
    }

    uint64_t DefaultRIP() const override {
      return reinterpret_cast<uint64_t>(Code);
    }
};

int main(int argc, char **argv) {
  void *LoaderHandle = dlopen("libFEXCore.so", RTLD_LAZY);
  printf("Handle: %p\n", LoaderHandle);

  if (LoaderHandle == 0) {
    return -1;
  }

  if (!FCL::LoadFEXCoreSymbols(LoadSymbolFEXCore, LoaderHandle)) {
    printf("Couldn't get a symbol\n");
    return -1;
  }
  auto SBRKPointer = FCL::FEXCore::Allocator::DisableSBRKAllocations();

  FCL::LogMan::Throw::InstallHandler(AssertHandler);
  FCL::LogMan::Msg::InstallHandler(MsgHandler);

  FCL::FEXCore::Config::Initialize();
  FCL::FEXCore::Config::Load();
  FCL::FEXCore::Config::ReloadMetaLayer();
  FCL::FEXCore::Config::Set(FEXCore::Config::CONFIG_IS64BIT_MODE, "1");

  FCL::FEXCore::Telemetry::Initialize();

  // System allocator is now system allocator or FEX
  FCL::FEXCore::Context::InitializeStaticTables(FEXCore::Context::MODE_64BIT);

  // Create a context.
  auto CTX = FCL::FEXCore::Context::CreateNewContext();
  CTX->InitializeContext();

  jmp_buf LongJump{};
  int LongJumpVal{};

  auto SignalDelegation = fextl::make_unique<DummySignalDelegator>();
  GlobalSignal::SignalDelegator = SignalDelegation.get();

  SignalDelegation->RegisterFrontendHostSignalHandler(SIGSEGV, [&LongJump](FEXCore::Core::InternalThreadState *Thread, int Signal, void *info, void *ucontext) {
    longjmp(LongJump, 1);

    constexpr uint8_t HLT = 0xF4;
    if (reinterpret_cast<uint8_t*>(Thread->CurrentFrame->State.rip)[0] != HLT) {
      return false;
    }

    return false;
  }, true);


  CTX->SetSignalDelegator(SignalDelegation.get());
  CTX->SetSyscallHandler(new DummySyscallHandler());

  TestCodeLoader Loader{};

  bool Loaded = CTX->InitCore(Loader.DefaultRIP(), Loader.GetStackPointer());
  if (!Loaded) {
    fprintf(stderr, "Couldn't initialize CPU core\n");
    return -1;
  }

  LongJumpVal = setjmp(LongJump);
  if (!LongJumpVal) {
    CTX->RunUntilExit();
  }

  FEXCore::Core::CPUState State;
  CTX->GetCPUState(&State);

  int Result = 0;
  if (State.gregs[FEXCore::X86State::REG_RAX] != 0x4142434445464748ULL) {
    Msg::DFmt("Didn't get correct result");
    Result = -1;
  }
  else {
    Msg::DFmt("RAX was correctly: 0x{:x} at 0x{:x}", State.gregs[FEXCore::X86State::REG_RAX], State.rip);
  }

  FCL::FEXCore::Allocator::ReenableSBRKAllocations(SBRKPointer);
  SignalDelegation.reset();
  CTX.reset();

  dlclose(LoaderHandle);

  return Result;
}
