#pragma once
#include <X11/Xlib.h>
#include <stdint.h>

#include "common/Host.h"

namespace FEX {
using XID_32 = uint32_t;
using compat_ulong = uint32_t;
using compat_long = int32_t;
using XPointer_32 = compat_ptr<char>;
using VisualID_32 = uint32_t;
using Colormap_32 = XID_32;
using Window_32 = XID_32;
using Pixmap_32 = XID_32;
using Colormap_32 = XID_32;
using Cursor_32 = XID_32;
using Time_32 = compat_ulong;
using KeySym_32 = XID_32;

#define CPYT(x) val.x = x
#define CPYF(x) x = val->x

struct Visual_32 {
	compat_ptr<XExtData> ext_data;
	VisualID_32 visualid;
#if defined(__cplusplus) || defined(c_plusplus)
	int c_class;
#else
	int class;
#endif
	uint32_t red_mask, green_mask, blue_mask;
	int bits_per_rgb;
	int map_entries;

  Visual_32() = delete;

  Visual_32(Visual *val)
    : ext_data { val->ext_data } {
    CPYF(visualid);
#if defined(__cplusplus) || defined(c_plusplus)
    CPYF(c_class);
#else
    CPYF(class);
#endif
    CPYF(red_mask);
    CPYF(green_mask);
    CPYF(blue_mask);
    CPYF(bits_per_rgb);
    CPYF(map_entries);
  }

  operator Visual() const {
    Visual val{};
    CPYT(ext_data);
    CPYT(visualid);
#if defined(__cplusplus) || defined(c_plusplus)
    CPYT(c_class);
#else
    CPYT(class);
#endif
    CPYT(red_mask);
    CPYT(green_mask);
    CPYT(blue_mask);
    CPYT(bits_per_rgb);
    CPYT(map_entries);
    return val;
  }
};

struct
__attribute__((annotate("alias-x86_32-Screen")))
__attribute__((annotate("fex-match")))
Screen_32 {
	compat_ptr<XExtData> ext_data;
	compat_ptr<_XDisplay> display;
	Window_32 root;
	int width, height;
	int mwidth, mheight;
	int ndepths;
	compat_ptr<Depth> depths;
	int root_depth;
	compat_ptr<Visual> root_visual;
	compat_ptr<_XGC> default_gc;
	Colormap_32 cmap;
  compat_ulong white_pixel;
  compat_ulong black_pixel;
	int max_maps, min_maps;
	int backing_store;
	Bool save_unders;
	compat_long root_input_mask;

  Screen_32() = delete;

  Screen_32(Screen *val)
    : ext_data { val->ext_data }
    , display { val->display }
    , depths { val->depths }
    , root_visual { val->root_visual }
    , default_gc { val->default_gc } {
    CPYF(root);
    CPYF(width);
    CPYF(height);
    CPYF(mwidth);
    CPYF(mheight);
    CPYF(ndepths);
    CPYF(root_depth);
    CPYF(cmap);
    CPYF(white_pixel);
    CPYF(black_pixel);
    CPYF(max_maps);
    CPYF(min_maps);
    CPYF(backing_store);
    CPYF(save_unders);
    CPYF(root_input_mask);
  }

  operator Screen() const {
    Screen val{};
    CPYT(ext_data);
    CPYT(display);
    CPYT(root);
    CPYT(width);
    CPYT(height);
    CPYT(mwidth);
    CPYT(mheight);
    CPYT(ndepths);
    CPYT(depths);
    CPYT(root_depth);
    CPYT(root_visual);
    CPYT(default_gc);
    CPYT(cmap);
    CPYT(white_pixel);
    CPYT(black_pixel);
    CPYT(max_maps);
    CPYT(min_maps);
    CPYT(backing_store);
    CPYT(save_unders);
    CPYT(root_input_mask);
    return val;
  }
};

struct
__attribute__((annotate("alias-x86_32-_XDisplay")))
__attribute__((annotate("fex-match")))
Display_32 {
  compat_ptr<XExtData> ext_data;
	compat_ptr<_XPrivate> private1;
	int fd;
	int private2;
	int proto_major_version;
	int proto_minor_version;
	compat_ptr<char> vendor;
  XID_32 private3;
	XID_32 private4;
	XID_32 private5;
	int private6;
  compat_ptr<void> resource_alloc;
	//XID_32 (*resource_alloc)(	/* allocator function */
	//	struct _XDisplay*
	//);
	int byte_order;
	int bitmap_unit;
	int bitmap_pad;
	int bitmap_bit_order;
	int nformats;
	compat_ptr<ScreenFormat> pixmap_format;
	int private8;
	int release;
	compat_ptr<_XPrivate> private9;
  compat_ptr<_XPrivate> private10;
	int qlen;
	compat_ulong last_request_read;
	compat_ulong request;
	XPointer_32 private11;
	XPointer_32 private12;
	XPointer_32 private13;
	XPointer_32 private14;
	unsigned max_request_size;
	compat_ptr<_XrmHashBucketRec> db;
  compat_ptr<void> private15;
	//int (*private15)(
	//	struct _XDisplay*
	//	);
	compat_ptr<char> display_name;
	int default_screen;
	int nscreens;
	compat_ptr<Screen_32> screens;
	compat_ulong motion_buffer;
  compat_ulong private16;
	int min_keycode;
	int max_keycode;
	XPointer_32 private17;
	XPointer_32 private18;
	int private19;
	compat_ptr<char> xdefaults;

  Display_32() = delete;

  Display_32(Display *Disp, Screen_32 *Screens)
    : ext_data { ((_XPrivDisplay)Disp)->ext_data }
    , private1 { ((_XPrivDisplay)Disp)->private1 }
    , vendor { ((_XPrivDisplay)Disp)->vendor }
    , resource_alloc { (void*) ((_XPrivDisplay)Disp)->resource_alloc }
    , pixmap_format { ((_XPrivDisplay)Disp)->pixmap_format }
    , private9 { ((_XPrivDisplay)Disp)->private9 }
    , private10 { ((_XPrivDisplay)Disp)->private10 }
    , private11 { ((_XPrivDisplay)Disp)->private11 }
    , private12 { ((_XPrivDisplay)Disp)->private12 }
    , private13 { ((_XPrivDisplay)Disp)->private13 }
    , private14 { ((_XPrivDisplay)Disp)->private14 }
    , db { ((_XPrivDisplay)Disp)->db }
    , private15 { (void*) ((_XPrivDisplay)Disp)->private15 }
    , display_name { ((_XPrivDisplay)Disp)->display_name }
    , screens { Screens }
    , private17 { ((_XPrivDisplay)Disp)->private17 }
    , private18 { ((_XPrivDisplay)Disp)->private18 }
    , xdefaults { ((_XPrivDisplay)Disp)->xdefaults } {
    _XPrivDisplay val = (_XPrivDisplay)Disp;
    CPYF(fd);
    CPYF(private2);
    CPYF(proto_major_version);
    CPYF(proto_minor_version);
    CPYF(private3);
    CPYF(private4);
    CPYF(private5);
    CPYF(private6);
    CPYF(byte_order);
    CPYF(bitmap_unit);
    CPYF(bitmap_pad);
    CPYF(bitmap_bit_order);
    CPYF(nformats);
    CPYF(private8);
    CPYF(release);
    CPYF(qlen);
    CPYF(last_request_read);
    CPYF(request);
    CPYF(max_request_size);
    CPYF(default_screen);
    CPYF(nscreens);
    CPYF(motion_buffer);
    CPYF(private16);
    CPYF(min_keycode);
    CPYF(max_keycode);
    CPYF(private19);
  }
};

struct XVisualInfo_32 {
  compat_ptr<FEX::Visual_32> visual;
  VisualID_32 visualid;
  int screen;
  int depth;
#if defined(__cplusplus) || defined(c_plusplus)
  int c_class;					/* C++ */
#else
  int class;
#endif
  uint32_t red_mask;
  uint32_t green_mask;
  uint32_t blue_mask;
  int colormap_size;
  int bits_per_rgb;

