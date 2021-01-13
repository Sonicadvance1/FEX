#include "Tests/VDSO_Generator.h"
#include "FEXCore/Utils/LogManager.h"

#include <fstream>
#include <string>
#include <vector>

namespace FEX {
static bool LoadFile(std::vector<char> &Data, std::string Filename) {
  std::fstream File;
  File.open(Filename, std::ios::in);

  if (!File.is_open()) {
    return false;
  }

  if (!File.seekg(0, std::fstream::end)) {
    LogMan::Msg::D("Couldn't load configuration file: Seek end");
    return false;
  }

  auto FileSize = File.tellg();
  if (File.fail()) {
    LogMan::Msg::D("Couldn't load configuration file: tellg");
    return false;
  }

  if (!File.seekg(0, std::fstream::beg)) {
    LogMan::Msg::D("Couldn't load configuration file: Seek beginning");
    return false;
  }

  if (FileSize > 0) {
    Data.resize(FileSize);
    if (!File.read(&Data.at(0), FileSize)) {
      // Probably means permissions aren't set. Just early exit
      return false;
    }
    File.close();
  }
  else {
    return false;
  }

  return true;
}

void VDSOGenerator::Initialize(bool Is64Bit) {
  if (Is64Bit) {
    LoadFile(FileData, "64bit_vdso.elf");
  }
  else {
    LoadFile(FileData, "/mnt/Work/Work/work/FEXNew/Build/32bit_vdso.elf");
  }
  Size = FileData.size();
}
}
