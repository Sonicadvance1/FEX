#include <FEXCore/Utils/LogManager.h>
#include <sstream>
#include <vector>

#include <signal.h>

namespace LogMan {

namespace Throw {
std::vector<ThrowHandler> Handlers;
void InstallHandler(ThrowHandler Handler) { Handlers.emplace_back(Handler); }

[[noreturn]] void M(const char *fmt, va_list args) {
  size_t MsgSize = 1024;
  char *Buffer = reinterpret_cast<char*>(alloca(MsgSize));
  size_t Return = vsnprintf(Buffer, MsgSize, fmt, args);
  if (Return >= MsgSize) {
    // Allocate a bigger size on failure
    MsgSize = Return;
    Buffer = reinterpret_cast<char*>(alloca(MsgSize));
    vsnprintf(Buffer, MsgSize, fmt, args);
  }

  for (auto &Handler : Handlers) {
    Handler(Buffer);
  }

  kill(0, SIGILL);
  __builtin_trap();
}
} // namespace Throw

namespace Msg {
std::vector<MsgHandler> Handlers;
void InstallHandler(MsgHandler Handler) { Handlers.emplace_back(Handler); }

void M(DebugLevels Level, const char *fmt, va_list args) {
  size_t MsgSize = 1024;
  char *Buffer = reinterpret_cast<char*>(alloca(MsgSize));
  size_t Return = vsnprintf(Buffer, MsgSize, fmt, args);
  if (Return >= MsgSize) {
    // Allocate a bigger size on failure
    MsgSize = Return;
    Buffer = reinterpret_cast<char*>(alloca(MsgSize));
    vsnprintf(Buffer, MsgSize, fmt, args);
  }
  for (auto &Handler : Handlers) {
    Handler(Level, Buffer);
  }
}

} // namespace Msg
} // namespace LogMan
