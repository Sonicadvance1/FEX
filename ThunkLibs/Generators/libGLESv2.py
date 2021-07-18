#!/usr/bin/python3
from ThunkHelpers import *

lib("libGLESv2")
fn("const GLubyte* glGetString(GLenum)")

Generate()
