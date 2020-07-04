#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <signal.h>

namespace FEXCore {
namespace Context {
  struct Context;
}
namespace Core {
  struct InternalThreadState;
}

  class SignalDelegator {
  public:
    // Returns true if the host handled the signal
    // Arguments are the same as sigaction handler
    using HostSignalDelegatorFunction = std::function<bool(FEXCore::Core::InternalThreadState *Thread, int Signal, void *info, void *ucontext)>;
    SignalDelegator(FEXCore::Context::Context *ctx);

    void RegisterTLSState(FEXCore::Core::InternalThreadState *Thread);
    void MaskThreadSignals();
    void ResetThreadSignalMask();

    void RegisterHostSignalHandler(int Signal, HostSignalDelegatorFunction Func);
    uint64_t RegisterGuestSignalHandler(int Signal, const struct sigaction *Action, struct sigaction *OldAction);

    // Called from the thunk handler to handle the signal
    void HandleSignal(int Signal, void *Info, void *UContext);

  private:
    FEXCore::Context::Context *CTX;

    constexpr static size_t MAX_SIGNALS {64};
    HostSignalDelegatorFunction HostHandlers[MAX_SIGNALS]{};
    struct sigaction GuestHandlers[MAX_SIGNALS]{};
    std::atomic<bool> ThunkHandlerInstalled[MAX_SIGNALS]{};
    void InstallHostThunk(int Signal);

    std::mutex HostDelegatorMutex;
    std::mutex GuestDelegatorMutex;
  };
}
