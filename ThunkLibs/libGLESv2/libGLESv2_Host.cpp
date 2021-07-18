/*
$info$
tags: thunklibs|GLESv2
desc: Uses eglGetProcAddress instead of dlsym
$end_info$
*/

#include <alloca.h>
#include <cstdio>
#include <dlfcn.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "Defines.h"

// #define dlsym(so, name) (void*)eglGetProcAddress((const GLubyte*)name)

#include "common/Host.h"

#include "ldr_ptrs.inl"
#include "function_unpacks.inl"

// Generated: function_unpacks
extern "C" {
static void fexfn_unpack_libGLESv2_glGetString(void *argsv){
  if (fexldr_ptr_libGLESv2_glGetString == nullptr) {
    (void*&)fexldr_ptr_libGLESv2_glGetString = (void*)eglGetProcAddress("glGetString");
  }
  struct __attribute__((packed)) arg_t {GLenum a_0;compat_ptr<GLubyte> rv;};
  auto args = (arg_t*)((uint64_t)*(uint32_t*)((uint64_t)argsv + 4));
  auto Res = 
    fexldr_ptr_libGLESv2_glGetString
    (args->a_0);
  args->rv = (GLubyte*)Res;
}

  static void fexfn_unpack_libGLESv2_glShaderSource(void *argsv){
    printf("Inside of glShaderSource On the host side\n");
    if (fexldr_ptr_libGLESv2_glShaderSource == nullptr) {
      printf("dlsym couldn't load 'glShaderSource'. Trying to reload \n");
      (void*&)fexldr_ptr_libGLESv2_glShaderSource = (void*)eglGetProcAddress("glShaderSource");
      if (fexldr_ptr_libGLESv2_glShaderSource == nullptr) printf("EGL couldn't load 'glShaderSource' \n");
      else printf("Found the function after the fact\n");
    }
    struct __attribute__((packed)) arg_t {GLuint a_0;GLsizei a_1;compat_ptr<compat_ptr<GLchar>> a_2;compat_ptr<GLint> a_3;};

    auto args = (arg_t*)((uint64_t)*(uint32_t*)((uint64_t)argsv + 4));

    GLchar **Strings = (GLchar **)alloca(sizeof(uintptr_t) * args->a_1);

    for (size_t i = 0; i < args->a_1; ++i) {
      Strings[i] = args->a_2[i];
      printf("Shader string: '%s'\n", Strings[i]);
    }

    fexldr_ptr_libGLESv2_glShaderSource
      (args->a_0,args->a_1, Strings,args->a_3);
    printf("After Host wrapped call glShaderSource \n");
  }

}
static ExportEntry exports[] = {
    #include "tab_function_unpacks.inl"
    { nullptr, nullptr }
};

#include "ldr.inl"

EXPORTS(libGLESv2)
