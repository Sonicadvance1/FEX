#!/usr/bin/python3
from ThunkHelpers import *

lib("libX11")

cb("int XIfEventCB(Display*, XEvent*, XPointer)")
cb("int XSetErrorHandlerCB(Display*, XErrorEvent*)")

fn32("Display* XOpenDisplay(const char*)")
no_unpack()

fn32("int XSetStandardProperties(Display*,Window,const char*,const char*,Pixmap,char**,int,XSizeHints*)");
no_unpack()

fn32("int XFree(void*)",
     "int XFree(compat_ptr<void>)")

fn32("XVisualInfo* XGetVisualInfo(Display*, long int, XVisualInfo*, int*)")
no_unpack()

fn32("int XDestroyWindow(Display*, Window)",
     "int XDestroyWindow(compat_ptr<FEX::Display_32>, FEX::Window_32)")
no_unpack()

fn32("int XSetNormalHints(Display*, Window, XSizeHints*)")
no_unpack()

fn32("Window XCreateWindow(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual*, long unsigned int, XSetWindowAttributes*)")
no_unpack()

fn32("Colormap XCreateColormap(Display*, Window, Visual*, int)")
no_unpack()

fn32("KeySym XLookupKeysym(XKeyEvent*, int)",
     "FEX::KeySym_32 XLookupKeysym(compat_ptr<FEX::XKeyEvent_32>, int)")
no_unpack()

fn32("int XCloseDisplay(Display*)",
     "int XCloseDisplay(compat_ptr<void /*Display*/>)")
no_unpack()

fn32("int XLookupString(XKeyEvent*, char*, int, KeySym*, XComposeStatus*)",
     "int XLookupString(compat_ptr<FEX::XKeyEvent_32>, compat_ptr<char>, int, compat_ptr<FEX::KeySym_32>, compat_ptr<XComposeStatus>)",)
no_unpack()

fn32("int XNextEvent(Display*, XEvent*)",
     "int XNextEvent(compat_ptr<FEX::Display_32>, compat_ptr<FEX::XEvent_32>)")
no_unpack()

fn32("int XMapWindow(Display*, Window)",
     "int XMapWindow(compat_ptr<FEX::Display_32>, FEX::Window_32)")
no_unpack()

fn32("int XPending(Display*)",
     "int XPending(compat_ptr<FEX::Display_32>)")
no_unpack()

Generate()
