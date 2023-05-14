#include <dlfcn.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/utsname.h>

#include <Alert.h>
#include <Application.h>

#include <Engine/Engine.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Network/Network.h>
#include <Engine/Application.h>

#include "HaikuPlatform.h"

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

NeEngineApplication::NeEngineApplication() : BApplication("application/x-vnd.test.app") { }

int
Sys_Main(int argc, char *argv[])
{
	if (!E_Init(argc, argv))
		return -1;

	return E_Run();
}

extern "C" bool
Sys_InitPlatform(void)
{
	new NeEngineApplication();

	uname(&f_uname);

	return true;
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
	uint64_t pages = sysconf(_SC_PHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
}

uint64_t
Sys_FreeMemory(void)
{
	uint64_t pages = sysconf(_SC_AVPHYS_PAGES);
	uint64_t page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
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
	return !((NeEngineWindow *)E_screen)->IsHidden();
}

void
Sys_MessageBox(const char *title, const char *message, int icon)
{
	alert_type type;
	switch (icon) {
	case MSG_ICON_NONE: type = B_EMPTY_ALERT; break;
	case MSG_ICON_INFO: type = B_INFO_ALERT; break;
	case MSG_ICON_WARN: type = B_WARNING_ALERT; break;
	case MSG_ICON_ERROR: type = B_STOP_ALERT; break;
	}
	
	BAlert *a = new BAlert(title, message, "OK", NULL, NULL, B_WIDTH_AS_USUAL, type);
	a->Go();
}

bool
Sys_ProcessEvents(void)
{
	//
}

void *
Sys_LoadLibrary(const char *name)
{
	char *path = NULL;
	
	if (!name)
		return dlopen(NULL, RTLD_NOW);
	
	if (access(name, R_OK) < 0) {
		const char *prefix = "", *suffix = "";
		size_t len = strlen(name);
		
		path = (char *)Sys_Alloc(sizeof(char), 2048, MH_Transient);
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
Sys_DirectoryPath(enum NeSystemDirectory sd, char *out, size_t len)
{
	switch (sd) {
	case SD_SAVE_GAME: snprintf(out, len, "%s/.config/%s/saves", getenv("HOME"), App_applicationInfo.name); break; 
	case SD_APP_DATA: snprintf(out, len, "%s/.config/%s/data", getenv("HOME"), App_applicationInfo.name); break; 
	case SD_TEMP: snprintf(out, len, "/tmp/"); break;
	}
}

bool
Sys_FileExists(const char *path)
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
		
	char *dir = (char *)Sys_Alloc(sizeof(*dir), 4096, MH_Transient);
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

extern "C" void
Sys_TermPlatform(void)
{
	delete be_app;
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

extern "C" bool Net_InitPlatform(void) { return true; }

NeSocket
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
Net_Connect(NeSocket socket, const char *host, uint16_t port)
{
	struct hostent *h = gethostbyname(host);
	if (!h)
		return false;
	
	struct sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *(struct in_addr *)h->h_addr;
	
	return connect(socket, (struct sockaddr *)&addr, sizeof(addr)) == 0;
}

bool
Net_Listen(NeSocket socket, uint16_t port, int32_t backlog)
{
	struct sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	
	if (bind(socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return false;
	
	return listen(socket, backlog) == 0;
}

NeSocket
Net_Accept(NeSocket socket)
{
	return accept(socket, NULL, 0);
}

ssize_t
Net_Send(NeSocket socket, const void *data, uint32_t count)
{
	return send(socket, data, count, 0);
}

ssize_t
Net_Recv(NeSocket socket, void *data, uint32_t count)
{
	return recv(socket, data, count, 0);
}

void
Net_Close(NeSocket socket)
{
	close(socket);
}

extern "C" void Net_TermPlatform(void) { }

/* NekoEngine
 *
 * Haiku.cxx
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
