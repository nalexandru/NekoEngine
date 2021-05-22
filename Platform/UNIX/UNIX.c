#include <stdio.h>

#include <sched.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <Input/Input.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Application.h>

#include "UNIXPlatform.h"

#if (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)
#define	USE_POSIX_MEMALIGN
#elif (defined(__GLIBC__) && ((__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 8) || __GLIBC__ > 2) && defined(__LP64__)) \
		|| (defined(__FreeBSD__) && !defined(__arm__) && !defined(__mips__)) \
		|| defined(__APPLE__)
#define	USE_MALLOC
#elif __STDC_VERSION__ >= 201112L
#define USE_ALIGNED_ALLOC
#elif __SSE__
#include <intrin.h>
#define USE_MM_MALLLOC
#else
#define USE_MEMALIGN
#endif

Display *X11_display;
XVisualInfo X11_visualInfo;
Atom X11_WM_PROTOCOLS, X11_WM_DELETE_WINDOW, X11_NET_WM_STATE, X11_NET_WM_PID,
	X11_NET_WM_WINDOW_TYPE, X11_NET_WM_WINDOW_TYPE_NORMAL, X11_NET_WM_BYPASS_COMPOSITOR;

static uint32_t _cpuCount = 0, _cpuFreq = 0, _cpuThreadCount = 0;
static char _cpuName[128] = "Unknown";
static char _colors[4][8] =
{
	"\x1B[35m\0",
	"\x1B[0m\0",
	"\x1B[33m\0",
	"\x1B[31m\0"
};
static struct utsname _uname;

static inline void _CpuInfo(void);

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
	return _uname.nodename;
}

const char *
Sys_Machine(void)
{
	return _uname.machine;
}

const char *
Sys_CpuName(void)
{
	return _cpuName;
}

uint32_t
Sys_CpuFreq(void)
{
	return _cpuFreq;
}

uint32_t
Sys_CpuCount(void)
{
	return _cpuCount;
}

uint32_t
Sys_CpuThreadCount(void)
{
	return _cpuThreadCount;
}

const char *
Sys_OperatingSystem(void)
{
	return _uname.sysname;
}

const char *
Sys_OperatingSystemVersion(void)
{
	return _uname.release;
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

	MessageBox((HWND)E_screen, message, title, type);*/
}

