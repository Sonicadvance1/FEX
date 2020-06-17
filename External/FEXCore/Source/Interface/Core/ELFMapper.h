#pragma once
#include <cstddef>
#include <stdint.h>
#include <string>

namespace ELFLoader {
struct ELFSymbol;
}

namespace FEXCore::ELFMapper {
  void Init();
  void Shutdown();
  void AddELFToList(std::string *Filename, uint64_t Addr, size_t Size);
  std::pair<::ELFLoader::ELFSymbol const *, std::string const*> FindELFSymbol(uint64_t Addr);
}
