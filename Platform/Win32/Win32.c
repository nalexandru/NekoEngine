#include <stdio.h>

#if defined(_M_AMD64) || defined(_M_IX86) && _MSC_VER >= 1400
#	include <intrin.h>
#endif

#include <float.h>

#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Application.h>
#include <Network/Network.h>

#include "Win32Platform.h"
#include <ShlObj.h>
#include <fcntl.h>
#include <io.h>

// NVIDIA Optimus
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;

__declspec(dllexport) HINSTANCE Win32_instance;

static uint32_t _cpuCount = 0, _cpuThreadCount = 0, _cpuFreq = 0;
static HANDLE _stdout, _stderr, _k32;
static WORD _cpuArch, _colors[4] =
{
	13, 7, 14, 12
};
static WORD _defaultColor;
static char _hostname[MAX_COMPUTERNAME_LENGTH + 1], _cpu[50], _osName[128], _osVersionString[48];
static struct NeSysVersion _osVersion;

static inline void _LoadOSInfo(void);
static inline void _CalcCPUFreq(void);

wchar_t *
NeWin32_UTF8toUCS2(const char *text)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	wchar_t *out = Sys_Alloc(sizeof(*out), len, MH_Transient);
	MultiByteToWideChar(CP_UTF8, 0, text, -1, out, len);
	return out;
}

char *
NeWin32_UCS2toUTF8(const wchar_t *text)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
	char *out = Sys_Alloc(sizeof(*out), len, MH_Transient);
	WideCharToMultiByte(CP_UTF8, 0, text, -1, out, len, NULL, NULL);
	return out;
}

bool
Sys_InitDbgOut(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!IsDebuggerPresent() && Sys_MachineType() == MT_PC) {
		FreeConsole();
		AllocConsole();
		AttachConsole(GetCurrentProcessId());

		(void)freopen("CON", "w", stdout);
		(void)freopen("CON", "w", stderr);

		SetConsoleTitleA("Debug Console");
	}

	_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	_stderr = GetStdHandle(STD_ERROR_HANDLE);

	GetConsoleScreenBufferInfo(_stdout, &csbi);
	_defaultColor = csbi.wAttributes;

	return true;
}

void
Sys_DbgOut(int color, const char *module, const char *severity, const char *text)
{
	SetConsoleTextAttribute(_stdout, _colors[color]);

	if (IsDebuggerPresent()) {
		char buff[4096];
		snprintf(buff, 4096, "[%s][%s]: %s\n", module, severity, text);
		OutputDebugStringA(buff);

		/*wchar_t buff[4096];
		swprintf(buff, 4096, L"[%ls][%ls]: %ls\n", module, severity, text);
		OutputDebugStringW(buff);*/
	} else {
		fprintf(stdout, "[%s][%s]: %s\n", module, severity, text);
	}

	SetConsoleTextAttribute(_stdout, _defaultColor);
}

void
Sys_TermDbgOut(void)
{

}

uint64_t
Sys_Time(void)
{
	LARGE_INTEGER freq, ctr;

	NtQueryPerformanceCounter(&ctr, &freq);

	return ((ctr.QuadPart / freq.QuadPart) * 1000000000UL) + ((ctr.QuadPart % freq.QuadPart) * 1000000000UL / freq.QuadPart);
}

