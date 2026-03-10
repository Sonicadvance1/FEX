#pragma once
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

enum cudaError_enum {};

enum CUdevice_attribute {};

typedef int CUdevice_v1;
typedef CUdevice_v1 CUdevice;
typedef enum cudaError_enum CUresult;
typedef struct CUasyncCallbackEntry_st* CUasyncCallbackHandle;

typedef struct CUctx_st* CUcontext;

CUresult cuCtxCreate_v2(CUcontext* pctx, unsigned int flags, CUdevice dev);
CUresult cuInit(unsigned int Flags);

CUresult cuDeviceGet(CUdevice* device, int ordinal);
CUresult cuDeviceGetName(char* name, int len, CUdevice dev);
CUresult cuMemGetInfo_v2(size_t* free, size_t* total);
CUresult cuDeviceGetCount(int* count);
CUresult cuDeviceGetAttribute(int* pi, CUdevice_attribute attrib, CUdevice dev);
CUresult cuCtxDestroy(CUcontext ctx);
CUresult cuCtxDestroy_v2(CUcontext ctx);

CUresult cuDeviceUnregisterAsyncNotification(CUdevice device, CUasyncCallbackHandle callback);

#ifdef __cplusplus
}
#endif
