#pragma once
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace FEXCore::X86Tables {
struct X86InstInfo;
}

namespace FEXCore::InstructionTelemetry {
  enum InstructionType {
    OP_FADD,
    OP_FSUB,
    OP_FMUL,
    OP_FDIV,
    OP_FCHS,
    OP_FABS,
    OP_FTST,
    OP_FRNDINT,
    OP_FXTRACT,
    OP_FNINIT,
    OP_FCOMI,
    OP_FXCH,
    OP_UNARY,
    OP_BINARY,
    OP_SINCOS,
    OP_FYL2X,
    OP_TAN,
    OP_ATAN,
    OP_LDENV,
    OP_FNSTENV,
    OP_FLDCW,
    OP_FSTCW,
    OP_LDSW,
    OP_FNSTSW,
    OP_FNSAVE,
    OP_FRSTOR,
    OP_FXAM,
    OP_FCMOV,
    OP_FXSAVE,
    OP_FXRSTOR,
    OP_FIST,
    OP_FST,
    OP_FILD,
    OP_FLDCONST,
    OP_FBSTP,
    OP_FBLD,
    OP_FLD,
  };

  struct ProgramRange {
    uint64_t Begin, End;
    std::string Name;
  };

  struct DetailStruct {
    uint64_t *Count{};
    uint64_t RIP{};
    uint64_t InstructionType;
    FEXCore::X86Tables::X86InstInfo const* TableInfo;
    std::string ProgramName;
    uint64_t ProgramBase;
    uint64_t ProgramEnd;
  };

  class InstDetails final {
  public:
    DetailStruct *AddInstructionData(uint64_t RIP, uint64_t InstructionType, FEXCore::X86Tables::X86InstInfo const* TableInfo);
    void SaveResults();
    ~InstDetails();

  private:
    std::mutex DetailsLock;
    std::map<std::pair<uint64_t, uint32_t>, DetailStruct> Details;

    struct CountsPool {
      uint64_t Index;
      uint64_t Offset;
      constexpr static size_t POOL_SIZE = 4096;
      std::vector<uint8_t*> Pools;
    };

    std::map<uint32_t, CountsPool> CountsPools;
    ProgramRange FindProgramName(uint64_t RIP);
    std::mutex ProgramNamesLock;
    std::set<std::string> ProgramNames;

    std::mutex PoolLock;
    CountsPool *GetTLSPool();
    uint64_t *AllocateAtomicCounter(CountsPool *Pool);
  };
}