bool
Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size)
{
	HANDLE file, map;
	DWORD fileAccess, fileShare, mapAccess, protect;

	if (!write) {
		mapAccess = FILE_MAP_READ;
		fileAccess = GENERIC_READ;
		fileShare = FILE_SHARE_READ;
		protect = PAGE_READONLY;
	} else {
		mapAccess = FILE_MAP_WRITE;
		fileAccess = GENERIC_READ | GENERIC_WRITE;
		fileShare = FILE_SHARE_WRITE;
		protect = PAGE_READWRITE;
	}

	file = CreateFileA(path, fileAccess, fileShare, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	LARGE_INTEGER sz;
	GetFileSizeEx(file, &sz);
	*size = sz.QuadPart;

	map = CreateFileMappingA(file, NULL, protect, 0, 0, NULL);
	if (!map) {
		CloseHandle(file);
		return false;
	}

	*ptr = MapViewOfFile(map, mapAccess, 0, 0, 0);

	CloseHandle(file);
	CloseHandle(map);

	return true;
}

void
Sys_UnmapFile(const void *ptr, uint64_t size)
{
	UnmapViewOfFile(ptr);
}

const char *
Sys_Hostname(void)
{
	DWORD size = sizeof(_hostname);

	if (!_hostname[0])
		GetComputerNameA(_hostname, &size);

	return _hostname;
}

const char *
Sys_Machine(void)
{
	switch (_cpuArch)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:			return "x64";
	case PROCESSOR_ARCHITECTURE_INTEL:			return "x86";
	case PROCESSOR_ARCHITECTURE_ARM64:			return "arm64";
	case PROCESSOR_ARCHITECTURE_ARM:			return "arm";
	case PROCESSOR_ARCHITECTURE_MIPS:			return "MIPS";
	case PROCESSOR_ARCHITECTURE_ALPHA:			return "Alpha";
	case PROCESSOR_ARCHITECTURE_ALPHA64:		return "Alpha 64";
	case PROCESSOR_ARCHITECTURE_SHX:			return "SuperH";
	case PROCESSOR_ARCHITECTURE_PPC:			return "PowerPC";
	case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:	return "Itanium on x64";
	case PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64:	return "arm on x64";
	case PROCESSOR_ARCHITECTURE_IA64:			return "Itanium";
	case PROCESSOR_ARCHITECTURE_IA32_ON_ARM64:	return "x86 on arm64";
	case PROCESSOR_ARCHITECTURE_MSIL:			return "msil";
	default:									return "Unknown";
	}
}

const char *
Sys_CpuName(void)
{
	if (!_cpu[0]) {
#if defined(_M_AMD64) || defined(_M_IX86)
		int cpuInfo[4] = { 0 };

		__cpuid(cpuInfo, 0x80000002);
		memcpy(_cpu, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000003);
		memcpy(_cpu + 16, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000004);
		memcpy(_cpu + 32, cpuInfo, sizeof(cpuInfo));
#elif defined(_M_ARM64)
#else // Unknown architecture, attempt to read from registry
		HKEY key;
		DWORD size = 0;

		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &key)
				== ERROR_SUCCESS) {
			size = sizeof(_cpu);
			RegQueryValueExA(key, "ProcessorNameString", 0, 0, (LPBYTE)_cpu, &size);

			RegCloseKey(key);
		}
#endif

		if (!_cpu[0])
			snprintf(_cpu, sizeof(_cpu), "Unknown");
	}

	return _cpu;
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

uint64_t
Sys_TotalMemory(void)
{
	MEMORYSTATUSEX memStatus = { sizeof(memStatus) };
	GlobalMemoryStatusEx(&memStatus);
	return memStatus.ullTotalPhys;
}

uint64_t
Sys_FreeMemory(void)
{
	MEMORYSTATUSEX memStatus = { sizeof(memStatus) };
	GlobalMemoryStatusEx(&memStatus);
	return memStatus.ullAvailPhys;
}

const char *
Sys_OperatingSystem(void)
{
	return _osName;
}

const char *
Sys_OperatingSystemVersionString(void)
{
	return _osVersionString;
}

struct NeSysVersion
Sys_OperatingSystemVersion(void)
{
	return _osVersion;
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
	return !IsIconic(E_screen);
}

void
Sys_MessageBox(const char *title, const char *message, int icon)
{
	UINT type = MB_OK;

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

	MessageBoxW((HWND)E_screen, NeWin32_UTF8toUCS2(message), NeWin32_UTF8toUCS2(title), type);
}

bool
Sys_ProcessEvents(void)
{
	MSG msg;

	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_INPUT) {
			HandleInput(msg.hwnd, msg.lParam, msg.wParam);
		} else if (msg.message == WM_QUIT) {
			return false;
		} else {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return true;
}

void *
Sys_LoadLibrary(const char *path)
{
	UINT mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	void *module = NULL;

	if (path) {
		module = LoadLibraryA(path);

		if (!module && !strstr(path, ".dll")) {
			char *newPath = Sys_Alloc(sizeof(*newPath), MAX_PATH, MH_Transient);
			snprintf(newPath, MAX_PATH, "%s.dll", path);

			module = LoadLibraryA(newPath);
		}
	} else {
		module = GetModuleHandle(NULL);
	}

	SetErrorMode(mode);
	return module;
}

void *
Sys_GetProcAddress(void *lib, const char *name)
{
	return GetProcAddress(lib, name);
}

void
Sys_UnloadLibrary(void *lib)
{
	FreeLibrary(lib);
}

