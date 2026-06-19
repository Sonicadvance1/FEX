// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CUhostFn)(void*);
enum cudaError_enum {};
enum CUgreenCtxCreate_flags {};
enum CUdevResourceType {};

struct anti_zluda_result {
  uint64_t low, high;
};

#ifdef __cplusplus
}
#endif