  XVisualInfo_32() = delete;

  XVisualInfo_32(XVisualInfo *val, Visual_32 *vis)
    : visual { vis } {
    CPYF(visualid);
    CPYF(screen);
#if defined(__cplusplus) || defined(c_plusplus)
    CPYF(c_class);
#else
    CPYF(class);
#endif
    CPYF(depth);
    CPYF(red_mask);
    CPYF(green_mask);
    CPYF(blue_mask);
    CPYF(colormap_size);
    CPYF(bits_per_rgb);
  }

  operator XVisualInfo() const {
    XVisualInfo val;
    CPYT(visual);
    CPYT(visualid);
    CPYT(screen);
#if defined(__cplusplus) || defined(c_plusplus)
    CPYT(c_class);
#else
    CPYT(class);
#endif
    CPYT(depth);
    CPYT(red_mask);
    CPYT(green_mask);
    CPYT(blue_mask);
    CPYT(colormap_size);
    CPYT(bits_per_rgb);

    return val;
  }
};

struct XSizeHints_32 {
  int32_t flags;
	int x, y;
	int width, height;
	int min_width, min_height;
	int max_width, max_height;
  int width_inc, height_inc;
	struct {
		int x;
		int y;
	} min_aspect, max_aspect;
	int base_width, base_height;
	int win_gravity;

