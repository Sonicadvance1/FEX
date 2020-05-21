#pragma once
#include <cmath>
#include <cstring>
#include <stdint.h>
#include <string>
#include <sstream>

extern "C" {
#include "SoftFloat-3e/platform.h"
#include "SoftFloat-3e/softfloat.h"
}

struct X80SoftFloat {
  struct __attribute__((packed)) {
    uint64_t Significand : 64;
    uint16_t Exponent    : 15;
    unsigned Sign        : 1;
  };

  X80SoftFloat() { memset(this, 0, sizeof(*this)); }
  X80SoftFloat(unsigned _Sign, uint16_t _Exponent, uint64_t _Significand)
    : Significand {_Significand}
    , Exponent {_Exponent}
    , Sign {_Sign}
    {
  }

  std::string str() {
    std::ostringstream string;
    string << std::hex << Sign;
    string << "_" << Exponent;
    string << "_" << (Significand >> 63);
    string << "_" << (Significand & ((1ULL << 63) - 1));
    return string.str();
  }

  // Ops
  static X80SoftFloat FADD(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    return extF80_add(lhs, rhs);
  }

  static X80SoftFloat FSUB(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    return extF80_sub(lhs, rhs);
  }

  static X80SoftFloat FMUL(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    return extF80_mul(lhs, rhs);
  }

  static X80SoftFloat FDIV(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    return extF80_div(lhs, rhs);
  }

  static X80SoftFloat FATAN(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    double Src1_d = lhs;
    double Src2_d = rhs;
    double Tmp = atan2(Src1_d, Src2_d);
    return Tmp;
  }

  static X80SoftFloat FREM(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    return extF80_rem(lhs, rhs);
  }

  static X80SoftFloat FREM1(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    return extF80_rem(lhs, rhs);
  }

  static X80SoftFloat FSCALE(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    double Src1_d = lhs;
    double Src2_d = rhs;
    double Tmp = Src1_d * exp2(trunc(Src2_d));
    return Tmp;
  }

  static X80SoftFloat FRNDINT(X80SoftFloat const &lhs) {
    return extF80_roundToInt(lhs, softfloat_round_near_even, false);
  }

  static X80SoftFloat F2XM1(X80SoftFloat const &lhs) {
    double Src_d = lhs;
    Src_d = exp2(Src_d) - 1.0;
    return Src_d;
  }

  static X80SoftFloat FYL2X(X80SoftFloat const &lhs, X80SoftFloat const &rhs) {
    double Src1_d = lhs;
    double Src2_d = rhs;
    double Tmp = Src2_d * log2l(Src1_d);
    return Tmp;
  }

  static X80SoftFloat FTAN(X80SoftFloat const &lhs) {
    double Src_d = lhs;
    Src_d = tan(Src_d);
    return Src_d;
  }

  static X80SoftFloat FSQRT(X80SoftFloat const &lhs) {
    return extF80_sqrt(lhs);
  }

  static X80SoftFloat FSIN(X80SoftFloat const &lhs) {
    double Src_d = lhs;
    Src_d = sin(Src_d);
    return Src_d;
  }

  static X80SoftFloat FCOS(X80SoftFloat const &lhs) {
    double Src_d = lhs;
    Src_d = cos(Src_d);
    return Src_d;
  }

  static X80SoftFloat FXTRACT_EXP(X80SoftFloat const &lhs) {
    int32_t TrueExp = lhs.Exponent - ExponentBias;
    return i32_to_extF80(TrueExp);
  }

  static X80SoftFloat FXTRACT_SIG(X80SoftFloat const &lhs) {
    X80SoftFloat Tmp = lhs;
    Tmp.Exponent = 0x3FFF;
    Tmp.Sign = lhs.Sign;
    return Tmp;
  }

  static void FCMP(X80SoftFloat const &lhs, X80SoftFloat const &rhs, bool *eq, bool *lt, bool *nan) {
    *eq = extF80_eq(lhs, rhs);
    *lt = extF80_lt(lhs, rhs);
    *nan = IsNan(lhs) || IsNan(rhs);
  }

  operator float() const {
    float32_t Result = extF80_to_f32(*this);
    return *(float*)&Result;
  }

  operator double() const {
    float64_t Result = extF80_to_f64(*this);
    return *(double*)&Result;
  }

  operator int16_t() const {
    return extF80_to_i32(*this, softfloat_round_near_even, false);
  }

  operator int32_t() const {
    return extF80_to_i32(*this, softfloat_round_near_even, false);
  }

  operator int64_t() const {
    return extF80_to_i64(*this, softfloat_round_near_even, false);
  }

  void operator=(const float rhs) {
    *this = f32_to_extF80(*(float32_t*)&rhs);
  }

  void operator=(const double rhs) {
    *this = f64_to_extF80(*(float64_t*)&rhs);
  }

  void operator=(const int16_t rhs) {
    *this = i32_to_extF80(rhs);
  }

  void operator=(const int32_t rhs) {
    *this = i32_to_extF80(rhs);
  }

  operator void*() {
    return reinterpret_cast<void*>(this);
  }

#ifdef _M_X86_64
  X80SoftFloat(long double rhs) {
    // Long double here is matching our packing
    memcpy(this, &rhs, 10);
  }

  void operator=(long double rhs) {
    memcpy(this, &rhs, 10);
  }

  operator long double() const {
    return *(long double*)this;
  }
#endif

  X80SoftFloat(extFloat80_t rhs) {
    memcpy(this, &rhs, sizeof(*this));
  }

  void operator=(extFloat80_t rhs) {
    memcpy(this, &rhs, sizeof(*this));
  }

  operator extFloat80_t() const {
    return *(extFloat80_t*)this;
  }

  static bool IsNan(X80SoftFloat const &lhs) {
    return (lhs.Exponent == 0x7FFF) &&
      (lhs.Significand & IntegerBit) &&
      (lhs.Significand & Bottom62Significand);
  }

private:
  static constexpr uint64_t IntegerBit = (1ULL << 63);
  static constexpr uint64_t Bottom62Significand = ((1ULL << 62) - 1);
  static constexpr uint32_t ExponentBias = 16383;
};

static_assert(sizeof(X80SoftFloat) == 10, "tword must be 10bytes in size");