bool
Sys_InitPlatform(void)
{
	SYSTEM_INFO si = { 0 };

	Win32_instance = GetModuleHandle(NULL);

	_k32 = LoadLibraryW(L"kernel32");
	if (!_k32)
		return false;

	GetNativeSystemInfo(&si);
	_cpuArch = si.wProcessorArchitecture;

	DWORD len = 0;
	GetLogicalProcessorInformation(NULL, &len);

	SYSTEM_LOGICAL_PROCESSOR_INFORMATION *info = calloc(len, sizeof(*info));
	if (!info)
		return false;

	GetLogicalProcessorInformation(info, &len);

	_cpuCount = 0;
	_cpuThreadCount = 0;
	for (DWORD i = 0; i < len / sizeof(*info); ++i) {
		if (info[i].Relationship != RelationProcessorCore)
			continue;

		++_cpuCount;

		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << (sizeof(ULONG_PTR) * 8 - 1);

		for (DWORD j = 0; j <= sizeof(ULONG_PTR) * 8 - 1; ++j) {
			bitSetCount += ((info[i].ProcessorMask & bitTest) ? 1 : 0);
			bitTest /= 2;
		}

		_cpuThreadCount += bitSetCount;
	}

	free(info);

	_LoadOSInfo();
	_CalcCPUFreq();

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	k32_SetThreadDescription = (HRESULT (WINAPI *)(HANDLE, PCWSTR))GetProcAddress(_k32, "SetThreadDescription");

#ifdef _M_IX86	// Enable low precision on x86 platforms
	_controlfp(_PC_24, _MCW_PC);
#endif

	return true;
}

void
Sys_TermPlatform(void)
{
	CoUninitialize();

	FreeLibrary(_k32);
}

void
Sys_ZeroMemory(void *mem, size_t size)
{
	SecureZeroMemory(mem, size);
}

void
Sys_Sleep(uint32_t sec)
{
	Sleep(sec * 1000);
}

void
Sys_MSleep(uint32_t msec)
{
	LARGE_INTEGER li = { .QuadPart = (LONGLONG)msec * -10000 };
	NtDelayExecution(FALSE, &li);
}

void
Sys_USleep(uint32_t usec)
{
	LARGE_INTEGER li = { .QuadPart = (LONGLONG)usec * -10 };
	NtDelayExecution(FALSE, &li);
}

void
Sys_DirectoryPath(enum NeSystemDirectory sd, char *out, size_t len)
{
	WCHAR *path = NULL;

	memset(out, 0x0, len);

	switch (sd) {
	case SD_SAVE_GAME: SHGetKnownFolderPath(&FOLDERID_SavedGames, 0, NULL, &path); break;
	case SD_APP_DATA: SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &path); break;
	case SD_TEMP: GetTempPathA((DWORD)len, out); break;
	}

	if (path) {
		snprintf(out, len, "%ls\\%s", path, App_applicationInfo.name);
		CoTaskMemFree(path);
	}
}

bool
Sys_FileExists(const char *path)
{
	DWORD attr = GetFileAttributesA(path);
	return attr != INVALID_FILE_ATTRIBUTES;
}

bool
Sys_DirectoryExists(const char *path)
{
	DWORD attr = GetFileAttributesA(path);
	return attr != INVALID_FILE_ATTRIBUTES && attr & FILE_ATTRIBUTE_DIRECTORY;
}

bool
Sys_CreateDirectory(const char *path)
{
	if (CreateDirectoryA(path, NULL))
		return true;

	if (GetLastError() != ERROR_PATH_NOT_FOUND)
		return false;

	char *dir = Sys_Alloc(sizeof(*dir), 4096, MH_Transient);
	memcpy(dir, path, strnlen(path, 4096));

	for (char *p = dir + 1; *p; ++p) {
		if (*p != '/')
			continue;

		*p = 0x0;

		if (!CreateDirectoryA(dir, NULL) && GetLastError() != ERROR_PATH_NOT_FOUND)
			return false;

		*p = '/';
	}

	return !CreateDirectoryA(path, NULL) ? GetLastError() == ERROR_PATH_NOT_FOUND : true;
}

void
Sys_ExecutableLocation(char *buff, size_t len)
{
	DWORD dLen = (DWORD)len;
	LPWSTR str = Sys_Alloc(sizeof(*str), len, MH_Transient);
	GetModuleFileNameW(NULL, str, dLen);
	snprintf(buff, len, "%s", NeWin32_UCS2toUTF8(str));
	char *p = strrchr(buff, '\\');
	*p = 0x0;
}

