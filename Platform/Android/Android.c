#include <stdio.h>

#include <android/log.h>

#include <sched.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netinet/in.h>

#include <System/Log.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Network/Network.h>
#include <Engine/Engine.h>
#include <Engine/Application.h>

#include "AndroidPlatform.h"

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

#include <sys/inotify.h>
#include <linux/limits.h>

static uint32_t f_cpuCount = 0, f_cpuFreq = 0, f_cpuThreadCount = 0;
static char f_cpuName[128] = "Unknown";
static char f_colors[4][8] =
{
	"\x1B[35m\0",
	"\x1B[0m\0",
	"\x1B[33m\0",
	"\x1B[31m\0"
};
static struct utsname f_uname;

static inline void CpuInfo(void);

static void HandleCmd(struct android_app *app, int32_t cmd)
{
	switch (cmd) {
	case APP_CMD_START:
		E_screen = app->window;
		Android_assetManager = app->activity->assetManager;
	break;
	case APP_CMD_INIT_WINDOW:
		E_screen = app->window;
		E_ScreenResized(0, 0);
	break;
	case APP_CMD_TERM_WINDOW:
	break;
	case APP_CMD_DESTROY:
		E.Shutdown();
	break;
	}
}

int
Sys_Main(int argc, char *argv[])
{
	Android_app->onAppCmd = HandleCmd;

	if (!E_Init(argc, argv))
		return -1;

	return E_Run();
}

bool
Sys_InitDbgOut(void)
{
	return true;
}

void
Sys_DbgOut(int color, const char *module, const char *severity, const char *text)
{
	fprintf(stderr, "%s[%s][%s]: %s\x1B[0m\n", f_colors[color], module, severity, text);
}

void
Sys_TermDbgOut(void)
{
}

uint64_t
Sys_Time(void)
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	return (uint64_t)tp.tv_sec * (uint64_t)1000000000 + (uint64_t)tp.tv_nsec;
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
	return f_uname.nodename;
}

const char *
Sys_Machine(void)
{
	return f_uname.machine;
}

const char *
Sys_CpuName(void)
{
	return f_cpuName;
}

uint32_t
Sys_CpuFreq(void)
{
	return f_cpuFreq;
}

uint32_t
Sys_CpuCount(void)
{
	return f_cpuCount;
}

uint32_t
Sys_CpuThreadCount(void)
{
	return f_cpuThreadCount;
}

