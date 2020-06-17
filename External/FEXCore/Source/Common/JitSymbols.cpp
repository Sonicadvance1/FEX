#include "Common/JitSymbols.h"
#include "ELFLoader.h"

#include <mutex>
#include <string>
#include <sstream>
#include <unistd.h>

namespace FEXCore {
  JITSymbols::JITSymbols() {
    std::stringstream PerfMap;
    PerfMap << "/tmp/perf-" << getpid() << ".map";

    fp = fopen(PerfMap.str().c_str(), "wb");
    if (fp) {
      // Disable buffering on this file
      setvbuf(fp, nullptr, _IONBF, 0);
    }
  }

  JITSymbols::~JITSymbols() {
    if (fp) {
      fclose(fp);
    }
  }

  void JITSymbols::Register(void *HostAddr, uint64_t GuestAddr, uint32_t CodeSize, ::ELFLoader::ELFSymbol const *Symbol, std::string const *File) {
    if (!fp) return;

    static std::mutex RegisterMutex{};
    std::lock_guard<std::mutex> lk(RegisterMutex);

    // Linux perf format is very straightforward
    // `<HostPtr> <Size> <Name>\n`
    std::stringstream String;
    if (Symbol) {
      String << std::hex << HostAddr << " " << CodeSize << " " << "JIT_" << Symbol->Name << "_" << *File << std::endl;
    }
    else if (File) {
      String << std::hex << HostAddr << " " << CodeSize << " " << "JIT_" << *File << std::endl;
    }
    else {
      String << std::hex << HostAddr << " " << CodeSize << " " << "JIT_0x" << GuestAddr << std::endl;
    }
    fwrite(String.str().c_str(), 1, String.str().size(), fp);
  }
}

