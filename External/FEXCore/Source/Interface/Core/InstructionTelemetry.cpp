#include "Interface/Core/InstructionTelemetry.h"
#include <atomic>
#include <cstring>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <FEXCore/Utils/Allocator.h>
#include <FEXCore/Utils/LogManager.h>
#include <FEXCore/Config/Config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>

namespace FEXCore::InstructionTelemetry {

  InstDetails::CountsPool *InstDetails::GetTLSPool() {
    std::unique_lock<std::mutex> lk {PoolLock};
    uint32_t TID = ::gettid();
    auto it = CountsPools.find(TID);
    if (it == CountsPools.end()) {
      CountsPools.emplace(TID, CountsPool{});
      it = CountsPools.find(TID);
    }
    return &it->second;
  }

  InstDetails::~InstDetails() {
    std::unique_lock<std::mutex> lk {PoolLock};
    for (auto &Pool : CountsPools) {
      for (auto Alloc : Pool.second.Pools) {
        free(Alloc);
      }
    }
  }

  uint64_t *InstDetails::AllocateAtomicCounter(InstDetails::CountsPool *Pool) {
    if (Pool->Pools.empty() ||
        Pool->Offset == InstDetails::CountsPool::POOL_SIZE) {

      // Only increment index if we weren't empty
      if (!Pool->Pools.empty()) {
        ++Pool->Index;
      }

      // Add new pool
      uint8_t *Ptr = (uint8_t*)malloc(InstDetails::CountsPool::POOL_SIZE);
      Pool->Pools.push_back(Ptr);
      memset(Ptr, 0, InstDetails::CountsPool::POOL_SIZE);
      // Set our Offset to zero
      Pool->Offset = 0;
    }

    uint8_t *Ptr = Pool->Pools.at(Pool->Index) + Pool->Offset;
    Pool->Offset += 8;
    return reinterpret_cast<uint64_t*>(Ptr);
  }

  ProgramRange InstDetails::FindProgramName(uint64_t RIP) {
    // Open our maps file and find where the memory is mapped
    std::fstream fs;
    fs.open("/proc/self/maps", std::fstream::in | std::fstream::binary);
    std::string Line;
    struct MappingDetail {
      uint64_t Begin, End;
      std::string Name;
    };
    std::vector<MappingDetail> Details;
    while (std::getline(fs, Line)) {
      if (fs.eof()) break;

      uint64_t Begin, End;

      const char *Str = Line.c_str();
      if (sscanf(Str, "%lx-%lx", &Begin, &End) == 2) {
        // Skip mem mapping
        Str = strchr(Str, ' ');
        // Skip permissions
        Str = strchr(Str+1, ' ');
        // skip...Size?
        Str = strchr(Str+1, ' ');
        // Skip inode pair
        Str = strchr(Str+1, ' ');
        // Skip....file size?
        Str = strchr(Str+1, ' ');
        while (Str[0] == ' ') ++Str;

        if (strlen(Str) != 0) {
          Details.emplace_back(MappingDetail{Begin, End, Str});
//          LogMan::Msg::D("\t%lx-%lx %s", Begin, End, Str);
        }
        else {
          Details.emplace_back(MappingDetail{Begin, End, ""});
        }
      }
    }
    fs.close();

    std::string CurrentName{};
    struct ProgramRange CurrentRange{};
    std::vector<ProgramRange> Ranges;

    for (auto it = Details.begin(); it != Details.end(); ++it) {
      if (CurrentName != it->Name) {
        if (!CurrentName.empty() && it->Name.empty()) {
          // If we have some extraneous ranges without name, concat
          CurrentRange.End = it->End;
          continue;
        }

        if (!CurrentName.empty()) {
          // Name changed. Switch over
          Ranges.emplace_back(CurrentRange);
        }

        CurrentName = it->Name;
        CurrentRange.Name = it->Name;

        // Never before hit instance
        CurrentRange.Begin = it->Begin;
      }

      // Update end of range
      CurrentRange.End = it->End;
    }

    // Make sure to push back final range
    Ranges.emplace_back(CurrentRange);

    for (auto it = Ranges.begin(); it != Ranges.end(); ++it) {
      if (RIP >= it->Begin && RIP < it->End) {
        return *it;
      }
    }

    return {0, 0, "<Unknown>"};
  }

