// SPDX-License-Identifier: MIT
#include "Interface/Core/OpcodeDispatcher.h"
#include "Interface/Core/X86Tables/X86Tables.h"

#include <FEXCore/Core/CoreState.h>
#include <FEXCore/Core/X86Enums.h>
#include <FEXCore/Utils/EnumUtils.h>
#include <FEXCore/Utils/LogManager.h>
#include <FEXCore/Utils/FPState.h>

#include <stddef.h>
#include <stdint.h>

namespace FEXCore::IR {
class OrderedNode;
#define OpcodeArgs [[maybe_unused]] FEXCore::X86Tables::DecodedOp Op

// TODO new x87 ops
//
//
template<size_t width>
void OpDispatchBuilder::TODO_FLD(OpcodeArgs) {
  CurrentHeader->HasX87 = true;

  size_t read_width = (width == 80) ? 16 : width / 8;

  OrderedNode *data{};

  if (!Op->Src[0].IsNone()) {
    // Read from memory
    data = LoadSource_WithOpSize(FPRClass, Op, Op->Src[0], read_width, Op->Flags);
  }
  else {
    // Implicit arg
    data = _ReadStackValue(Op->OP & 7);
  }
  _PushStack(data, OpSize::i128Bit, true, read_width);
}

template
void OpDispatchBuilder::TODO_FLD<32>(OpcodeArgs);
template
void OpDispatchBuilder::TODO_FLD<64>(OpcodeArgs);
template
void OpDispatchBuilder::TODO_FLD<80>(OpcodeArgs);

template<size_t width>
void OpDispatchBuilder::TODO_FST(OpcodeArgs) {
  CurrentHeader->HasX87 = true;
  const bool Pop = (Op->TableInfo->Flags & X86Tables::InstFlags::FLAGS_POP) != 0;
  OrderedNode *Mem = LoadSource(GPRClass, Op, Op->Dest, Op->Flags, {.LoadData = false});

  if (Pop) {
    if constexpr (width == 80) {
      _PopStackMemory(Mem, OpSize::i128Bit, true, 10);
    }
    else if constexpr (width == 32 || width == 64) {
      _PopStackMemory(Mem, OpSize::i128Bit, true, width / 8);
    }
  }
  else {
    if constexpr (width == 80) {
      _StoreStackMemory(Mem, OpSize::i128Bit, true, 10);
    }
    else if constexpr (width == 32 || width == 64) {
      _StoreStackMemory(Mem, OpSize::i128Bit, true, width / 8);
    }
  }
}

template
void OpDispatchBuilder::TODO_FST<32>(OpcodeArgs);
template
void OpDispatchBuilder::TODO_FST<64>(OpcodeArgs);
template
void OpDispatchBuilder::TODO_FST<80>(OpcodeArgs);

}
