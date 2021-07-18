
#!/usr/bin/python3
from ThunkHelpers import *

lib("libEGL")

# EGLDisplay, EGLSurface, EGLContext is type void*
# So need to convert to compat_ptr<void>
fn32("EGLBoolean eglBindAPI(EGLenum)")
fn32("EGLBoolean eglChooseConfig(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*)",
     "EGLBoolean eglChooseConfig(compat_ptr<void /*EGLDisplay*/>, compat_ptr<EGLint>, compat_ptr<void /*EGLConfig*/>, EGLint, compat_ptr<EGLint>)")

fn32("EGLBoolean eglDestroyContext(EGLDisplay, EGLContext)",
     "EGLBoolean eglDestroyContext(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLSurface*/>)")

fn32("EGLBoolean eglDestroySurface(EGLDisplay, EGLSurface)",
     "EGLBoolean eglDestroySurface(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLSurface*/>)")

fn32("EGLBoolean eglInitialize(EGLDisplay, EGLint*, EGLint*)",
     "EGLBoolean eglInitialize(compat_ptr<void /*EGLDisplay*/>, compat_ptr<EGLint>, compat_ptr<EGLint>)"  )

fn32("EGLBoolean eglMakeCurrent(EGLDisplay, EGLSurface, EGLSurface, EGLContext)",
     "EGLBoolean eglMakeCurrent(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLSurface*/>, compat_ptr<void /*EGLSurface*/>, compat_ptr<void /*EGLContext*/>)")

fn32("EGLBoolean eglQuerySurface(EGLDisplay, EGLSurface, EGLint, EGLint*)",
     "EGLBoolean eglQuerySurface(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLSurface*/>, EGLint, compat_ptr<EGLint>)")

fn32("EGLBoolean eglSurfaceAttrib(EGLDisplay, EGLSurface, EGLint, EGLint)",
     "EGLBoolean eglSurfaceAttrib(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLSurface*/>, EGLint, EGLint)")

fn32("EGLBoolean eglSwapBuffers(EGLDisplay, EGLSurface)",
     "EGLBoolean eglSwapBuffers(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLSurface*/>)")

fn32("EGLBoolean eglTerminate(EGLDisplay)",
     "EGLBoolean eglTerminate(compat_ptr<void /*EGLDisplay*/>)")

fn32("EGLint eglGetError()")
fn32("EGLContext eglCreateContext(EGLDisplay, EGLConfig, EGLContext, const EGLint*)",
     "compat_ptr<void> eglCreateContext(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLConfig*/>, compat_ptr<void /*EGLContext*/>, compat_ptr<EGLint>)")

fn32("EGLSurface eglCreateWindowSurface(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*)",
     "compat_ptr<void /*EGLSurface*/> eglCreateWindowSurface(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLConfig*/>, compat_ptr<EGLNativeWindowType>, compat_ptr<EGLint>)")
no_unpack()

fn32("EGLContext eglGetCurrentContext()",
     "compat_ptr<void /*EGLContext*/> eglGetCurrentContext()")

fn32("EGLDisplay eglGetCurrentDisplay()",
     "compat_ptr<void /*EGLDisplay*/> eglGetCurrentDisplay()")

fn32("EGLSurface eglGetCurrentSurface(EGLint)",
     "compat_ptr<void /*EGLSurface*/> eglGetCurrentSurface(EGLint)")

fn32("EGLDisplay eglGetDisplay(EGLNativeDisplayType)",
     "compat_ptr<void /*EGLDisplay*/> eglGetDisplay(compat_ptr<void>)")
no_unpack()

fn32("const char* eglQueryString(EGLDisplay, EGLint)",
     "compat_ptr<const char> eglQueryString(compat_ptr<void /*EGLDisplay*/>, EGLint)")

fn32("EGLBoolean eglGetConfigs(EGLDisplay, EGLConfig *, EGLint, EGLint *)",
     "EGLBoolean eglGetConfigs(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLConfig*/>, EGLint, compat_ptr<EGLint>)")

fn32("EGLBoolean eglGetConfigAttrib(EGLDisplay, EGLConfig, EGLint, EGLint *)",
     "EGLBoolean eglGetConfigAttrib(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLConfig*/>, EGLint, compat_ptr<EGLint>)")

fn32("EGLDisplay eglGetPlatformDisplayEXT(EGLenum, void *, const EGLint *)",
     "compat_ptr<void /*EGLDisplay*/> eglGetPlatformDisplayEXT(EGLenum, compat_ptr<void>, compat_ptr<EGLint>)")

fn32("EGLSurface eglCreatePixmapSurface(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *)"
     "compat_ptr<void /*EGLSurface*/> eglCreatePixmapSurface(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLConfig*/>, compat_ptr<void /*EGLNativePixmapType*/>, compat_ptr<EGLint>)")

# EGL_MESA_query_driver
fn32("char *eglGetDisplayDriverConfig(EGLDisplay)",
     "compat_ptr<char> eglGetDisplayDriverConfig(compat_ptr<void /*EGLDisplay*/>)")
fn32("char *eglGetDisplayDriverName(EGLDisplay)",
     "compat_ptr<char> eglGetDisplayDriverName(compat_ptr<void /*EGLDisplay*/>)")

# EGL_EXT_device_base
# EGLDeviceEXT is also void*
# needs to convert to compat_ptr<void>
fn32("EGLBoolean eglQueryDeviceAttribEXT(EGLDeviceEXT, EGLint, EGLAttrib *)",
    "EGLBoolean eglQueryDeviceAttribEXT(compat_ptr<void /*EGLDeviceEXT*/>, EGLint, compat_ptr<EGLAttrib>)")
fn32("char *eglQueryDeviceStringEXT(EGLDeviceEXT, EGLint)",
    "compat_ptr<char> eglQueryDeviceStringEXT(compat_ptr<void /*EGLDeviceEXT*/>, EGLint)")
fn32("EGLBoolean eglQueryDevicesEXT(EGLint, EGLDeviceEXT *, EGLint *)",
     "EGLBoolean eglQueryDevicesEXT(EGLint, compat_ptr<compat_ptr<void /*EGLDeviceEXT*/>>, compat_ptr<EGLint>)")
no_unpack()

fn32("EGLBoolean eglQueryDisplayAttribEXT(EGLDisplay, EGLint, EGLAttrib *)",
     "EGLBoolean eglQueryDisplayAttribEXT(compat_ptr<void /*EGLDisplay*/>, EGLint, compat_ptr<EGLAttrib>)")

fn32("EGLBoolean eglQueryContext(EGLDisplay, EGLContext, EGLint, EGLint *)",
     "EGLBoolean eglQueryContext(compat_ptr<void /*EGLDisplay*/>, compat_ptr<void /*EGLContext*/>, EGLint, compat_ptr<EGLint>)")


Generate()