  void InstDetails::SaveResults() {
    struct stat st{};
    if (stat("/tmp/FEXDetails", &st) == -1) {
      mkdir("/tmp/FEXDetails/", 0777);
    }
    FEX_CONFIG_OPT(AppFilename, APP_FILENAME);
    std::filesystem::path Path(AppFilename());
    std::string Filename = "/tmp/FEXDetails/FEX_" + Path.filename().string() + "_" + std::to_string(::getpid()) + ".txt";

    FILE *fp;
    fp = fopen(Filename.c_str(), "w");
    bool Coalesce = true;
    uint64_t Totals = 0;
    if (Coalesce) {
      std::map<uint64_t, DetailStruct> PIDDetails;
      for (auto &Detail : Details) {
        auto PIDDetail = PIDDetails.find(Detail.first.first);
        if (PIDDetail == PIDDetails.end()) {
          // Doesn't exist yet, just add
          PIDDetails.emplace(Detail.first.first, Detail.second);
        }
        else {
          auto PIDName = PIDDetail->second.ProgramName;
          if (PIDName != Detail.second.ProgramName) {
            // process mismatch over time
            PIDDetail->second.ProgramName = "<Conflicted>";
          }
          // Combine counts
          *PIDDetail->second.Count += *Detail.second.Count;
        }
      }
      fprintf(fp, "TYPE, COUNT, RIP, Base, End, Program, Total\n");

      for (auto &Detail : PIDDetails) {
        uint64_t Count = std::atomic_load<uint64_t>((std::atomic<uint64_t>*)Detail.second.Count);
        Totals += Count;
      }

      for (auto &Detail : PIDDetails) {
        uint64_t Count = std::atomic_load<uint64_t>((std::atomic<uint64_t>*)Detail.second.Count);
        fprintf(fp, "%016ld, %016ld, 0x%016lx, 0x%lx, 0x%lx, %s, %016ld\n",
          Detail.second.InstructionType,
          Count,
          Detail.first,
          Detail.second.ProgramBase, Detail.second.ProgramEnd, Detail.second.ProgramName.c_str(),
          Totals);
      }
    }
    else {
      fprintf(fp, "TID, COUNT, RIP, Base, End, Program\n");

      for (auto &Detail : Details) {
        fprintf(fp, "%d, 0x%016lx, 0x%016lx, 0x%lx, 0x%lx, %s\n",
          Detail.first.second,
          std::atomic_load<uint64_t>((std::atomic<uint64_t>*)Detail.second.Count),
          Detail.first.first,
          Detail.second.ProgramBase, Detail.second.ProgramEnd, Detail.second.ProgramName.c_str());
      }
    }
    fclose(fp);
    LogMan::Msg::D("Saved to '%s'", Filename.c_str());
  }

  DetailStruct *InstDetails::AddInstructionData(uint64_t RIP, uint64_t InstructionType, FEXCore::X86Tables::X86InstInfo const* TableInfo) {
    ProgramRange ProgramName(FindProgramName(RIP));
    std::string const *ProgramNameDB{};
    {
      std::unique_lock<std::mutex> lk {ProgramNamesLock};
      if (ProgramNames.find(ProgramName.Name) == ProgramNames.end()) {
        ProgramNames.emplace(ProgramName.Name);
      }

      auto it = ProgramNames.find(ProgramName.Name);
      ProgramNameDB = &(*it);
    }

    std::pair<uint32_t, uint32_t> Key(RIP, ::gettid());
    std::unique_lock<std::mutex> lk {DetailsLock};
    auto Detail = Details.find(Key);
    if (Detail == Details.end()) {
      DetailStruct NewData = {
        .Count = AllocateAtomicCounter(GetTLSPool()),
        .InstructionType = InstructionType,
        .ProgramName = *ProgramNameDB,
        .ProgramBase = ProgramName.Begin,
        .ProgramEnd = ProgramName.End,
      };
      auto Detail = Details.emplace(Key, NewData);
      return &Detail.first->second;
    }
    else {
      return &Detail->second;
    }
  }
}

