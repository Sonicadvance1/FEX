#include <cstdio>
#include <dlfcn.h>
#include <stdint.h>

#include <FEXCore/Core/Context.h>

#include "Thunk.h"

#include "libThunkLib_initializers.inl"
#include "libThunkLib_forwards.inl"

namespace FEXCore::Context {
  void HandleCallback(FEXCore::Context::Context *CTX, uint64_t RIP);
}

extern "C" {
  typedef uint32_t (*CallbackPtr)();
static void fexthunks_forward_libThunkLib_print(FEXCore::Context::Context *ctx, void *argsv){
struct arg_t {const char* a_0; CallbackPtr a_1;};
auto args = (arg_t*)argsv;

FEXCore::Context::HandleCallback(ctx, (uint64_t)args->a_1);
printf("Thunked with forward access to CTX!! '%s'\n", args->a_0);
}
}

static ExportEntry exports[] = {
    #include "libThunkLib_thunkmap.inl"
    { nullptr, nullptr }
};

EXPORTS(libThunkLib) 