  XSizeHints_32() = delete;
  XSizeHints_32(XSizeHints *val) {
    CPYF(flags);
    CPYF(x);
    CPYF(y);
    CPYF(width);
    CPYF(height);
    CPYF(min_width);
    CPYF(min_height);
    CPYF(max_width);
    CPYF(max_height);
    CPYF(width_inc);
    CPYF(height_inc);
    CPYF(min_aspect.x);
    CPYF(min_aspect.y);
    CPYF(max_aspect.x);
    CPYF(max_aspect.y);
    CPYF(base_width);
    CPYF(base_height);
    CPYF(win_gravity);
  }

  operator XSizeHints() const {
    XSizeHints val{};
    CPYT(flags);
    CPYT(x);
    CPYT(y);
    CPYT(width);
    CPYT(height);
    CPYT(min_width);
    CPYT(min_height);
    CPYT(max_width);
    CPYT(max_height);
    CPYT(width_inc);
    CPYT(height_inc);
    CPYT(min_aspect.x);
    CPYT(min_aspect.y);
    CPYT(max_aspect.x);
    CPYT(max_aspect.y);
    CPYT(base_width);
    CPYT(base_height);
    CPYT(win_gravity);
    return val;
  }
};

struct XSetWindowAttributes_32 {
    Pixmap_32 background_pixmap;
    compat_ulong background_pixel;
    Pixmap_32 border_pixmap;
    compat_ulong border_pixel;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    compat_ulong backing_planes;
    compat_ulong backing_pixel;
    Bool save_under;
    compat_long event_mask;
    compat_long do_not_propagate_mask;
    Bool override_redirect;
    Colormap_32 colormap;
    Cursor_32 cursor;

    XSetWindowAttributes_32() = delete;

    XSetWindowAttributes_32(XSetWindowAttributes *val) {
      CPYF(background_pixmap);
      CPYF(background_pixel);
      CPYF(border_pixmap);
      CPYF(border_pixel);
      CPYF(bit_gravity);
      CPYF(win_gravity);
      CPYF(backing_store);
      CPYF(backing_planes);
      CPYF(backing_pixel);
      CPYF(save_under);
      CPYF(event_mask);
      CPYF(do_not_propagate_mask);
      CPYF(override_redirect);
      CPYF(colormap);
      CPYF(cursor);
    }

    operator XSetWindowAttributes() const {
      XSetWindowAttributes val{};

      CPYT(background_pixmap);
      CPYT(background_pixel);
      CPYT(border_pixmap);
      CPYT(border_pixel);
      CPYT(bit_gravity);
      CPYT(win_gravity);
      CPYT(backing_store);
      CPYT(backing_planes);
      CPYT(backing_pixel);
      CPYT(save_under);
      CPYT(event_mask);
      CPYT(do_not_propagate_mask);
      CPYT(override_redirect);
      CPYT(colormap);
      CPYT(cursor);
      return val;
    }
};

struct XAnyEvent_32 {
	int type;
	compat_ulong serial;
	Bool send_event;
	compat_ptr<FEX::Display_32> display;
  FEX::Window_32 window;
};

struct XKeyEvent_32 {
	int type;
	compat_ulong serial;
	Bool send_event;
	compat_ptr<FEX::Display_32> display;
  FEX::Window_32 window;
  FEX::Window_32 root;
	FEX::Window_32 subwindow;
  FEX::Time_32 time;
	int x, y;
	int x_root, y_root;
	unsigned int state;
	unsigned int keycode;
	Bool same_screen;
};

struct XExposeEvent_32 {
	int type;
	compat_ulong serial;
	Bool send_event;
	compat_ptr<FEX::Display_32> display;
  FEX::Window_32 window;
	int x, y;
	int width, height;
	int count;
};

struct XConfigureEvent_32 {
	int type;
	compat_ulong serial;
	Bool send_event;
	compat_ptr<FEX::Display_32> display;
  FEX::Window_32 event;
	FEX::Window_32 window;
	int x, y;
	int width, height;
	int border_width;
  FEX::Window_32 above;
	Bool override_redirect;
};

union XEvent_32 {
  int type;
	XAnyEvent_32 xany;
	XKeyEvent_32 xkey;
	//XButtonEvent xbutton;
	//XMotionEvent xmotion;
	//XCrossingEvent xcrossing;
	//XFocusChangeEvent xfocus;
	XExposeEvent_32 xexpose;
	//XGraphicsExposeEvent xgraphicsexpose;
	//XNoExposeEvent xnoexpose;
	//XVisibilityEvent xvisibility;
	//XCreateWindowEvent xcreatewindow;
	//XDestroyWindowEvent xdestroywindow;
	//XUnmapEvent xunmap;
	//XMapEvent xmap;
	//XMapRequestEvent xmaprequest;
	//XReparentEvent xreparent;
	XConfigureEvent_32 xconfigure;
	//XGravityEvent xgravity;
	//XResizeRequestEvent xresizerequest;
	//XConfigureRequestEvent xconfigurerequest;
	//XCirculateEvent xcirculate;
	//XCirculateRequestEvent xcirculaterequest;
	//XPropertyEvent xproperty;
	//XSelectionClearEvent xselectionclear;
	//XSelectionRequestEvent xselectionrequest;
	//XSelectionEvent xselection;
	//XColormapEvent xcolormap;
	//XClientMessageEvent xclient;
	//XMappingEvent xmapping;
	//XErrorEvent xerror;
	//XKeymapEvent xkeymap;
	//XGenericEvent xgeneric;
	//XGenericEventCookie xcookie;
	compat_long pad[24];

