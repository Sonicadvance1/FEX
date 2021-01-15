#pragma once
#include <cstdint>
#include <functional>
#include <vector>

namespace FEXCore {
namespace Core {
  struct InternalThreadState;
}

namespace IR {
class IREmitter;
}

/**
 * @brief Code loader class so the CPU backend can load code in a generic fashion
 *
 * This class is expected to have multiple different style of code loaders
*/
class CodeLoader {
public:

  /**
   * @brief CPU Core uses this to choose what the stack size should be for this code
   */
  virtual uint64_t StackSize() const = 0;
  /**
   * @brief Allows the code loader to set up the stack the way it wants
   *
   * @param HostPtr The host facing pointer to the base of the stack.
   * Size of memory will be at least the size that StackSize() returns
   *
   * @param GuestPtr The guest facing memory location where the base of the stack lives
   *
   * @return The location that the guest stack pointer register should be set to
   *
   * Probably will be GuestPtr + StackSize() - <Some amount>
   */
  virtual uint64_t SetupStack() = 0;

  /**
   * @brief Function to return the guest RIP that the code should start out at
   */
  virtual uint64_t DefaultRIP() const = 0;

  virtual void GetInitLocations(std::vector<uint64_t> *Locations) {}

  /**
   * @brief Allows the loader to map memory regions that it needs
   *
   * Code loader is expected to call the Mapper function with a memory offset and size for mapping
   *
   * @param Mapper Returns the host facing pointer for memory setup if the codfe loader needs to do things to it
   */
  virtual void MapMemoryRegion() {}

  /**
   * @brief Memory writer function for loading code in to guest memory
   *
   * First argument = Data to write
   * Second argument = Guest memory data location
   * Third argument = Guest memory size
   */
  virtual void LoadMemory() = 0;

  /**
   * @brief Get the final RIP we are supposed to end up on in a debugger
   *
   * @return When the debugger reaches this RIP then we know that we have completed
   */
  virtual uint64_t GetFinalRIP() { return ~0ULL; }

  virtual char const *FindSymbolNameInRange(uint64_t Address) { return nullptr; }
  virtual void GetExecveArguments(std::vector<char const*> *Args) {}

  virtual void GetAuxv(uint64_t& addr, uint64_t& size) {}

  using IRHandler = std::function<void(uint64_t Addr, FEXCore::IR::IREmitter *IR)>;
  virtual void AddIR(IRHandler Handler) {}

  virtual void InitializeThreadState(FEXCore::Core::InternalThreadState *Thread) {}
};


}
