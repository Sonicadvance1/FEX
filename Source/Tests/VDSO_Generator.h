#pragma once
#include <cstdint>
#include <vector>

namespace FEX {
class VDSOGenerator {
public:
  void Initialize(bool Is64Bit);

  uint64_t GetSize() const { return Size; }
  char *Data() { return &FileData.at(0); }

  void SetBase(uint64_t Base) { VDSOBase = Base; }
  uint64_t GetBase() const { return VDSOBase; }

private:
  uint64_t Size{};
  uint64_t VDSOBase{};
  std::vector<char> FileData{};
};
}