  XEvent_32() = delete;

  XEvent_32(XAnyEvent *key, FEX::Display_32 *Disp) {
    XEvent *val = (XEvent*)key;
    // Header first
    CPYF(xany.type);
    CPYF(xany.serial);
    CPYF(xany.send_event);
    xany.display = Disp;
  }

  operator XAnyEvent() const {
    XEvent val{};
    // Header first
    CPYT(xany.type);
    CPYT(xany.serial);
    CPYT(xany.send_event);
    CPYT(xany.display);
    return val.xany;
  }

  XEvent_32(XKeyEvent *key, FEX::Display_32 *Disp) {
    XEvent *val = (XEvent*)key;
    // Header first
    CPYF(xany.type);
    CPYF(xany.serial);
    CPYF(xany.send_event);
    xany.display = Disp;

    // XKeyEvent things now
    CPYF(xkey.window);
    CPYF(xkey.root);
    CPYF(xkey.subwindow);
    CPYF(xkey.time);
    CPYF(xkey.x);
    CPYF(xkey.y);
    CPYF(xkey.x_root);
    CPYF(xkey.y_root);
    CPYF(xkey.state);
    CPYF(xkey.keycode);
    CPYF(xkey.same_screen);
  }

  operator XKeyEvent() const {
    XEvent val{};
    // Header first
    CPYT(xany.type);
    CPYT(xany.serial);
    CPYT(xany.send_event);
    CPYT(xany.display);

    // XKeyEvent things now
    CPYT(xkey.window);
    CPYT(xkey.root);
    CPYT(xkey.subwindow);
    CPYT(xkey.time);
    CPYT(xkey.x);
    CPYT(xkey.y);
    CPYT(xkey.x_root);
    CPYT(xkey.y_root);
    CPYT(xkey.state);
    CPYT(xkey.keycode);
    CPYT(xkey.same_screen);

    return val.xkey;
  }

  XEvent_32(XExposeEvent *key, FEX::Display_32 *Disp) {
    XEvent *val = (XEvent*)key;
    // Header first
    CPYF(xany.type);
    CPYF(xany.serial);
    CPYF(xany.send_event);
    xany.display = Disp;

    // XExposeEvent
    CPYF(xexpose.x);
    CPYF(xexpose.y);
    CPYF(xexpose.width);
    CPYF(xexpose.height);
    CPYF(xexpose.count);
  }

  operator XExposeEvent() const {
    XEvent val{};
    // Header first
    CPYT(xany.type);
    CPYT(xany.serial);
    CPYT(xany.send_event);
    CPYT(xany.display);

    // XExposeEvent
    CPYT(xexpose.x);
    CPYT(xexpose.y);
    CPYT(xexpose.width);
    CPYT(xexpose.height);
    CPYT(xexpose.count);

    return val.xexpose;
  }

  XEvent_32(XConfigureEvent *key, FEX::Display_32 *Disp) {
    XEvent *val = (XEvent*)key;
    // Header first
    CPYF(xany.type);
    CPYF(xany.serial);
    CPYF(xany.send_event);
    xany.display = Disp;

    CPYF(xconfigure.event);
    CPYF(xconfigure.window);
    CPYF(xconfigure.x);
    CPYF(xconfigure.y);
    CPYF(xconfigure.width);
    CPYF(xconfigure.height);
    CPYF(xconfigure.border_width);
    CPYF(xconfigure.above);
    CPYF(xconfigure.override_redirect);
  }

  operator XConfigureEvent() const {
    XEvent val{};
    // Header first
    CPYT(xany.type);
    CPYT(xany.serial);
    CPYT(xany.send_event);
    CPYT(xany.display);

    CPYT(xconfigure.event);
    CPYT(xconfigure.window);
    CPYT(xconfigure.x);
    CPYT(xconfigure.y);
    CPYT(xconfigure.width);
    CPYT(xconfigure.height);
    CPYT(xconfigure.border_width);
    CPYT(xconfigure.above);
    CPYT(xconfigure.override_redirect);

    return val.xconfigure;
  }
};

}
