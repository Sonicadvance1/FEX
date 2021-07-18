#!/usr/bin/python3
from ThunkHelpers import *

lib("libGLESv2")
# typedef khronos_ssize_t GLsizeiptr;
# typedef khronos_intptr_t GLintptr;
# These are 64bit or 32bit depending on host

fn32("const GLubyte* glGetString(GLenum)",
     "compat_ptr<GLubyte> glGetString(GLenum)")
no_unpack()

fn32("void glAttachShader(GLuint, GLuint)")

fn32("void glBindAttribLocation(GLuint, GLuint, const GLchar *)",
     "void glBindAttribLocation(GLuint, GLuint, compat_ptr<GLchar>)")

fn32("void glGetShaderiv(GLuint, GLenum, GLint *)",
     "void glGetShaderiv(GLuint, GLenum, compat_ptr<GLint>)")

fn32("void glClear(GLbitfield)")
fn32("void glDisableVertexAttribArray(GLuint)")

fn32("void glUniformMatrix4fv(GLint, GLsizei, GLboolean, const GLfloat*)",
     "void glUniformMatrix4fv(GLint, GLsizei, GLboolean, compat_ptr<GLfloat>)")

fn32("void glUseProgram(GLuint)")
fn32("void glClearColor(GLclampf, GLclampf, GLclampf, GLclampf)")
fn32("void glEnableVertexAttribArray(GLuint)")

fn32("void glGetProgramInfoLog(GLuint, GLsizei, GLsizei*, GLchar*)",
     "void glGetProgramInfoLog(GLuint, GLsizei, compat_ptr<GLsizei>, compat_ptr<GLchar>)")

fn32("void glGetProgramiv(GLuint, GLenum, GLint*)",
     "void glGetProgramiv(GLuint, GLenum, compat_ptr<GLint>)")

fn32("GLuint glCreateShader(GLenum)")
fn32("GLuint glCreateProgram()")
fn32("void glDrawArrays(GLenum, GLint, GLsizei)")

fn32("void glVertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)",
     "void glVertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, compat_ptr<void>)")

fn32("void glLinkProgram(GLuint)")

fn32("void glShaderSource(GLuint, GLsizei, const GLchar* const*, const GLint*)",
     "void glShaderSource(GLuint, GLsizei, compat_ptr<compat_ptr<GLchar>>, compat_ptr<GLint>)")
no_unpack()

fn32("void glViewport(GLint, GLint, GLsizei, GLsizei)")

fn32("GLint glGetUniformLocation(GLuint, const GLchar*)",
     "GLint glGetUniformLocation(GLuint, compat_ptr<GLchar>)")

fn32("void glCompileShader(GLuint)")

fn32("void* glMapBufferOES(GLenum, GLenum)",
     "compat_ptr<void> glMapBufferOES(GLenum, GLenum)")

fn32("void glEnable(GLenum)")

fn32("void glBufferData(GLenum, GLsizeiptr, const void*, GLenum)",
     "void glBufferData(GLenum, FEX::GLsizeiptr_32, compat_ptr<void>, GLenum)")

fn32("void glGenBuffers(GLsizei, GLuint*)",
     "void glGenBuffers(GLsizei, compat_ptr<GLuint>)")

fn32("void glFlush()")
fn32("void glBindBuffer(GLenum, GLuint)")

fn32("void glGetShaderInfoLog(GLuint, GLsizei, GLsizei*, GLchar*)",
     "void glGetShaderInfoLog(GLuint, GLsizei, compat_ptr<GLsizei>, compat_ptr<GLchar>)")

fn32("void glUniform4fv(GLint, GLsizei, const GLfloat*)",
     "void glUniform4fv(GLint, GLsizei, compat_ptr<GLfloat>)")

Generate()