uint64_t
Sys_TotalMemory(void)
{
#if defined(_SC_PHYS_PAGES)
	uint64_t pages = sysconf(_SC_PHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#elif defined(HW_PHYSMEM)
	uint64_t mem = 0;
	size_t len = sizeof(mem);
#if defined(SYS_PLATFORM_APPLE) && defined(HW_MEMSIZE)
	static int mib[2] = { CTL_HW, HW_MEMSIZE };
#else
	static int mib[2] = { CTL_HW, HW_PHYSMEM };
#endif

	if (sysctl(mib, 2, &mem, &len, NULL, 0) < 0)
		return 0;

	return mem;
#elif defined(SYS_PLATFORM_IRIX)
	struct rminfo rmi;

	sysmp(MP_KERNADDR, MPSA_RMINFO, &rmi);

	return rmi.physmem;
#else
#	error FATAL: sys_mem_total NOT IMPLEMENTED FOR THIS PLATFORM
	return 0;
#endif
}

uint64_t
Sys_FreeMemory(void)
{
#if defined(_SC_AVPHYS_PAGES)
	uint64_t pages = sysconf(_SC_AVPHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#elif defined(HW_USERMEM)
	uint64_t mem = 0;
	size_t len = sizeof(mem);
	int mib[2] = { CTL_HW, HW_USERMEM };

	if (sysctl(mib, 2, &mem, &len, NULL, 0) < 0)
		return 0;

	return mem;
#elif defined(SYS_PLATFORM_IRIX)
	struct rminfo rmi;

	sysmp(MP_KERNADDR, MPSA_RMINFO, &rmi);

	return rmi.freemem;
#else
#	error FATAL: sys_mem_free NOT IMPLEMENTED FOR THIS PLATFORM
	return 0;
#endif
}

const char *
Sys_OperatingSystem(void)
{
	return f_uname.sysname;
}

const char *
Sys_OperatingSystemVersionString(void)
{
	return f_uname.release;
}

struct NeSysVersion
Sys_OperatingSystemVersion(void)
{
	struct NeSysVersion sv;
	sscanf(f_uname.release, "%d.%d.%d", &sv.major, &sv.minor, &sv.revision);
	return sv;
}

enum NeMachineType
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
Sys_MessageBox(const char *title, const char *message, int icon)
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
	int ident;
	int events;
	struct android_poll_source *source;

	while ((ident = ALooper_pollAll(0, NULL, &events, &source)) >= 0) {
		if (source)
			source->process(Android_app, source);

		if (Android_app->destroyRequested)
			return false;

		if (ident == LOOPER_ID_USER) {

		}

		/*auto inputBuf = android_app_swap_input_buffers(app);
		if (inputBuf == nullptr) {
			return;
		}

		// For the minimum, apps need to process the exit event (for example,
		// listening to AKEYCODE_BACK). This sample has done that in the Kotlin side
		// and not processing other input events, we just reset the event counter
		// inside the android_input_buffer to keep app glue code in a working state.
		android_app_clear_motion_events(inputBuf);
		android_app_clear_motion_events(inputBuf);*/
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

	void *mod = dlopen(path, RTLD_GLOBAL | RTLD_NOW);
	if (!mod)
		Sys_LogEntry(ANDROID_MOD, LOG_CRITICAL, "Failed to load library %s: %s", path, dlerror());
	return mod;
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
Sys_DirectoryPath(enum NeSystemDirectory sd, char *out, size_t len)
{	
	switch (sd) {
	case SD_SAVE_GAME: snprintf(out, len, "%s/.config/%s/saves", getenv("HOME"), App_applicationInfo.name); break; 
	case SD_APP_DATA: snprintf(out, len, "%s/.config/%s/data", getenv("HOME"), App_applicationInfo.name); break; 
	case SD_TEMP: snprintf(out, len, "/tmp/"); break;
	}
}

bool
Sys_FileExists(const char* path)
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

void
Sys_ExecutableLocation(char *buff, size_t len)
{
	char tmp[4096];
	snprintf(tmp, sizeof(tmp), "/proc/%d/exe", getpid());
	(void)readlink(tmp, buff, len);

	char *p = strrchr(buff, '/');
	if (p) *p++ = 0x0;
}

void
Sys_GetWorkingDirectory(char *buff, size_t len)
{
	getcwd(buff, len);
}

void
Sys_SetWorkingDirectory(const char *dir)
{
	chdir(dir);
}

void
Sys_UserName(char *buff, size_t len)
{
	getlogin_r(buff, len);
}

void
Sys_ZeroMemory(void *mem, size_t len)
{
	memset(mem, 0x0, len);
}

bool
Sys_InitPlatform(void)
{
	uname(&f_uname);
	CpuInfo();

	/*XInitThreads();

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
	X11_NET_WORKAREA = XInternAtom(X11_display, "_NET_WORKAREA", False);

	XUnlockDisplay(X11_display);*/

	return true;
}

void
Sys_TermPlatform(void)
{
}

intptr_t
Sys_GetCurrentProcess()
{
	return (intptr_t)getpid();
}

int32_t
Sys_GetCurrentProcessId()
{
	return getpid();
}

void
Sys_WaitForProcessExit(intptr_t handle)
{
	waitpid((pid_t)handle, NULL, 0);
}

intptr_t
Sys_Execute(char * const *argv, const char *wd, FILE **in, FILE **out, FILE **err, bool showWindow)
{
	int inPipes[2], outPipes[2], errPipes[2];

	if (in)
		if (pipe(inPipes))
			return -1;

	if (out)
		if (pipe(outPipes))
			return -1;

	if (err)
		if (pipe(errPipes))
			return -1;

	pid_t p = fork();
	if (!p) {
		if (in) {
			close(inPipes[1]);
			dup2(inPipes[0], STDIN_FILENO);
		}

		if (out) {
			close(outPipes[0]);
			dup2(outPipes[1], STDOUT_FILENO);
		}

		if (err) {
			close(errPipes[0]);
			dup2(errPipes[1], STDERR_FILENO);
		}

		if (wd)
			chdir(wd);

		execv(argv[0], argv);
	} else {
		if (in) {
			close(inPipes[0]);
			*in = fdopen(inPipes[1], "w");
		}

		if (out) {
			close(outPipes[1]);
			*out = fdopen(outPipes[0], "r");
		}

		if (err) {
			close(errPipes[1]);
			*err = fdopen(errPipes[0], "r");
		}
	}

	return (intptr_t)p;
}

bool
Sys_TerminateProcess(intptr_t handle)
{
	return !kill((pid_t)handle, SIGTERM);
}

bool
Sys_LockMemory(void *mem, size_t size)
{
	return mlock(mem, size) == 0;
}

bool
Sys_UnlockMemory(void *mem, size_t size)
{
	return munlock(mem, size) == 0;
}

void
Sys_DebugBreak(void)
{
	raise(SIGTRAP);
}

bool Net_InitPlatform(void) { return true; }

int32_t
Net_Socket(enum NeSocketType type, enum NeSocketProto proto)
{
	int st = SOCK_STREAM, sp = IPPROTO_TCP;

	switch (type) {
	case ST_STREAM: st = SOCK_STREAM; break;
	case ST_DGRAM: st = SOCK_DGRAM; break;
	case ST_RAW: st = SOCK_RAW; break;
	}

	switch (sp) {
	case SP_TCP: sp = IPPROTO_TCP; break;
	case SP_UDP: sp = IPPROTO_UDP; break;
	}

	return socket(AF_INET, st, sp);
}

bool
Net_Connect(int32_t socket, const char *host, uint16_t port)
{
	struct hostent *h = gethostbyname(host);
	if (!h)
		return false;

	struct sockaddr_in addr =
	{
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr = *(struct in_addr *)h->h_addr
	};
	return connect(socket, (struct sockaddr *)&addr, sizeof(addr)) == 0;
}

bool
Net_Listen(int32_t socket, uint16_t port, int32_t backlog)
{
	struct sockaddr_in addr =
	{
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(port)
	};
	if (bind(socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return false;
	
	return listen(socket, backlog) == 0;
}

int32_t
Net_Accept(int32_t socket)
{
	return accept(socket, NULL, 0);
}

ssize_t
Net_Send(int32_t socket, const void *data, uint32_t count)
{
	return send(socket, data, count, 0);
}

ssize_t
Net_Recv(int32_t socket, void *data, uint32_t count)
{
	return recv(socket, data, count, 0);
}

void
Net_Close(int32_t socket)
{
	close(socket);
}

void Net_TermPlatform(void) { }

void
CpuInfo(void)
{
	char buff[512];
	char *cpuNameId = NULL, *cpuFreqId = NULL, *cpuCoreId = NULL;
	FILE *fp = fopen("/proc/cpuinfo", "r");
 
#	if defined(__i386__) || defined(__amd64__) || defined(__x86_64__)
		cpuNameId = "model name";
		cpuFreqId = "cpu MHz";
		cpuCoreId = "cpu cores";
#	elif defined(SYS_ARCH_ARM) || defined(SYS_ARCH_ARM64)
		cpuNameId = "model name";
		cpuFreqId = "clock";
#	elif defined(SYS_ARCH_MIPS) || defined(SYS_ARCH_MIPS64)
		cpuNameId = "cpu model";
		cpuFreqId = "clock";
#	else
#		warning "Cpu info not implemented for this architecture (Platform/UNIX/UNIX.c)"
#	endif

	if (!cpuNameId || !cpuFreqId)
		return;
 
	f_cpuFreq = 0;
	memset(buff, 0x0, 512);
 
	if (fp) {
		while (fgets(buff, 512, fp)) {
			if (strstr(buff, cpuNameId)) {
				char *ptr = strchr(buff, ':');
				strlcpy(f_cpuName, ptr ? ptr + 2 : "Unknown", sizeof(f_cpuName));
			} else if (strstr(buff, cpuFreqId)) {
				char *ptr = strchr(buff, ':');
				if (ptr) {
					ptr += 2;
					ptr[strlen(ptr) - 1] = 0x0;
					f_cpuFreq = atoi(ptr);
				}
			} else if (cpuCoreId && strstr(buff, cpuCoreId)) {
				char *ptr = strchr(buff, ':');
				if (ptr) {
					ptr += 2;
					ptr[strlen(ptr) - 1] = 0x0;
					f_cpuCount = atoi(ptr);
					f_cpuThreadCount = sysconf(_SC_NPROCESSORS_ONLN);
				}
				break;
			}
			memset(buff, 0x0, 512);
		}
		fclose(fp);
	}

	if (f_cpuName[strlen(f_cpuName) - 1] == '\n')
		f_cpuName[strlen(f_cpuName) - 1] = 0x0;

	if (f_cpuCount && f_cpuThreadCount)
		return;

#if defined(__i386__) || defined(__amd64__) || defined(__x86_64__)
	uint32_t a = 11, b = 0, c = 1, d = 0;
	asm volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(a), "2"(c) : );

	printf("have cpu count: %d, %d\n", a, b);

	f_cpuCount = a;
	f_cpuThreadCount = b;

	return;
#endif

#ifdef SYS_PLATFORM_IRIX
		f_cpuCount = sysconf(_SC_NPROC_ONLN);
#else
#	ifndef _SC_NPROCESSORS_ONLN
		f_cpuCount = sysconf(HW_NCPU);
#	else
		f_cpuCount = sysconf(_SC_NPROCESSORS_ONLN);
#	endif
#endif

	f_cpuThreadCount = f_cpuCount;
}

// Directory Watch

struct DirWatch
{
	int fd, wd;
	NeDirWatchCallback cb;
	void *ud;
	pthread_t thread;
	bool stop;
};

static void *
DirWatchThreadProc(struct DirWatch *dw)
{
	uint8_t buff[sizeof(struct inotify_event) + NAME_MAX + 1];

	while (!dw->stop) {
		bzero(buff, sizeof(buff));
		const ssize_t rd = read(dw->fd, buff, sizeof(buff));
		if (rd < 0)
			continue;

		ssize_t offset = 0;
		while (offset < rd) {
			struct inotify_event *evt = (struct inotify_event *)&buff[offset];

			if ((evt->mask & IN_CREATE) == IN_CREATE)
				dw->cb(evt->name, FE_Create, dw->ud);
			else if ((evt->mask & IN_DELETE) == IN_DELETE)
				dw->cb(evt->name, FE_Delete, dw->ud);
			else if ((evt->mask & IN_CLOSE_WRITE) == IN_CLOSE_WRITE)
				dw->cb(evt->name, FE_Modify, dw->ud);

			offset += sizeof(*evt) + evt->len;
		}
	}

	pthread_exit(NULL);
}

void *
Sys_CreateDirWatch(const char *path, enum NeFSEvent mask, NeDirWatchCallback callback, void *ud)
{
	struct DirWatch *dw = Sys_Alloc(sizeof(*dw), 1, MH_System);

	dw->ud = ud;
	dw->cb = callback;
	dw->fd = inotify_init();

	int imask = 0;
	if ((mask & FE_Create) == FE_Create)
		imask |= IN_CREATE;

	if ((mask & FE_Delete) == FE_Delete)
		imask |= IN_DELETE;

	if ((mask & FE_Modify) == FE_Modify)
		imask |= IN_CLOSE_WRITE;

	dw->wd = inotify_add_watch(dw->fd, path, imask);

	pthread_create(&dw->thread, NULL, (void *(*)(void *))DirWatchThreadProc, dw);
	return dw;
}

void
Sys_DestroyDirWatch(void *handle)
{
	struct DirWatch *dw = handle;

	dw->stop = true;

	inotify_rm_watch(dw->fd, dw->wd);
	close(dw->fd);

	pthread_join(dw->thread, NULL);

	Sys_Free(dw);
}

/* NekoEngine
 *
 * Android.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
