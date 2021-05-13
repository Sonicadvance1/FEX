/*
$info$
tags: Bin|FEXLoader
desc: Glues the ELF loader, FEXCore and LinuxSyscalls to launch an elf under fex
$end_info$
*/

#include "Common/ArgumentLoader.h"
#include "Common/EnvironmentLoader.h"
#include "Common/Config.h"
#include "ELFCodeLoader.h"
#include "ELFCodeLoader2.h"
#include "Tests/LinuxSyscalls/x32/Syscalls.h"
#include "Tests/LinuxSyscalls/x64/Syscalls.h"
#include "Tests/LinuxSyscalls/SignalDelegator.h"

#include <FEXCore/Config/Config.h>
#include <FEXCore/Core/CodeLoader.h>
#include <FEXCore/Core/Context.h>
#include <FEXCore/Utils/Allocator.h>
#include <FEXCore/Utils/ELFContainer.h>
#include <FEXCore/Utils/LogManager.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <set>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <signal.h>

namespace {
static bool SilentLog;
static FILE *OutputFD {stderr};

void MsgHandler(LogMan::DebugLevels Level, char const *Message) {
  const char *CharLevel{nullptr};

  switch (Level) {
  case LogMan::NONE:
    CharLevel = "NONE";
    break;
  case LogMan::ASSERT:
    CharLevel = "ASSERT";
    break;
  case LogMan::ERROR:
    CharLevel = "ERROR";
    break;
  case LogMan::DEBUG:
    CharLevel = "DEBUG";
    break;
  case LogMan::INFO:
    CharLevel = "Info";
    break;
  case LogMan::STDOUT:
    CharLevel = "STDOUT";
    break;
  case LogMan::STDERR:
    CharLevel = "STDERR";
    break;
  default:
    CharLevel = "???";
    break;
  }

  if (!SilentLog) {
    fprintf(OutputFD, "[%s] %s\n", CharLevel, Message);
    fflush(OutputFD);
  }
}

void AssertHandler(char const *Message) {
  if (!SilentLog) {
    fprintf(OutputFD, "[ASSERT] %s\n", Message);
    fflush(OutputFD);
  }
}

bool CheckMemMapping() {
  std::fstream fs;
  fs.open("/proc/self/maps", std::fstream::in | std::fstream::binary);
  std::string Line;
  while (std::getline(fs, Line)) {
    if (fs.eof()) break;
    uint64_t Begin, End;
    if (sscanf(Line.c_str(), "%lx-%lx", &Begin, &End) == 2) {
      // If a memory range is living inside the 32bit memory space then we have a problem
      if (Begin < 0x1'0000'0000) {
        return false;
      }
    }
  }

  fs.close();
  return true;
}
}

void InterpreterHandler(std::string *Filename, std::string const &RootFS, std::vector<std::string> *args) {
  // Open the file pointer to the filename and see if we need to find an interpreter
  std::fstream File;
  size_t FileSize{0};
  File.open(*Filename, std::fstream::in | std::fstream::binary);

  if (!File.is_open())
    return;

  File.seekg(0, File.end);
  FileSize = File.tellg();
  File.seekg(0, File.beg);

  // Is the file large enough for shebang
  if (FileSize <= 2)
    return;

  // Handle shebang files
  if (File.get() == '#' &&
      File.get() == '!') {
    std::string InterpreterLine;
    std::getline(File, InterpreterLine);
    std::vector<std::string> ShebangArguments{};

    // Shebang line can have a single argument
    std::istringstream InterpreterSS(InterpreterLine);
    std::string Argument;
    while (std::getline(InterpreterSS, Argument, ' ')) {
      if (Argument.empty()) {
        continue;
      }
      ShebangArguments.emplace_back(Argument);
    }

    // Executable argument
    std::string &ShebangProgram = ShebangArguments[0];

    // If the filename is absolute then prepend the rootfs
    // If it is relative then don't append the rootfs
    if (ShebangProgram[0] == '/') {
      ShebangProgram = RootFS + ShebangProgram;
    }
    *Filename = ShebangProgram;

    // Insert all the arguments at the start
    args->insert(args->begin(), ShebangArguments.begin(), ShebangArguments.end());

    // Done here
    return;
  }
}

