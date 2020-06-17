#pragma once
#include <cstdint>
#include <cstdio>
#include <string>

namespace ELFLoader {
struct ELFSymbol;
}

namespace FEXCore {
class JITSymbols final {
public:
  JITSymbols();
  ~JITSymbols();
  void Register(void *HostAddr, uint64_t GuestAddr, uint32_t CodeSize, ::ELFLoader::ELFSymbol const *Symbol, std::string const *File);
private:
  FILE* fp{};
};
}
