/*
$info$
tags: backend|interpreter
$end_info$
*/

#include "Interface/Core/Interpreter/InterpreterClass.h"
#include "Interface/Core/Interpreter/InterpreterOps.h"
#include "Interface/Core/Interpreter/InterpreterDefines.h"

#include <cstdint>
#ifdef _M_X86_64
#include <xmmintrin.h>
#endif

namespace FEXCore::CPU {
[[noreturn]]
static void StopThread(FEXCore::Core::InternalThreadState *Thread) {
  Thread->CTX->StopThread(Thread);

  LOGMAN_MSG_A_FMT("unreachable");
  FEX_UNREACHABLE;
}

#define DEF_OP(x) void InterpreterOps::Op_##x(IR::IROp_Header *IROp, IROpData *Data, IR::NodeID Node)
DEF_OP(Fence) {
  auto Op = IROp->C<IR::IROp_Fence>();
  switch (Op->Fence) {
    case IR::Fence_Load.Val:
      std::atomic_thread_fence(std::memory_order_acquire);
      break;
    case IR::Fence_LoadStore.Val:
      std::atomic_thread_fence(std::memory_order_seq_cst);
      break;
    case IR::Fence_Store.Val:
      std::atomic_thread_fence(std::memory_order_release);
      break;
    default: LOGMAN_MSG_A_FMT("Unknown Fence: {}", Op->Fence); break;
  }
}

DEF_OP(Break) {
  auto Op = IROp->C<IR::IROp_Break>();
  switch (Op->Reason) {
    case FEXCore::IR::Break_Halt: // HLT
      StopThread(Data->State);
    break;
    case FEXCore::IR::Break_InvalidInstruction:
      tgkill(Data->State->ThreadManager.PID, Data->State->ThreadManager.TID, SIGILL);
    break;
  default: LOGMAN_MSG_A_FMT("Unknown Break Reason: {}", Op->Reason); break;
  }
}

DEF_OP(GetRoundingMode) {
  uint32_t GuestRounding{};
#ifdef _M_ARM_64
  uint64_t Tmp{};
  __asm(R"(
    mrs %[Tmp], FPCR;
  )"
  : [Tmp] "=r" (Tmp));
  LogMan::Msg::D("GetRoundingMode: FPCR: 0x%08x", Tmp);
  // Extract the rounding
  // On ARM the ordering is different than on x86
  GuestRounding |= ((Tmp >> 24) & 1) ? IR::ROUND_MODE_FLUSH_TO_ZERO : 0;
  uint8_t RoundingMode = (Tmp >> 22) & 0b11;
  if (RoundingMode == 0)
    GuestRounding |= IR::ROUND_MODE_NEAREST;
  else if (RoundingMode == 1)
    GuestRounding |= IR::ROUND_MODE_POSITIVE_INFINITY;
  else if (RoundingMode == 2)
    GuestRounding |= IR::ROUND_MODE_NEGATIVE_INFINITY;
  else if (RoundingMode == 3)
    GuestRounding |= IR::ROUND_MODE_TOWARDS_ZERO;

  // Extract the ExceptionMask from the FPCR
  // Remember that x86 uses a mask, AArch64 uses enable, so it needs to be inverted
  // Mapping
  // Reiterated from IR.json for clarity
  //   [3]: DAZ - Denormals Are Zero",
  //        x86:     MXCSR[6]",
  //        AArch64: FPCR[0] (Only when AFP is supported)",
  //   Masks are set to mask the exception",
  //   AArch64 needs to invert these",
  //   [4]: Invalid Operation Exception Mask",
  //        x86:     MXCSR[7]",
  //        AArch64: FPCR[8]",
  //   [5]: Denormalized Operands Exception Mask",
  //        x86:     MXCSR[8]",
  //        AArch64: FPCR[15]",
  //   [6]: Zero-Divide Exception Mask",
  //        x86:     MXCSR[9]",
  //        AArch64: FPCR[9]",
  //   [7]: Overflow Exception Mask",
  //        x86:     MXCSR[10]",
  //        AArch64: FPCR[10]",
  //   [8]: Underflow Exception Mask",
  //        x86:     MXCSR[11]",
  //        AArch64: FPCR[11]",
  //   [9]: Precision Exception Mask",
  //        x86:     MXCSR[12]",
  //        AArch64: FPCR[12]",


  uint32_t HostExceptionMask = ~Tmp;

  // DAZ
  GuestRounding |= ((HostExceptionMask >> 0) & 1) << 3;

  // IOE
  GuestRounding |= ((HostExceptionMask >> 8) & 1) << 4;

  // DZE
  GuestRounding |= ((HostExceptionMask >> 9) & 1) << 6;

  // OFE
  GuestRounding |= ((HostExceptionMask >> 10) & 1) << 7;

  // UFE
  GuestRounding |= ((HostExceptionMask >> 11) & 1) << 8;

  // IXE
  GuestRounding |= ((HostExceptionMask >> 12) & 1) << 9;

  // IDE
  GuestRounding |= ((HostExceptionMask >> 15) & 1) << 5;

  LogMan::Msg::D("Guest Rounding is 0x%x", GuestRounding);
#else
  uint32_t HostRounding = _mm_getcsr();

  // Extract the rounding
  GuestRounding = (HostRounding >> 13) & 0b111;

  // Extract the exception mask
  GuestRounding |= ((HostRounding >> 6) & 0b1111111) << 3;
#endif
  memcpy(GDP, &GuestRounding, sizeof(GuestRounding));
}

