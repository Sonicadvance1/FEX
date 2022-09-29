#pragma once

#include <FEXCore/Utils/MathUtils.h>
#include <FEXCore/Utils/LogManager.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace FEXCore {

template<typename T>
struct FlexBitSet final {
  using ElementType = T;
  constexpr static size_t MinimumSize = sizeof(ElementType);
  constexpr static size_t MinimumSizeBits = sizeof(ElementType) * 8;

  T Memory[];

  bool Get(size_t Element) const {
    return (Memory[Element / MinimumSizeBits] & (1ULL << (Element % MinimumSizeBits))) != 0;
  }
  bool TestAndClear(size_t Element) {
    bool Value = Get(Element);
    Memory[Element / MinimumSizeBits] &= ~(1ULL << (Element % MinimumSizeBits));
    return Value;
  }
  void Set(size_t Element) {
    Memory[Element / MinimumSizeBits] |= (1ULL << (Element % MinimumSizeBits));
  }
  void Clear(size_t Element) {
    Memory[Element / MinimumSizeBits] &= ~(1ULL << (Element % MinimumSizeBits));
  }
  void MemClear(size_t Elements) {
    memset(Memory, 0, FEXCore::AlignUp(Elements / MinimumSizeBits, MinimumSizeBits));
  }
  void MemSet(size_t Elements) {
    memset(Memory, 0xFF, FEXCore::AlignUp(Elements / MinimumSizeBits, MinimumSizeBits));
  }

  struct BitsetScanResults {
    size_t FoundElement;
    bool FoundHole;
  };

  template<bool WantUnset>
  BitsetScanResults BackwardScanForRange(size_t BeginningElement, size_t ElementCount, size_t ElementsInSet) {
    bool FoundHole {};
    for (size_t CurrentPage = BeginningElement;
         CurrentPage >= ElementCount;) {
      size_t Remaining = ElementCount;
      LOGMAN_THROW_AA_FMT(Remaining <= CurrentPage, "Scanning less than available range");

      while (Remaining) {
        if (this->Get(CurrentPage - Remaining) == WantUnset) {
          // Has an intersecting range
          break;
        }
        --Remaining;
      }

      if (Remaining) {
        // If we found at least one Element hole then track that
        if (Remaining != ElementCount) {
          FoundHole = true;
        }

        // Didn't find a slab range
        CurrentPage -= Remaining;
      }
      else {
        // We have a slab range
        return BitsetScanResults{CurrentPage - ElementCount, FoundHole};
      }
    }

    return BitsetScanResults {~0ULL, FoundHole};
  }

  template<bool WantUnset>
  BitsetScanResults ForwardScanForRange(size_t BeginningElement, size_t ElementCount, size_t ElementsInSet) {
    bool FoundHole {};

    for (size_t CurrentElement = BeginningElement;
         CurrentElement < (ElementsInSet - ElementCount);) {
      // If we have enough free space, check if we have enough free pages that are contiguous
      size_t Remaining = ElementCount;

      assert((CurrentElement + Remaining - 1) < ElementsInSet);
      while (Remaining) {
        if (this->Get(CurrentElement + Remaining - 1) == WantUnset) {
          // Has an intersecting range
          break;
        }
        --Remaining;
      }

      if (Remaining) {
        // If we found at least one Element hole then track that
        if (Remaining != ElementCount) {
          FoundHole = true;
        }

        // Didn't find a slab range
        CurrentElement += Remaining;
      }
      else {
        // We have a slab range
        return BitsetScanResults {CurrentElement, FoundHole};
      }
    }

    return BitsetScanResults {~0ULL, FoundHole};
  }

  // This very explicitly doesn't let you take an address
  // Is only a getter
  bool operator[](size_t Element) const {
    return Get(Element);
  }

  static size_t Size(uint64_t Elements) {
    return FEXCore::AlignUp(Elements / MinimumSizeBits, MinimumSizeBits);
  }
};

static_assert(sizeof(FlexBitSet<uint64_t>) == 0, "This needs to be a flex member");
static_assert(std::is_trivially_copyable_v<FlexBitSet<uint64_t>>, "Needs to be trivially copyable");

} // namespace FEXCore
