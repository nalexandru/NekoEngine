#include <stdio.h>

#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/utsname.h>

#include <Input/Input.h>
#include <System/System.h>
#include <Engine/Engine.h>

#include "UNIXPlatform.h"

Display *X11_Display;
XVisualInfo X11_VisualInfo;
Atom X11_WM_PROTOCOLS, X11_WM_DELETE_WINDOW, X11_NET_WM_STATE, X11_NET_WM_PID,
	X11_NET_WM_WINDOW_TYPE, X11_NET_WM_WINDOW_TYPE_NORMAL, X11_NET_WM_BYPASS_COMPOSITOR;

static int _numCpus = 0;
static char _colors[4][8] =
{
	"\x1B[35m\0",
	"\x1B[0m\0",
	"\x1B[33m\0",
	"\x1B[31m\0"
};
static struct utsname _uname;

bool
Sys_InitDbgOut(void)
{
	return true;
}

void
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	fwprintf(stderr, L"%hs[%ls][%ls]: %ls\x1B[0m\n", _colors[color], module, severity, text);
}

void
Sys_TermDbgOut(void)
{
}

uint64_t
Sys_Time(void)
{
#ifdef CLOCK_MONOTONIC
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
 
    return (uint64_t)tp.tv_sec * (uint64_t)1000000000 + (uint64_t)tp.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
 
    return (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec;
#endif
}

bool
Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size)
{
	FILE *fp = NULL;
	int fd, p = 0, f = 0, mode = 0;

	fp = fopen(path, "r");
	if (!fp)
		return false;

	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
	fclose(fp);

	if (write) {
		p = PROT_READ | PROT_WRITE;
		mode = O_RDWR;
		f = MAP_PRIVATE;
	} else {
		p = PROT_READ;
		mode = O_RDONLY;
		f = MAP_SHARED;
	}

	fd = open(path, mode);
	if (!fd)
		return false;

	*ptr = mmap(0, *size, p, f, fd, 0);

	close(fd);

	return true;
}

void
Sys_UnmapFile(const void *ptr, uint64_t size)
{
	munmap((void *)ptr, size);
}

uint32_t
Sys_TlsAlloc(void)
{
	pthread_key_t key;
	pthread_key_create(&key, NULL);
	return (uint32_t)key;
}

void *
Sys_TlsGet(uint32_t key)
{
	return pthread_getspecific((pthread_key_t)key);
}

void
Sys_TlsSet(uint32_t key, void *data)
{
	pthread_setspecific((pthread_key_t)key, data);
}

void
Sys_TlsFree(uint32_t key)
{
	pthread_key_delete((pthread_key_t)key);
}

void
Sys_Yield(void)
{
	sched_yield();
}

const char *
Sys_Hostname(void)
{
	if (!_uname.sysname[0])
		uname(&_uname);

	return _uname.nodename;
}

const char *
Sys_Machine(void)
{
	if (!_uname.sysname[0])
		uname(&_uname);

	return _uname.machine;
}

const char *
Sys_CpuName(void)
{
	return "Unknown";
}

int
Sys_NumCpus(void)
{
	return _numCpus;
}

enum MachineType
Sys_MachineType(void)
{
	return MT_PC;
}

uint32_t
Sys_Capabilities(void)
{
	return SC_MMIO;
}

bool
Sys_ScreenVisible(void)
{
	return true;
}

bool
Sys_UniversalWindows(void)
{
	return false;
}

void
Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon)
{
/*	UINT type = MB_OK;

	switch (icon) {
		case MSG_ICON_NONE:
			type = 0;
			break;
		case MSG_ICON_INFO:
			type |= MB_ICONINFORMATION;
			break;
		case MSG_ICON_WARN:
			type |= MB_ICONWARNING;
			break;
		case MSG_ICON_ERROR:
			type |= MB_ICONERROR;
			break;
	}

	MessageBox((HWND)E_Screen, message, title, type);*/
}

bool
Sys_ProcessEvents(void)
{
	XEvent ev, nev;
	while (XPending(X11_Display)) {
		XNextEvent(X11_Display, &ev);
		
		switch (ev.type) {
		case KeyPress: {
			In_ButtonState[X11_Keymap[ev.xkey.keycode]] = true;
		} break;
		case KeyRelease: {
			if (XEventsQueued(X11_Display, QueuedAfterReading)) {
				XPeekEvent(X11_Display, &nev);
				
				if (nev.type == KeyPress &&
						nev.xkey.time == ev.xkey.time &&
						nev.xkey.keycode == ev.xkey.keycode)
					break;
			}
			
			In_ButtonState[X11_Keymap[ev.xkey.keycode]] = false;
		} break;
		case ButtonPress:
		case ButtonRelease: {
			switch (ev.xbutton.button) {
			case Button1: In_ButtonState[BTN_MOUSE_LMB] = ev.type == ButtonPress;
			case Button2: In_ButtonState[BTN_MOUSE_RMB] = ev.type == ButtonPress;
			case Button3: In_ButtonState[BTN_MOUSE_MMB] = ev.type == ButtonPress;
			case Button4: In_ButtonState[BTN_MOUSE_BTN4] = ev.type == ButtonPress;
			case Button5: In_ButtonState[BTN_MOUSE_BTN5] = ev.type == ButtonPress;
			}
		} break;
		case ConfigureNotify: {
			XConfigureEvent ce = ev.xconfigure;
			
			if (ce.width == *E_ScreenWidth && ce.height == *E_ScreenHeight)
				break;
			
			*E_ScreenWidth = ce.width;
			*E_ScreenHeight = ce.height;
			
			//ScreenResized
		} break;
		case ClientMessage: {
			if (ev.xclient.data.l[0] == X11_WM_DELETE_WINDOW)
				return false;
		} break;
		case DestroyNotify: {
			return false;
		} break;
		default: {
		} break;
		}
	}

	return true;
}

bool
Sys_InitPlatform(void)
{
#ifdef SYS_PLATFORM_IRIX
	_numCpus = sysconf(_SC_NPROC_ONLN);
#else
	#ifndef _SC_NPROCESSORS_ONLN
		_numCpus = sysconf(HW_NCPU);
	#else
		_numCpus = sysconf(_SC_NPROCESSORS_ONLN);
	#endif
#endif

	X11_Display = XOpenDisplay(NULL);
	if (!X11_Display)
		return false;
	
	int screen = XDefaultScreen(X11_Display);
	if (!XMatchVisualInfo(X11_Display, screen, 24, TrueColor, &X11_VisualInfo))
		return false;
	
	X11_NET_WM_STATE = XInternAtom(X11_Display, "_NET_WM_STATE", False);
	X11_NET_WM_PID = XInternAtom(X11_Display, "_NET_WM_PID", False);
	X11_NET_WM_WINDOW_TYPE = XInternAtom(X11_Display, "_NET_WM_WINDOW_TYPE", False);
	X11_NET_WM_WINDOW_TYPE_NORMAL = XInternAtom(X11_Display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	X11_NET_WM_BYPASS_COMPOSITOR = XInternAtom(X11_Display, "_NET_WM_BYPASS_COMPOSITOR", False);
	X11_WM_PROTOCOLS = XInternAtom(X11_Display, "WM_PROTOCOLS", False);
	X11_WM_DELETE_WINDOW = XInternAtom(X11_Display, "WM_DELETE_WINDOW", False);

	return true;
}

void
Sys_TermPlatform(void)
{
	XCloseDisplay(X11_Display);
}