bool RanAsInterpreter(char *Program) {
  return strstr(Program, "FEXInterpreter") != nullptr;
}

bool IsInterpreterInstalled() {
  // The interpreter is installed if both the binfmt_misc handlers are available
  return std::filesystem::exists("/proc/sys/fs/binfmt_misc/FEX-x86") &&
         std::filesystem::exists("/proc/sys/fs/binfmt_misc/FEX-x86_64");
}

bool HandleSIGSYS(int Signal, void *info, void *ucontext) {
  ucontext_t* _context = (ucontext_t*)ucontext;
  siginfo_t *HostSigInfo = reinterpret_cast<siginfo_t*>(info);
  uint32_t Syscall = _context->uc_mcontext.gregs[REG_RAX];
  // This allows us to trap syscalls and put them down either a 32-bit path or a 64-bit path
  // Depending on where the syscall comes from we need to send it down different paths
  // 1) For syscalls outside of FEX we want to send mmap and munmap through our 64-bit allocator
  //  a) Allows dlopen to load elf libraries in to 64-bit space
  // 2) For syscalls inside of FEX we want them going down either a 32-bit path or 64-bit path
  //  a) Need to think about this more
  // 3) Guest space syscalls will get pushed down the correct 32-bit path translated to 64-bit syscalls
  //  a) Or if the host supports 32-bit syscalls then we use those. Doesn't affect behaviour
  if (Syscall & 0x80000000U) {
    // 32-bit support
    switch (Syscall & ~0x80000000U) {
      default: {
        register uint64_t rax __asm__ ("rax") = Syscall & ~0x80000000U;
        register uint64_t rdi __asm__ ("rdi") = _context->uc_mcontext.gregs[REG_RDI];
        register uint64_t rsi __asm__ ("rsi") = _context->uc_mcontext.gregs[REG_RSI];
        register uint64_t rdx __asm__ ("rdx") = _context->uc_mcontext.gregs[REG_RDX];
        register uint64_t r10 __asm__ ("r10") = _context->uc_mcontext.gregs[REG_R10];
        register uint64_t r8  __asm__ ("r8")  = _context->uc_mcontext.gregs[REG_R8];
        register uint64_t r9  __asm__ ("r9")  = _context->uc_mcontext.gregs[REG_R9];
        __asm volatile("syscall;"
          : "+r" (rax)
          : "r" (rdi)
          , "r" (rsi)
          , "r" (rdx)
          , "r" (r10)
          , "r" (r8)
          , "r" (r9)
          : "memory");
        _context->uc_mcontext.gregs[REG_RAX] = rax;
        return true;
        break;
      }
    }
  }
  switch (Syscall) {
    case SYS_mmap: {
      _context->uc_mcontext.gregs[REG_RAX] = reinterpret_cast<uint64_t>(FEXCore::Allocator::mmap(
        (void*)_context->uc_mcontext.gregs[REG_RDI],
        (size_t)_context->uc_mcontext.gregs[REG_RSI],
        (int)_context->uc_mcontext.gregs[REG_RDX],
        (int)_context->uc_mcontext.gregs[REG_R10],
        (int)_context->uc_mcontext.gregs[REG_R8],
        (off_t)_context->uc_mcontext.gregs[REG_R9]));
      if (_context->uc_mcontext.gregs[REG_RAX] == -1) {
        _context->uc_mcontext.gregs[REG_RAX] = -errno;
      }
      return true;
      break;
    }
    case SYS_munmap: {
      _context->uc_mcontext.gregs[REG_RAX] = FEXCore::Allocator::munmap(
        (void*)_context->uc_mcontext.gregs[REG_RDI],
        (size_t)_context->uc_mcontext.gregs[REG_RSI]);
      if (_context->uc_mcontext.gregs[REG_RAX] == -1) {
        _context->uc_mcontext.gregs[REG_RAX] = -errno;
      }
      return true;
      break;
    }
    default: {
      register uint64_t rax __asm__ ("rax") = Syscall;
      register uint64_t rdi __asm__ ("rdi") = _context->uc_mcontext.gregs[REG_RDI];
      register uint64_t rsi __asm__ ("rsi") = _context->uc_mcontext.gregs[REG_RSI];
      register uint64_t rdx __asm__ ("rdx") = _context->uc_mcontext.gregs[REG_RDX];
      register uint64_t r10 __asm__ ("r10") = _context->uc_mcontext.gregs[REG_R10];
      register uint64_t r8  __asm__ ("r8")  = _context->uc_mcontext.gregs[REG_R8];
      register uint64_t r9  __asm__ ("r9")  = _context->uc_mcontext.gregs[REG_R9];
      __asm volatile("syscall;"
        : "+r" (rax)
        : "r" (rdi)
        , "r" (rsi)
        , "r" (rdx)
        , "r" (r10)
        , "r" (r8)
        , "r" (r9)
        : "memory");
      _context->uc_mcontext.gregs[REG_RAX] = rax;
      return true;
      break;
    }
  }
  return false;
}

