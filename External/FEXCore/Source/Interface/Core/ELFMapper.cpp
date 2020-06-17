#include "ELFLoader.h"
#include "ELFSymbolDatabase.h"
#include "LogManager.h"
#include <memory>

namespace {
struct ELFPair {
  std::shared_ptr<::ELFLoader::ELFContainer> File;
  std::shared_ptr<::ELFLoader::ELFSymbolDatabase> DB;
  size_t Addr;
  size_t Size;
};

class ELFLoaderMap {
public:
  ELFLoaderMap() {
  }

  void AddELFToList(std::string *Filename, uint64_t Addr, size_t Size) {
    ELFPair ELF{};
    ELF.File = std::make_unique<::ELFLoader::ELFContainer>(*Filename, "", true);
    if (!ELF.File->Loaded()) {
      return;
    }
    LogMan::Msg::D("We were able to load the elf");
    ELF.DB = std::make_unique<::ELFLoader::ELFSymbolDatabase>(ELF.File.get());
    ELF.Addr = Addr;
    ELF.Size = Size;
    ELFMap[Addr] = ELF;
  }

  ELFPair *FindELF(uint64_t Addr) {
    auto iter = ELFMap.upper_bound(Addr);
    if (iter != ELFMap.begin()) {
      --iter;
    }

    if (iter == ELFMap.end()) {
      return nullptr;
    }

    if (iter != ELFMap.end() &&
        iter->first <= Addr &&
        iter->first + iter->second.Size > Addr) {
      return &iter->second;
    }

    return nullptr;
  }

private:
  std::map<uint64_t, ELFPair> ELFMap;
};

static ELFLoaderMap Map;
}

namespace FEXCore::ELFMapper {
  void Init() {
  }

  void Shutdown() {
  }

  void AddELFToList(std::string *Filename, uint64_t Addr, size_t Size) {
    Map.AddELFToList(Filename, Addr, Size);
  }

  std::pair<::ELFLoader::ELFSymbol const *, std::string const*> FindELFSymbol(uint64_t Addr) {
    auto ELF = Map.FindELF(Addr);
    if (!ELF) {
      return {};
    }

    uint64_t AdjustedAddress = Addr;
    if (ELF->File->WasDynamic()) {
      AdjustedAddress -= ELF->Addr;
    }

    auto Symbol = ELF->File->GetSymbolInRange(::ELFLoader::ELFContainer::RangeType{AdjustedAddress, 1});
//    if (Symbol) {
//      LogMan::Msg::D("Symbol is in file at base Addr: %p. '%s'", ELF->Addr, Symbol ? Symbol->Name : "<Unknown>");
//    }
    return {Symbol, &ELF->File->GetFilename()};
  }

}