void
Sys_GetWorkingDirectory(char *buff, size_t len)
{
	DWORD dLen = (DWORD)len;
	LPWSTR str = Sys_Alloc(sizeof(*str), len, MH_Transient);
	GetCurrentDirectoryW(dLen, str);
	snprintf(buff, len, "%s", NeWin32_UCS2toUTF8(str));
}

void
Sys_SetWorkingDirectory(const char *dir)
{
	LPWSTR str = NeWin32_UTF8toUCS2(dir);
	SetCurrentDirectoryW(str);
}

void
Sys_UserName(char *buff, size_t len)
{
	DWORD dLen = (DWORD)len;
	LPWSTR str = Sys_Alloc(sizeof(*str), len, MH_Transient);
	GetUserNameW(str, &dLen);
	snprintf(buff, len, "%s", NeWin32_UCS2toUTF8(str));
}

intptr_t
Sys_GetCurrentProcess()
{
	return (intptr_t)GetCurrentProcess();
}

int32_t
Sys_GetCurrentProcessId()
{
	return GetCurrentProcessId();
}

void
Sys_WaitForProcessExit(intptr_t handle)
{
	WaitForSingleObject((HANDLE)handle, INFINITE);
}

intptr_t
Sys_Execute(char * const *argv, const char *wd, FILE **in, FILE **out, FILE **err, bool showWindow)
{
	HANDLE in_rd = 0, in_wr = 0, out_rd = 0, out_wr = 0, err_rd = 0, err_wr = 0;
	char * const *arg = &argv[1];

	SECURITY_ATTRIBUTES sa =
	{
		.nLength = sizeof(sa),
		.bInheritHandle = TRUE,
		.lpSecurityDescriptor = NULL
	};

	if (in) {
		CreatePipe(&in_rd, &in_wr, &sa, 0);
		SetHandleInformation(in_wr, HANDLE_FLAG_INHERIT, 0);
	}

	if (out) {
		CreatePipe(&out_rd, &out_wr, &sa, 0);
		SetHandleInformation(out_rd, HANDLE_FLAG_INHERIT, 0);
	}

	if (err) {
		CreatePipe(&err_rd, &err_wr, &sa, 0);
		SetHandleInformation(err_rd, HANDLE_FLAG_INHERIT, 0);
	}

	STARTUPINFOA si =
	{
		.cb = sizeof(si),
		.hStdInput = in ? in_rd : NULL,
		.hStdError = err ? err_wr : NULL,
		.hStdOutput = out ? out_wr : NULL,
		.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW,
		.wShowWindow = showWindow ? SW_SHOWDEFAULT : SW_HIDE
	};

	// Maximum size according to
	// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa
	char cmdline[32768] = { 0 };
	while (*arg) {
		size_t len = strlen(cmdline);
		if ((sizeof(cmdline) - len) - strlen(*arg++) < 0)
			break;
		snprintf(cmdline + len, sizeof(cmdline) - len, "%s ", *arg++);
	}

	if (strlen(cmdline))
		cmdline[strlen(cmdline) - 1] = 0x0;

	PROCESS_INFORMATION pi = { 0 };
	BOOL rc = CreateProcessA(argv[0], cmdline, NULL, NULL, TRUE, 0, NULL, wd, &si, &pi);

	if (!rc)
		goto error;

	CloseHandle(pi.hThread);

	// Assume these can't fail
	if (in) {
		CloseHandle(in_rd);
		*in = _fdopen(_open_osfhandle((intptr_t)in_wr, _O_WRONLY), "w");
	}

	if (out) {
		CloseHandle(out_wr);
		*out = _fdopen(_open_osfhandle((intptr_t)out_rd, _O_RDONLY), "r");
	}

	if (err) {
		CloseHandle(err_wr);
		*err = _fdopen(_open_osfhandle((intptr_t)err_rd, _O_RDONLY), "r");
	}

	return (intptr_t)pi.hProcess;

error:
	if (in) {
		CloseHandle(in_rd);
		CloseHandle(in_wr);
	}

	if (out) {
		CloseHandle(out_rd);
		CloseHandle(out_wr);
	}

	if (err) {
		CloseHandle(err_rd);
		CloseHandle(err_wr);
	}

	return -1;
}

bool
Sys_TerminateProcess(intptr_t handle)
{
	return TerminateProcess((HANDLE)handle, 0);
}

