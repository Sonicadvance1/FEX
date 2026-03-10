#include <common/GeneratorInterface.h>
#include "cuda_defines.h"

template<auto>
struct fex_gen_config {
  unsigned version = 1;
};

template<typename>
struct fex_gen_type {};


template<>
struct fex_gen_type<struct CUctx_st> : fexgen::opaque_type {};
template<>
struct fex_gen_type<CUcontext> : fexgen::opaque_type {};

template<>
struct fex_gen_type<struct CUasyncCallbackEntry_st> : fexgen::opaque_type {};

template<>
struct fex_gen_type<CUasyncCallbackHandle> : fexgen::opaque_type {};

template<>
struct fex_gen_config<cuInit> {};

template<>
struct fex_gen_config<cuDeviceGet> {};

template<>
struct fex_gen_config<cuDeviceGetName> {};


#ifndef IS_32BIT_THUNK
template<>
struct fex_gen_config<cuMemGetInfo_v2> {};
#endif

template<>
struct fex_gen_config<cuDeviceGetCount> {};

template<>
struct fex_gen_config<cuDeviceGetAttribute> {};

template<>
struct fex_gen_config<cuCtxCreate_v2> {};

template<>
struct fex_gen_config<cuCtxDestroy> {};

template<>
struct fex_gen_config<cuCtxDestroy_v2> {};

template<>
struct fex_gen_config<cuDeviceUnregisterAsyncNotification> {};
