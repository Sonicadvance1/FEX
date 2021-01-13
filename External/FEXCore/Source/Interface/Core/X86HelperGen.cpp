#include "Interface/Core/X86HelperGen.h"

#include <cstring>
#include <stdlib.h>
#include <vector>
#include <sys/mman.h>

#include <FEXCore/Utils/LogManager.h>

namespace FEXCore {
X86GeneratedCode::X86GeneratedCode() {
  // Allocate a page for our emulated guest

  uint32_t Flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifndef MAP_32BIT
#define MAP_32BIT 0
#endif
//  if (!Is64BitMode()) {
//    Flags |= MAP_32BIT;
//  }
  CodePtr = mmap((void*)0xFFFE0000, 0x1000, PROT_READ | PROT_WRITE, Flags, -1, 0); //malloc(0x1000);
  LogMan::Msg::D("Sigret handler: %p", CodePtr);

  SignalReturn = reinterpret_cast<uint64_t>(CodePtr);
  CallbackReturn = reinterpret_cast<uint64_t>(CodePtr) + 2;

  if (!Is64BitMode() &&
      SignalReturn >= 0x1'0000'0000ULL) {
    LogMan::Msg::A("Sig ret handle needs to be below 4GB");
  }

  const std::vector<uint8_t> SignalReturnCode = {
    0x0F, 0x36, // SIGRET FEX instruction
    0x0F, 0x37, // CALLBACKRET FEX Instruction
  };

  memcpy(CodePtr, &SignalReturnCode.at(0), SignalReturnCode.size());
}

X86GeneratedCode::~X86GeneratedCode() {
  free(CodePtr);
}

}

