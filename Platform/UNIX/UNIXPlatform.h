#ifndef _UNIX_PLATFORM_H_
#define _UNIX_PLATFORM_H_

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <Input/Input.h>

extern Display *X11_Display;
extern XVisualInfo X11_VisualInfo;
extern Atom X11_WM_PROTOCOLS, X11_WM_DELETE_WINDOW, X11_NET_WM_STATE, X11_NET_WM_PID,
	X11_NET_WM_WINDOW_TYPE, X11_NET_WM_WINDOW_TYPE_NORMAL, X11_NET_WM_BYPASS_COMPOSITOR;
extern enum Button X11_Keymap[];

#endif /* _UNIX_PLATFORM_H_ */