bool
Sys_ProcessEvents(void)
{
	XEvent ev, nev;
	while (XPending(X11_display)) {
		XNextEvent(X11_display, &ev);
		
		switch (ev.type) {
		case KeyPress: {
			In_buttonState[X11_keymap[ev.xkey.keycode]] = true;
		} break;
		case KeyRelease: {
			if (XEventsQueued(X11_display, QueuedAfterReading)) {
				XPeekEvent(X11_display, &nev);
				
				if (nev.type == KeyPress &&
						nev.xkey.time == ev.xkey.time &&
						nev.xkey.keycode == ev.xkey.keycode)
					break;
			}
			
			In_buttonState[X11_keymap[ev.xkey.keycode]] = false;
		} break;
		case ButtonPress:
		case ButtonRelease: {
			switch (ev.xbutton.button) {
			case Button1: In_buttonState[BTN_MOUSE_LMB] = ev.type == ButtonPress;
			case Button2: In_buttonState[BTN_MOUSE_RMB] = ev.type == ButtonPress;
			case Button3: In_buttonState[BTN_MOUSE_MMB] = ev.type == ButtonPress;
			case Button4: In_buttonState[BTN_MOUSE_BTN4] = ev.type == ButtonPress;
			case Button5: In_buttonState[BTN_MOUSE_BTN5] = ev.type == ButtonPress;
			}
		} break;
		case ConfigureNotify: {
			XConfigureEvent ce = ev.xconfigure;
			
			if (ce.width == *E_screenWidth && ce.height == *E_screenHeight)
				break;

			E_ScreenResized(ce.width, ce.height);
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

void *
Sys_LoadLibrary(const char *name)
{
	char *path = NULL;
	
	if (!name)
		return dlopen(NULL, RTLD_NOW);
	
	if (access(name, R_OK) < 0) {
		char *prefix = "", *suffix = "";
		size_t len = strlen(name);
		
		path = Sys_Alloc(sizeof(char), 2048, MH_Transient);
		if (!path)
			return NULL;
		
		if (!strchr(name, '/') && strncmp(name, "lib", 3))
			prefix = "lib";
	
		if (len < 4 || strncmp(name + len - 3, ".so", 3))
			suffix = ".so";
		
		snprintf(path, 2048, "%s%s%s", prefix, name, suffix);
	} else {
		path = (char *)name;
	}
	
	return dlopen(path, RTLD_NOW);
}

void *
Sys_GetProcAddress(void *lib, const char *name)
{
	if (!lib || !name)
		return NULL;
	return dlsym(lib, name);
}

void
Sys_UnloadLibrary(void *lib)
{
	if (lib)
		dlclose(lib);
}

void
Sys_Sleep(uint32_t sec)
{
	sleep(sec);
}

void
Sys_MSleep(uint32_t msec)
{
	usleep(msec * 1000);
}

void
Sys_USleep(uint32_t usec)
{
	usleep(usec);
}

void
Sys_DirectoryPath(enum SystemDirectory sd, char *out, size_t len)
{
	wchar_t *path = Sys_Alloc(sizeof(wchar_t *), len, MH_Transient);
	
	switch (sd) {
	case SD_SAVE_GAME: swprintf(path, len, L"%hs/.config/%ls/saves", getenv("HOME"), App_applicationInfo.name); break; 
	case SD_APP_DATA: swprintf(path, len, L"%hs/.config/%ls/data", getenv("HOME"), App_applicationInfo.name); break; 
	case SD_TEMP: swprintf(path, len, L"/tmp/"); break;
	}
	
	wcstombs(out, path, len);
}

bool
Sys_FileExists ( const char* path )
{
	struct stat st;
	return !stat(path, &st);
}

bool
Sys_DirectoryExists(const char *path)
{
	struct stat st;
	return !stat(path, &st) && S_ISDIR(st.st_mode);
}

bool
Sys_CreateDirectory(const char *path)
{
	if (!mkdir(path, 0700))
		return true;
		
	if (errno != ENOENT)
		return false;
		
	char *dir = Sys_Alloc(sizeof(*dir), 4096, MH_Transient);
	memcpy(dir, path, strlen(path));
	
	for (char *p = dir + 1; *p; ++p) {
		if (*p != '/')
			continue;
			
		*p = 0x0;
		
		if (mkdir(dir, 0700) && errno != EEXIST)
			return false;
			
		*p = '/';
	}
	
	if (mkdir(path, 0700))
		return errno == EEXIST;
	else
		return true;
}

void *
Sys_AlignedAlloc(size_t size, size_t alignment)
{
#if defined(USE_POSIX_MEMALIGN)
	void *mem;
	if (posix_memalign(&mem, alignment, size))
		return NULL;
	return mem;
#elif defined(USE_MEMALIGN)
	return memalign(alignment, size);
#elif defined(USE_MALLOC)
	return malloc(size);
#elif defined(USE_ALIGNED_ALLOC)
	return aligned_alloc(alignment, size);
#elif defined(USE_MM_MALLOC)
	return _mm_malloc(size, alignment);
#else
#error	Aligned memory allocation not implemented for this platform
#endif
}

void
Sys_AlignedFree(void *mem)
{
#if defined(USE_MALLOC) || defined(USE_ALIGNED_ALLOC) || defined(USE_MEMALIGN) || defined(USE_POSIX_MEMALIGN)
	return free(mem);
#elif defined(USE_MM_MALLOC)
	return _mm_free(mem);
#else
#error	Aligned memory allocation not implemented for this platform
#endif
}

void
Sys_ZeroMemory(void *mem, size_t len)
{
	memset(mem, 0x0, len);
}

bool
Sys_InitPlatform(void)
{
	uname(&_uname);

	XInitThreads();

	X11_display = XOpenDisplay(NULL);
	if (!X11_display)
		return false;
	
	XLockDisplay(X11_display);

	int screen = XDefaultScreen(X11_display);
	if (!XMatchVisualInfo(X11_display, screen, 24, TrueColor, &X11_visualInfo))
		return false;
	
	X11_NET_WM_STATE = XInternAtom(X11_display, "_NET_WM_STATE", False);
	X11_NET_WM_PID = XInternAtom(X11_display, "_NET_WM_PID", False);
	X11_NET_WM_WINDOW_TYPE = XInternAtom(X11_display, "_NET_WM_WINDOW_TYPE", False);
	X11_NET_WM_WINDOW_TYPE_NORMAL = XInternAtom(X11_display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	X11_NET_WM_BYPASS_COMPOSITOR = XInternAtom(X11_display, "_NET_WM_BYPASS_COMPOSITOR", False);
	X11_WM_PROTOCOLS = XInternAtom(X11_display, "WM_PROTOCOLS", False);
	X11_WM_DELETE_WINDOW = XInternAtom(X11_display, "WM_DELETE_WINDOW", False);

	XUnlockDisplay(X11_display);

	_CpuInfo();

	return true;
}

void
Sys_TermPlatform(void)
{
	//XCloseDisplay(X11_display);
}

void
_CpuInfo(void)
{
#if defined(__i386__) || defined(__amd64__) || defined(__x86_64__)
	uint32_t a = 11, b = 0, c = 1, d = 0;
	asm volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(a), "2"(c) : );

	_cpuCount = a;
	_cpuThreadCount = b;
#	define _HAVE_CPU_COUNT
#endif

#if defined(__linux__)
	char buff[512];
	char *cpuNameId = NULL, *cpuFreqId = NULL, *cpuCoreId = NULL;
	FILE *fp = fopen("/proc/cpuinfo", "r");
 
#	if defined(__i386__) || defined(__amd64__) || defined(__x86_64__)
		cpuNameId = "model name";
		cpuFreqId = "cpu MHz";
		cpuCoreId = "cpu cores";
#	elif defined(SYS_ARCH_PPC) || defined(SYS_ARCH_PPC64)
		cpuNameId = "cpu";
		cpuFreqId = "clock";
#	elif defined(SYS_ARCH_ARM) || defined(SYS_ARCH_ARM64)
		cpuNameId = "model name";
		cpuFreqId = "clock";
#	elif defined(SYS_ARCH_MIPS) || defined(SYS_ARCH_MIPS64)
		cpuNameId = "cpu model";
		cpuFreqId = "clock";
#	elif defined(SYS_ARCH_SPARC) || defined(SYS_ARCH_SPARC64)
		cpuNameId = "cpu";
		cpuFreqId = "clock";
#	elif defined(SYS_ARCH_HPPA)
		cpuNameId = "cpu";
		cpuFreqId = "cpu MHz";
#	elif defined(SYS_ARCH_ALPHA)
		cpuNameId = "cpu model";
		cpuFreqId = "cycle frequency [Hz]";
//		cpuFreqDiv = 1000;
#	else
#		warning "Cpu info not implemented for this architecture (Platform/UNIX/UNIX.c)"
#	endif

	if (!cpuNameId || !cpuFreqId)
		return;
 
	_cpuFreq = 0;
	memset(buff, 0x0, 512);
 
	if (fp) {
		while (fgets(buff, 512, fp)) {
			if (strstr(buff, cpuNameId)) {
				char *ptr = strchr(buff, ':');
				snprintf(_cpuName, sizeof(_cpuName), "%s", ptr ? ptr + 2 : "Unknown");
			} else if (strstr(buff, cpuFreqId)) {
				char *ptr = strchr(buff, ':');
				if (ptr) {
					ptr += 2;
					ptr[strlen(ptr) - 1] = 0x0;
					_cpuFreq = atoi(ptr);
				}
			} else if (strstr(buff, cpuCoreId)) {
				char *ptr = strchr(buff, ':');
				if (ptr) {
					ptr += 2;
					ptr[strlen(ptr) - 1] = 0x0;
					_cpuCount = atoi(ptr);
					_cpuThreadCount = sysconf(_SC_NPROCESSORS_ONLN);
				}
				break;
			}
			memset(buff, 0x0, 512);
		}
		fclose(fp);
	}
#else
#	warning "Cpu info not implemented for this platform (Platform/UNIX/UNIX.c)"
#endif

	if (_cpuName[strlen(_cpuName) - 1] == '\n')
		_cpuName[strlen(_cpuName) - 1] = 0x0;

#ifndef _HAVE_CPU_COUNT
#	ifdef SYS_PLATFORM_IRIX
		_cpuCount = sysconf(_SC_NPROC_ONLN);
#	else
	#	ifndef _SC_NPROCESSORS_ONLN
			_cpuCount = sysconf(HW_NCPU);
	#	else
			_cpuCount = sysconf(_SC_NPROCESSORS_ONLN);
#		endif
#	endif
	_cpuThreadCount = _cpuCount;
#endif
}