DEF_OP(SetRoundingMode) {
  auto Op = IROp->C<IR::IROp_SetRoundingMode>();
  uint32_t GuestRounding = *GetSrc<uint32_t*>(Data->SSAData, Op->Header.Args[0]);
#ifdef _M_ARM_64
  uint64_t HostRounding{};
  __asm volatile(R"(
    mrs %[Tmp], FPCR;
  )"
  : [Tmp] "=r" (HostRounding));
  // Mask out the rounding
  HostRounding &= ~(0b111 << 22);

  // Mask out exception mask
  constexpr uint32_t AArch64ExceptionMask =
    (1U << 0) |  // FIZ - Flush Inputs to Zero
    (1U << 8) |  // IOE - Invalid Operation Exception enable
    (1U << 9) |  // DZE - Divide by Zero exception enable
    (1U << 10) | // OFE - Overflow exception enable
    (1U << 11) | // UFE - Underflow exception enable
    (1U << 12) | // IXE - Inexact exception enable
    (1U << 15);  // IDE - Input denormal exception enable

  HostRounding &= ~(AArch64ExceptionMask);

  // Insert flush to zero
  HostRounding |= (GuestRounding & IR::ROUND_MODE_FLUSH_TO_ZERO) ? (1U << 24) : 0;

  // Insert the rounding mode
  uint8_t RoundingMode = GuestRounding & 0b11;
  if (RoundingMode == IR::ROUND_MODE_NEAREST)
    HostRounding |= (0b00U << 22);
  else if (RoundingMode == IR::ROUND_MODE_POSITIVE_INFINITY)
    HostRounding |= (0b01U << 22);
  else if (RoundingMode == IR::ROUND_MODE_NEGATIVE_INFINITY)
    HostRounding |= (0b10U << 22);
  else if (RoundingMode == IR::ROUND_MODE_TOWARDS_ZERO)
    HostRounding |= (0b11U << 22);

  // Insert the exception mask
  // The one provided is a mask, where AArch64 is a trap enable bit
  // This means we need to invert the incoming modes
  uint32_t GuestExceptionMask = ~((GuestRounding >> 3) & 0b1111111);

  // Mapping
  // Reiterated from IR.json for clarity
  //   [3]: DAZ - Denormals Are Zero",
  //        x86:     MXCSR[6]",
  //        AArch64: FPCR[0] (Only when AFP is supported)",
  //   Masks are set to mask the exception",
  //   AArch64 needs to invert these",
  //   [4]: Invalid Operation Exception Mask",
  //        x86:     MXCSR[7]",
  //        AArch64: FPCR[8]",
  //   [5]: Denormalized Operands Exception Mask",
  //        x86:     MXCSR[8]",
  //        AArch64: FPCR[15]",
  //   [6]: Zero-Divide Exception Mask",
  //        x86:     MXCSR[9]",
  //        AArch64: FPCR[9]",
  //   [7]: Overflow Exception Mask",
  //        x86:     MXCSR[10]",
  //        AArch64: FPCR[10]",
  //   [8]: Underflow Exception Mask",
  //        x86:     MXCSR[11]",
  //        AArch64: FPCR[11]",
  //   [9]: Precision Exception Mask",
  //        x86:     MXCSR[12]",
  //        AArch64: FPCR[12]",

  // Now that we've inverted the mask we just need to insert
  // DAZ
  HostRounding |= ((GuestExceptionMask >> 0) & 1) << 0;

  // IOE
  HostRounding |= ((GuestExceptionMask >> 1) & 1) << 8;

  // DZE
  HostRounding |= ((GuestExceptionMask >> 3) & 1) << 9;

  // OFE
  HostRounding |= ((GuestExceptionMask >> 4) & 1) << 10;

  // UFE
  HostRounding |= ((GuestExceptionMask >> 5) & 1) << 11;

  // IXE
  HostRounding |= ((GuestExceptionMask >> 6) & 1) << 12;

  // IDE
  HostRounding |= ((GuestExceptionMask >> 2) & 1) << 15;
  LogMan::Msg::D("Setting Host FPCR to 0x%08x", HostRounding);

  __asm volatile(R"(
    msr FPCR, %[Tmp];
  )"
  :: [Tmp] "r" (HostRounding));


  __asm volatile(R"(
    mrs %[Tmp], FPCR;
  )"
  : [Tmp] "=r" (HostRounding));
  LogMan::Msg::D("\tAfter set it became 0x%08x", HostRounding);
#else
  uint32_t HostRounding{};
  // Insert our new rounding mode
  HostRounding |= (GuestRounding & 0b111) << 13;

  // Insert the new exception mask
  HostRounding |= ((GuestRounding >> 3) & 0b1111111) << 6;

  // This implicitly clears the host's exception flags

  _mm_setcsr(HostRounding);
#endif
}

DEF_OP(Print) {
  auto Op = IROp->C<IR::IROp_Print>();
  uint8_t OpSize = IROp->Size;

  if (OpSize <= 8) {
    uint64_t Src = *GetSrc<uint64_t*>(Data->SSAData, Op->Header.Args[0]);
    LogMan::Msg::IFmt(">>>> Value in Arg: 0x{:x}, {}", Src, Src);
  }
  else if (OpSize == 16) {
    __uint128_t Src = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[0]);
    uint64_t Src0 = Src;
    uint64_t Src1 = Src >> 64;
    LogMan::Msg::IFmt(">>>> Value[0] in Arg: 0x{:x}, {}", Src0, Src0);
    LogMan::Msg::IFmt("     Value[1] in Arg: 0x{:x}, {}", Src1, Src1);
  }
  else
    LOGMAN_MSG_A_FMT("Unknown value size: {}", OpSize);
}

DEF_OP(ProcessorID) {
  uint32_t CPU, CPUNode;
  ::getcpu(&CPU, &CPUNode);
  GD = (CPUNode << 12) | CPU;
}

#undef DEF_OP

} // namespace FEXCore::CPU
