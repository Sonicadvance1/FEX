#pragma once
#include <stdint.h>

#include <FEXCore/Config/Config.h>

namespace FEXCore {
class X86GeneratedCode final {
public:
  X86GeneratedCode();
  ~X86GeneratedCode();

  uint64_t SignalReturn{};
  uint64_t CallbackReturn{};

  FEXCore::Config::Value<bool> Is64BitMode{FEXCore::Config::CONFIG_IS64BIT_MODE, 0};
private:
  void *CodePtr{};
};
}