void SetupUserDispatch(FEX::HLE::SignalDelegator *SignalDelegation) {
  // Scan and find our program
  std::fstream fs;
  fs.open("/proc/self/maps", std::fstream::in | std::fstream::binary);
  std::string Line;
  uint64_t Low{~0ULL};
  uint64_t High{};
  while (std::getline(fs, Line)) {
    if (fs.eof()) break;

    if (Line.find("FEX") == std::string::npos) continue;

    uint64_t Begin, End;
    if (sscanf(Line.c_str(), "%lx-%lx", &Begin, &End) == 2) {
      Low = std::min(Low, Begin);
      High = std::max(High, End);
    }
  }

  fs.close();

  LogMan::Msg::D("Allowing through [0x%lx, 0x%lx)", Low, High);

  // Setup our SIGSYS handler first
  SignalDelegation->RegisterFrontendHostSignalHandler(SIGSYS, [](FEXCore::Core::InternalThreadState *Thread, int Signal, void *info, void *ucontext) -> bool {
    return HandleSIGSYS(Signal, info, ucontext);
  });

  // We now have a min/max to search for
  prctl(PR_SET_SYSCALL_USER_DISPATCH, PR_SYS_DISPATCH_ON, Low, High - Low);
}

void DisableUserDispatch() {
  prctl(PR_SET_SYSCALL_USER_DISPATCH, PR_SYS_DISPATCH_OFF, 0, 0);
}

