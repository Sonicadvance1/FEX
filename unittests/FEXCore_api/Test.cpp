#include <functional>
#include <stdio.h>

namespace FEXCore {
  namespace Core {
    struct InternalThreadState;
  }

  using HostSignalDelegatorFunction = std::function<bool(FEXCore::Core::InternalThreadState *Thread, int Signal, void *info, void *ucontext)>;
}

namespace FCL::FEXCore {
  using RegisterFrontendHostSignalHandler_Type = void (*)(void*, int, ::FEXCore::HostSignalDelegatorFunction, bool);
  extern RegisterFrontendHostSignalHandler_Type RegisterFrontendHostSignalHandler;
}

namespace FEXCore {
  struct SignalDelegator {
    void RegisterFrontendHostSignalHandler(int Signal, HostSignalDelegatorFunction Func, bool Required);
  };

  void SignalDelegator::RegisterFrontendHostSignalHandler(int Signal, HostSignalDelegatorFunction Func, bool Required) {
    FCL::FEXCore::RegisterFrontendHostSignalHandler(this, Signal, Func, Required);
  }
}

namespace FEXCore::Assert {
  void ForcedAssert() {
  }
}