bool
Net_InitPlatform(void)
{
	WSADATA wd;
	return WSAStartup(MAKEWORD(2, 2), &wd) == 0;
}

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

	return (NeSocket)socket(AF_INET, st, sp);
}

bool
Net_Connect(NeSocket socket, const char *host, uint16_t port)
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
Net_Listen(NeSocket socket, uint16_t port, int32_t backlog)
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

NeSocket
Net_Accept(NeSocket socket)
{
	return (NeSocket)accept(socket, NULL, 0);
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
	closesocket(socket);
}

void
Net_TermPlatform(void)
{
	WSACleanup();
}

void
_LoadOSInfo(void)
{
	OSVERSIONINFOEXA osvi = { 0 };
	bool embedded = false, server = false;
	char spStr[20] = { 0 };

	osvi.dwOSVersionInfoSize = sizeof(osvi);

	if (GetVersionExA((LPOSVERSIONINFOA)&osvi)) {
		if (osvi.wSuiteMask & VER_SUITE_EMBEDDEDNT)
			embedded = true;

		if (osvi.wProductType != VER_NT_WORKSTATION)
			server = true;

		if (osvi.wServicePackMinor)
			(void)snprintf(spStr, sizeof(spStr), " Service Pack %u.%u", osvi.wServicePackMajor, osvi.wServicePackMinor);
		else if (osvi.wServicePackMajor)
			(void)snprintf(spStr, sizeof(spStr), " Service Pack %u", osvi.wServicePackMajor);
	} else {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
		GetVersionExA((LPOSVERSIONINFOA)&osvi);

		(void)snprintf(spStr, sizeof(spStr), "%s", osvi.szCSDVersion);
	}

	switch (osvi.dwPlatformId) {
	case VER_PLATFORM_WIN32s: (void)snprintf(_osName, sizeof(_osName), "%s", "Windows 3.x (Win32s)"); break;
	case VER_PLATFORM_WIN32_WINDOWS: (void)snprintf(_osName, sizeof(_osName), "%s", "Windows"); break;
	case VER_PLATFORM_WIN32_NT: (void)snprintf(_osName, sizeof(_osName), "%s%s%s", "Windows NT",
		embedded ? " Embedded" : "",
		server ? " Server" : " Workstation"); break;
	}

	if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 && osvi.dwBuildNumber == 9200) {
		/* 
		 * Windows 8.1 or later
		 * Starting with Windows 8.1 the real version is hidden by GetVersionEx
		 * so we have to look in the registry for it
		 */
		HKEY key = { 0 };
		DWORD regSize = 0;
		char verStr[8], buildStr[24];

		RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &key);
		
		regSize = sizeof(verStr);
		RegQueryValueExA(key, "CurrentVersion", 0, NULL, verStr, &regSize);

		regSize = sizeof(buildStr);
		RegQueryValueExA(key, "CurrentBuild", 0, NULL, buildStr, &regSize);

		/*
		 * At some point Microsoft changed the above registry
		 * key to have the value 6.3 for Win32 programs and
		 * 10.0 for UWP programs. The RTM build of Windows 10
		 * is 10240, so if the version is 6.3.x where x is >
		 * than 10240 set the appropriate verStr.
		 */
		if (!strncmp(verStr, "6.3", 3) && atoi(buildStr) > 10240)
			(void)snprintf(verStr, sizeof(verStr), "10.0");

		(void)sscanf(verStr, "%u.%u", &_osVersion.major, &_osVersion.minor);
		_osVersion.revision = atoi(buildStr);

		(void)snprintf(_osVersionString, sizeof(_osVersionString), "%s.%s%s", verStr, buildStr, spStr);
	} else {
		_osVersion.major = osvi.dwMajorVersion;
		_osVersion.minor = osvi.dwMinorVersion;
		_osVersion.revision = osvi.dwBuildNumber;

		(void)snprintf(_osVersionString, sizeof(_osVersionString), "%lu.%lu.%lu%s", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, spStr);
	}
}

void
_CalcCPUFreq(void)
{
	ULONGLONG winFSB, winRes;
	double cpuFSB, cpuRes, freq;

	cpuFSB = (double)__rdtsc();
	winFSB = GetTickCount64();

	Sleep(300);

	cpuRes = (double)__rdtsc();
	winRes = GetTickCount64();

	freq = cpuRes - cpuFSB;
	freq /= winRes - winFSB;

	_cpuFreq = (uint32_t)(freq / 1000);
}

/* NekoEngine
 *
 * Win32.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
