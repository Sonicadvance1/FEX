#include <GL/glx.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <cstring>

#include "Thunk.h"

LOAD_LIB(libThunkLib)

#include "libThunkLib_thunks.inl"

typedef void voidFunc();

extern "C" {
  typedef uint32_t (*CallbackPtr)();
static void fexthunks_forward_libThunkLib_print(FEXCore::Context::Context *ctx, void *argsv){
struct arg_t {const char* a_0;void* a_1;};
auto args = (arg_t*)argsv;

printf("FEXCore Thunked lib! '%s'\n", args->a_0);
}
}