int main(int argc, char **argv, char **const envp) {
  bool IsInterpreter = RanAsInterpreter(argv[0]);
  LogMan::Throw::InstallHandler(AssertHandler);
  LogMan::Msg::InstallHandler(MsgHandler);

#if !(defined(ENABLE_ASAN) && ENABLE_ASAN)
  // LLVM ASAN maps things to the lower 32bits
  if (!CheckMemMapping()) {
    LogMan::Msg::E("[Unsupported] FEX mapped to lower 32bits! Exiting!");
    return -1;
  }
#endif

  FEXCore::Config::Initialize();
  FEXCore::Config::AddLayer(std::make_unique<FEX::Config::MainLoader>());

  if (IsInterpreter) {
    FEX::ArgLoader::LoadWithoutArguments(argc, argv);
  }
  else {
    FEXCore::Config::AddLayer(std::make_unique<FEX::ArgLoader::ArgLoader>(argc, argv));
  }

  FEXCore::Config::AddLayer(std::make_unique<FEX::Config::EnvLoader>(envp));
  FEXCore::Config::Load();

  auto Args = FEX::ArgLoader::Get();
  auto ParsedArgs = FEX::ArgLoader::GetParsedArgs();

  if (Args.empty()) {
    // Early exit if we weren't passed an argument
    return 0;
  }

  std::string Program = Args[0];

  // These layers load on initialization
  FEXCore::Config::AddLayer(std::make_unique<FEX::Config::AppLoader>(std::filesystem::path(Program).filename(), true));
  FEXCore::Config::AddLayer(std::make_unique<FEX::Config::AppLoader>(std::filesystem::path(Program).filename(), false));

  // Reload the meta layer
  FEXCore::Config::ReloadMetaLayer();
  FEXCore::Config::Set(FEXCore::Config::CONFIG_IS_INTERPRETER, IsInterpreter ? "1" : "0");
  FEXCore::Config::Set(FEXCore::Config::CONFIG_INTERPRETER_INSTALLED, IsInterpreterInstalled() ? "1" : "0");

  FEX_CONFIG_OPT(SilentLog, SILENTLOG);
  FEX_CONFIG_OPT(AOTIRCapture, AOTIRCAPTURE);
  FEX_CONFIG_OPT(AOTIRGenerate, AOTIRGENERATE);
  FEX_CONFIG_OPT(AOTIRLoad, AOTIRLOAD);
  FEX_CONFIG_OPT(OutputLog, OUTPUTLOG);
  FEX_CONFIG_OPT(LDPath, ROOTFS);
  FEX_CONFIG_OPT(Environment, ENV);
  ::SilentLog = SilentLog();

  if (!::SilentLog) {
    auto LogFile = OutputLog();
    if (LogFile == "stderr") {
      OutputFD = stderr;
    }
    else if (LogFile == "stdout") {
      OutputFD = stdout;
    }
    else if (!LogFile.empty()) {
      OutputFD = fopen(LogFile.c_str(), "wb");
    }
  }

  InterpreterHandler(&Program, LDPath(), &Args);

  if (!std::filesystem::exists(Program)) {
    // Early exit if the program passed in doesn't exist
    // Will prevent a crash later
    LogMan::Msg::E("%s: command not found", Program.c_str());
    return -ENOEXEC;
  }


  uint32_t KernelVersion = FEX::HLE::SyscallHandler::CalculateHostKernelVersion();
  if (KernelVersion < FEX::HLE::SyscallHandler::KernelVersion(4, 17)) {
    // We require 4.17 minimum for MAP_FIXED_NOREPLACE
    LogMan::Msg::E("FEXLoader requires kernel 4.17 minimum. Expect problems.");
  }

  ELFCodeLoader2 Loader{Program, LDPath(), Args, ParsedArgs, envp, &Environment};
  //FEX::HarnessHelper::ELFCodeLoader Loader{Program, LDPath(), Args, ParsedArgs, envp, &Environment};

  if (!Loader.ELFWasLoaded()) {
    // Loader couldn't load this program for some reason
    LogMan::Msg::E("Invalid or Unsupported elf file.");
    return -ENOEXEC;
  }

  FEXCore::Config::Set(FEXCore::Config::CONFIG_APP_FILENAME, std::filesystem::canonical(Program));
  FEXCore::Config::Set(FEXCore::Config::CONFIG_IS64BIT_MODE, Loader.Is64BitMode() ? "1" : "0");

  FEX::HLE::x32::MemAllocator *Allocator = nullptr;

  std::unique_ptr<FEX::HLE::SignalDelegator> SignalDelegation = std::make_unique<FEX::HLE::SignalDelegator>();

  SetupUserDispatch(SignalDelegation.get());

  if (Loader.Is64BitMode()) {
    if (!Loader.MapMemory([](void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
      return FEXCore::Allocator::mmap(addr, length, prot, flags, fd, offset);
    }, [](void *addr, size_t length) {
      return FEXCore::Allocator::munmap(addr, length);
    })) {
      // failed to map
      LogMan::Msg::E("Failed to map 64-bit elf file.");
      return -ENOEXEC;
    }
  } else {
    FEX_CONFIG_OPT(Use32BitAllocator, FORCE32BITALLOCATOR);
    if (KernelVersion < FEX::HLE::SyscallHandler::KernelVersion(4, 17)) {
      Use32BitAllocator = true;
    }

    // Setup our userspace allocator
    if (!Use32BitAllocator &&
        KernelVersion >= FEX::HLE::SyscallHandler::KernelVersion(4, 17)) {
      FEXCore::Allocator::SetupHooks();
    }

    Allocator = FEX::HLE::x32::CreateAllocator(Use32BitAllocator);

    if (!Loader.MapMemory([Allocator](void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
      return Allocator->mmap(addr, length, prot, flags, fd, offset);
    }, [Allocator](void *addr, size_t length) {
      return Allocator->munmap(addr, length);
    })) {
      // failed to map
      LogMan::Msg::E("Failed to map 32-bit elf file.");
      return -ENOEXEC;
    }
  }

  FEXCore::Context::InitializeStaticTables(Loader.Is64BitMode() ? FEXCore::Context::MODE_64BIT : FEXCore::Context::MODE_32BIT);

  auto CTX = FEXCore::Context::CreateNewContext();
  FEXCore::Context::InitializeContext(CTX);

  std::unique_ptr<FEX::HLE::SyscallHandler> SyscallHandler{
    Loader.Is64BitMode() ?
      FEX::HLE::x64::CreateHandler(CTX, SignalDelegation.get()) :
      FEX::HLE::x32::CreateHandler(CTX, SignalDelegation.get(), Allocator)
  };

  SyscallHandler->SetCodeLoader(&Loader);

  auto BRKInfo = Loader.GetBRKInfo();

  SyscallHandler->DefaultProgramBreak(BRKInfo.Base, BRKInfo.Size);

  FEXCore::Context::SetSignalDelegator(CTX, SignalDelegation.get());
  FEXCore::Context::SetSyscallHandler(CTX, SyscallHandler.get());
  FEXCore::Context::InitCore(CTX, &Loader);

  // dlopen testing
  printf("Test\n");
  void *Ptr = dlopen("/mnt/Work/Work/Unicorn_Tests/ELFLoader/Tests/test_lib.so", RTLD_NOW);
  printf("dlopen result: %p\n", Ptr);

  FEXCore::Context::ExitReason ShutdownReason = FEXCore::Context::ExitReason::EXIT_SHUTDOWN;

  // There might already be an exit handler, leave it installed
  if(!FEXCore::Context::GetExitHandler(CTX)) {
    FEXCore::Context::SetExitHandler(CTX, [&](uint64_t thread, FEXCore::Context::ExitReason reason) {
      if (reason != FEXCore::Context::ExitReason::EXIT_DEBUG) {
        ShutdownReason = reason;
        FEXCore::Context::Stop(CTX);
      }
    });
  }

  if (AOTIRLoad() || AOTIRCapture() || AOTIRGenerate()) {
    LogMan::Msg::I("Warning: AOTIR is experimental, and might lead to crashes. Capture doesn't work with programs that fork.");
  }

  FEXCore::Context::SetAOTIRLoader(CTX, [](const std::string &fileid) -> int {
    auto filepath = std::filesystem::path(FEXCore::Config::GetDataDirectory()) / "aotir" / (fileid + ".aotir");

    return open(filepath.c_str(), O_RDONLY);
  });

  for(auto Section: Loader.Sections) {
    FEXCore::Context::AddNamedRegion(CTX, Section.Base, Section.Size, Section.Offs, Section.Filename);
  }

  if (AOTIRGenerate()) {
    for(auto Section: Loader.Sections) {
      if (Section.Executable && Section.Size > 16) {
        ELFLoader::ELFContainer container{Section.Filename, "", false};

        std::set<uintptr_t> BranchTargets;

        container.AddSymbols([&](ELFLoader::ELFSymbol* sym) {
          auto Destination = sym->Address + Section.ElfBase;

          if (! (Destination >= Section.Base && Destination <= (Section.Base + Section.Size)) ) {
            //printf("Sym : %lx %lx out of range\n", sym->Address, Destination);
            return; // outside of current section, unlikely to be real code
          }

          BranchTargets.insert(Destination);
        });

        LogMan::Msg::I("Symbol seed: %ld", BranchTargets.size());

        container.AddUnwindEntries([&](uintptr_t Entry) {
          auto Destination = Entry + Section.ElfBase;

          if (! (Destination >= Section.Base && Destination <= (Section.Base + Section.Size)) ) {
            //printf("Sym : %lx %lx out of range\n", sym->Address, Destination);
            return; // outside of current section, unlikely to be real code
          }

          BranchTargets.insert(Destination);
        });


        LogMan::Msg::I("Symbol + Unwind seed: %ld", BranchTargets.size());

        for (size_t Offset = 0; Offset < (Section.Size - 16); Offset++) {
          uint8_t *pCode = (uint8_t *)(Section.Base + Offset);

          if (*pCode == 0xE8) {
            uintptr_t Destination = (int)(pCode[1] | (pCode[2] << 8) | (pCode[3] << 16) | (pCode[4] << 24));
            Destination += (uintptr_t)pCode + 5;

            auto DestinationPtr = (uint8_t*)Destination;
            
            if (! (Destination >= Section.Base && Destination <= (Section.Base + Section.Size)) )
              continue; // outside of current section, unlikely to be real code

            if (DestinationPtr[0] == 0 && DestinationPtr[1] == 0)
              continue; // add al, [rax], unlikely to be real code
/*
            if (DestinationPtr[0] == 0x44 && DestinationPtr[1] == 0x0f && DestinationPtr[2] == 0x6f)
              continue; // REX.W + movq leads to frontend bugs
*/
            BranchTargets.insert(Destination);
          }

          if (pCode[0] == 0xf3 && pCode[1] == 0x0f && pCode[2] == 0x1e && pCode[3] == 0xfa) {
            BranchTargets.insert((uintptr_t)pCode);
          }
        }

        uint64_t SectionMaxAddress = Section.Base + Section.Size;
        std::set<uint64_t> ExternalBranches;

        FEXCore::Context::ConfigureAOTGen(CTX, &ExternalBranches, SectionMaxAddress);

        std::set<uint64_t> Compiled;
        int counter = 0;
        do {        
          LogMan::Msg::I("Discovered %ld Branch Targets in this pass", BranchTargets.size());
          for (auto RIP: BranchTargets) {
            if ((counter++) % 1000 == 0)
              LogMan::Msg::I("Compiling %d %lX", counter, RIP - Section.ElfBase);
            FEXCore::Context::CompileRIP(CTX, RIP);
            Compiled.insert(RIP);
          }
          LogMan::Msg::I("\nPass Done");
          BranchTargets.clear();
          for (auto Destination: ExternalBranches) {
            if (! (Destination >= Section.Base && Destination <= (Section.Base + Section.Size)) )
              continue;
            if (Compiled.contains(Destination))
              continue;
            BranchTargets.insert(Destination);
          }
          ExternalBranches.clear();
        } while (BranchTargets.size() > 0);
        LogMan::Msg::I("\nAll Done: %d", counter);
      }
    }
  } else {
    FEXCore::Context::RunUntilExit(CTX);
  }

  std::filesystem::create_directories(std::filesystem::path(FEXCore::Config::GetDataDirectory()) / "aotir");

  FEXCore::Context::WriteFilesWithCode(CTX, [](const std::string& fileid, const std::string& filename) {
    auto filepath = std::filesystem::path(FEXCore::Config::GetDataDirectory()) / "aotir" / (fileid + ".path");
    int fd = open(filepath.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (fd != -1) {
      write(fd, filename.c_str(), filename.size());
      close(fd);
    }
  });

  if (AOTIRCapture() || AOTIRGenerate()) {

    auto WroteCache = FEXCore::Context::WriteAOTIR(CTX, [](const std::string& fileid) -> std::unique_ptr<std::ostream> {
      auto filepath = std::filesystem::path(FEXCore::Config::GetDataDirectory()) / "aotir" / (fileid + ".aotir");
      auto AOTWrite = std::make_unique<std::ofstream>(filepath, std::ios::out | std::ios::binary);
      if (*AOTWrite) {
        std::filesystem::resize_file(filepath, 0);
        AOTWrite->seekp(0);
        LogMan::Msg::I("AOTIR: Storing %s", fileid.c_str());
      } else {
        LogMan::Msg::I("AOTIR: Failed to store %s", fileid.c_str());
      }
      return AOTWrite;
    });

    if (WroteCache) {
      LogMan::Msg::I("AOTIR Cache Stored");
    } else {
      LogMan::Msg::E("AOTIR Cache Store Failed");
    }
  }

  DisableUserDispatch();
  auto ProgramStatus = FEXCore::Context::GetProgramStatus(CTX);

  SyscallHandler.reset();
  SignalDelegation.reset();
  FEXCore::Context::DestroyContext(CTX);

  FEXCore::Config::Shutdown();

  LogMan::Throw::UnInstallHandlers();
  LogMan::Msg::UnInstallHandlers();

  if (OutputFD != stderr &&
      OutputFD != stdout &&
      OutputFD != nullptr) {
    fclose(OutputFD);
  }

  if (ShutdownReason == FEXCore::Context::ExitReason::EXIT_SHUTDOWN) {
    return ProgramStatus;
  }
  else {
    return -64 | ShutdownReason;
  }
}
