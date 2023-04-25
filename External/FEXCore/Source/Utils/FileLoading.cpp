#include <FEXCore/fextl/string.h>
#include <FEXCore/fextl/vector.h>
#include <FEXCore/Utils/FileLoading.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <span>
#include <unistd.h>

namespace FEXCore::FileLoading {

template<typename T>
static bool LoadFileImpl(T &Data, const fextl::string &Filepath, size_t FixedSize) {
  int FD = open(Filepath.c_str(), O_RDONLY);

  if (FD == -1) {
    return false;
  }

  size_t FileSize{};
  if (FixedSize == 0) {
    struct stat buf;
    if (fstat(FD, &buf) != 0) {
      close(FD);
      return false;
    }

    FileSize = buf.st_size;
  }
  else {
    FileSize = FixedSize;
  }

  ssize_t Read = -1;
  if (FileSize > 0) {
    Data.resize(FileSize);
    Read = pread(FD, &Data.at(0), FileSize, 0);
  }
  close(FD);
  return Read == FileSize;
}

bool LoadFile(fextl::vector<char> &Data, const fextl::string &Filepath, size_t FixedSize) {
  return LoadFileImpl(Data, Filepath, FixedSize);
}

bool LoadFile(fextl::string &Data, const fextl::string &Filepath, size_t FixedSize) {
  return LoadFileImpl(Data, Filepath, FixedSize);
}

ssize_t LoadFileToBuffer(const fextl::string &Filepath, std::span<char> Buffer) {
  int FD = open(Filepath.c_str(), O_RDONLY);

  if (FD == -1) {
    return -1;
  }

  ssize_t Read = pread(FD, Buffer.data(), Buffer.size(), 0);
  close(FD);
  return Read;
}


}
