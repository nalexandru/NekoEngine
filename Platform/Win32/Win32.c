#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

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
#include <Engine/Config.h>

#include "Win32Platform.h"

#include <fcntl.h>
#include <io.h>

// NVIDIA Optimus
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;

__declspec(dllexport) HINSTANCE Win32_instance;

struct DirWatch
{
	bool stop;
	HANDLE dir, thread;
	DWORD filter, bufferSize;
	FILE_NOTIFY_INFORMATION *buffer;
	void *ud;
	NeDirWatchCallback cb;
	OVERLAPPED ol;
};

static uint32_t f_cpuCount = 0, f_cpuThreadCount = 0, f_cpuFreq = 0;
static HANDLE f_stdout, f_stderr;
static HMODULE f_k32, f_dwmapi;
static WORD f_cpuArch, f_colors[4] =
{
	13, 7, 14, 12
};
static WORD f_defaultColor;
static char f_hostname[MAX_COMPUTERNAME_LENGTH + 1], f_cpu[50], f_osName[128], f_osVersionString[48];
static struct NeSysVersion f_osVersion;
static bool f_dbgConsole;

BOOL (WINAPI *k32_AttachConsole)(DWORD) = NULL;
void (WINAPI *k32_GetNativeSystemInfo)(LPSYSTEM_INFO) = NULL;
BOOL (WINAPI *k32_GetLogicalProcessorInformation)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD) = NULL;
HRESULT (WINAPI *dwmapi_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD) = NULL;

#ifdef NE_NT5_SUPPORT
static HMODULE f_s32, f_u32, f_xInput2;

BOOL (WINAPI *k32_CancelIoEx)(HANDLE, LPOVERLAPPED) = NULL;
BOOL (WINAPI *u32_RegisterRawInputDevices)(PCRAWINPUTDEVICE, UINT, UINT) = NULL;
UINT (WINAPI *u32_GetRawInputData)(HRAWINPUT, UINT, LPVOID, PUINT, UINT) = NULL;
LRESULT (WINAPI *u32_DefRawInputProc)(PRAWINPUT *, INT, UINT) = NULL;
HRESULT (WINAPI *s32_SHGetKnownFolderPath)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *) = NULL;
void (WINAPI *k32_InitializeSRWLock)(PSRWLOCK) = NULL;
void (WINAPI *k32_AcquireSRWLockExclusive)(PSRWLOCK) = NULL;
void (WINAPI *k32_ReleaseSRWLockExclusive)(PSRWLOCK) = NULL;
void (WINAPI *k32_InitializeConditionVariable)(PCONDITION_VARIABLE) = NULL;
void (WINAPI *k32_WakeConditionVariable)(PCONDITION_VARIABLE) = NULL;
void (WINAPI *k32_WakeAllConditionVariable)(PCONDITION_VARIABLE) = NULL;
BOOL (WINAPI *k32_SleepConditionVariableCS)(PCONDITION_VARIABLE, PCRITICAL_SECTION, DWORD) = NULL;
BOOL (WINAPI *k32_SleepConditionVariableSRW)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG) = NULL;
DWORD (WINAPI *xi2_XInputGetState)(DWORD dwUserIndex, XINPUT_STATE *) = NULL;

static bool InitCompat(void);
#endif

static inline void LoadOSInfo(void);
static inline void CalcCPUFreq(void);
static DWORD DirWatchThreadProc(struct DirWatch *dw);

int
Sys_Main(int argc, char *argv[])
{
#if _DEBUG
//	_crtBreakAlloc = 6081;
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
#endif

	if (!E_Init(argc, argv))
		return -1;

	return E_Run();
}

wchar_t *
NeWin32_UTF8toUCS2L(const char *text, int ulen)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, text, ulen, NULL, 0);
	wchar_t *out = Sys_Alloc(sizeof(*out), len, MH_Transient);
	MultiByteToWideChar(CP_UTF8, 0, text, ulen, out, len);
	return out;
}

wchar_t *
NeWin32_UTF8toUCS2(const char *text)
{
	return NeWin32_UTF8toUCS2L(text, -1);
}

char *
NeWin32_UCS2toUTF8L(const wchar_t *text, int wlen)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, text, wlen, NULL, 0, NULL, NULL);
	char *out = Sys_Alloc(sizeof(*out), len, MH_Transient);
	WideCharToMultiByte(CP_UTF8, 0, text, wlen, out, len, NULL, NULL);
	return out;
}

char *
NeWin32_UCS2toUTF8(const wchar_t *text)
{
	return NeWin32_UCS2toUTF8L(text, -1);
}

bool
Sys_InitDbgOut(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if ((f_dbgConsole = (CVAR_BOOL("Win32_ForceDebugConsole") || !IsDebuggerPresent()) && Sys_MachineType() == MT_PC)) {
		FreeConsole();
		AllocConsole();

		if (k32_AttachConsole)
			k32_AttachConsole(GetCurrentProcessId());

		(void)freopen("CON", "w", stdout);
		(void)freopen("CON", "w", stderr);

		SetConsoleTitleA("Debug Console");
	}

	f_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	f_stderr = GetStdHandle(STD_ERROR_HANDLE);

	GetConsoleScreenBufferInfo(f_stdout, &csbi);
	f_defaultColor = csbi.wAttributes;

	return true;
}

void
Sys_DbgOut(int color, const char *module, const char *severity, const char *text)
{
	SetConsoleTextAttribute(f_stdout, f_colors[color]);

	if (!f_dbgConsole && IsDebuggerPresent()) {
		char buff[4096];
		snprintf(buff, 4096, "[%s][%s]: %s\n", module, severity, text);
		OutputDebugStringA(buff);
	} else {
		fprintf(stdout, "[%s][%s]: %s\n", module, severity, text);
	}

	SetConsoleTextAttribute(f_stdout, f_defaultColor);
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
		mapAccess = FILE_MAP_READ | FILE_MAP_LARGE_PAGES;
		fileAccess = GENERIC_READ;
		fileShare = FILE_SHARE_READ;
		protect = PAGE_READONLY | SEC_LARGE_PAGES;
	} else {
		mapAccess = FILE_MAP_WRITE | FILE_MAP_LARGE_PAGES;
		fileAccess = GENERIC_READ | GENERIC_WRITE;
		fileShare = FILE_SHARE_WRITE;
		protect = PAGE_READWRITE | SEC_LARGE_PAGES;
	}

	file = CreateFileW(NeWin32_UTF8toUCS2(path), fileAccess, fileShare, NULL, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	LARGE_INTEGER sz;
	GetFileSizeEx(file, &sz);
	*size = sz.QuadPart;

	map = CreateFileMappingW(file, NULL, protect, 0, 0, NULL);
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
	DWORD size = sizeof(f_hostname);

	if (!f_hostname[0])
		GetComputerNameA(f_hostname, &size);

	return f_hostname;
}

const char *
Sys_Machine(void)
{
	switch (f_cpuArch)
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
	if (!f_cpu[0]) {
#if defined(_M_AMD64) || defined(_M_IX86)
		int cpuInfo[4] = { 0 };

		__cpuid(cpuInfo, 0x80000002);
		memcpy(f_cpu, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000003);
		memcpy(f_cpu + 16, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000004);
		memcpy(f_cpu + 32, cpuInfo, sizeof(cpuInfo));
#elif defined(_M_ARM64)
#else // Unknown architecture, attempt to read from registry
		HKEY key;
		DWORD size = 0;

		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &key)
				== ERROR_SUCCESS) {
			size = sizeof(f_cpu);
			RegQueryValueExA(key, "ProcessorNameString", 0, 0, (LPBYTE)f_cpu, &size);

			RegCloseKey(key);
		}
#endif

		if (!f_cpu[0])
			snprintf(f_cpu, sizeof(f_cpu), "Unknown");
	}

	return f_cpu;
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
	return f_osName;
}

const char *
Sys_OperatingSystemVersionString(void)
{
	return f_osVersionString;
}

struct NeSysVersion
Sys_OperatingSystemVersion(void)
{
	return f_osVersion;
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
		case MSG_ICON_NONE: type = 0; break;
		case MSG_ICON_INFO: type |= MB_ICONINFORMATION; break;
		case MSG_ICON_WARN: type |= MB_ICONWARNING; break;
		case MSG_ICON_ERROR: type |= MB_ICONERROR; break;
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

	f_k32 = LoadLibrary(TEXT("kernel32"));
	if (!f_k32)
		return false;

	f_dwmapi = LoadLibrary(TEXT("dwmapi"));

#ifdef NE_NT5_SUPPORT
	if (!InitCompat())
		return false;
#endif

	k32_AttachConsole = (BOOL (WINAPI *)(DWORD))GetProcAddress(f_k32, "AttachConsole");
	k32_GetNativeSystemInfo = (void (WINAPI *)(LPSYSTEM_INFO))GetProcAddress(f_k32, "GetNativeSystemInfo");
	k32_GetLogicalProcessorInformation = (BOOL (WINAPI *)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD))GetProcAddress(f_k32, "GetLogicalProcessorInformation");

	if (!k32_GetNativeSystemInfo)
		k32_GetNativeSystemInfo = GetSystemInfo;

	k32_GetNativeSystemInfo(&si);
	f_cpuArch = si.wProcessorArchitecture;

	if (k32_GetLogicalProcessorInformation) {
		DWORD len = 0;
		k32_GetLogicalProcessorInformation(NULL, &len);

		SYSTEM_LOGICAL_PROCESSOR_INFORMATION *info = calloc(len, sizeof(*info));
		if (!info)
			return false;

		k32_GetLogicalProcessorInformation(info, &len);

		f_cpuCount = 0;
		f_cpuThreadCount = 0;
		for (DWORD i = 0; i < len / sizeof(*info); ++i) {
			if (info[i].Relationship != RelationProcessorCore)
				continue;

			++f_cpuCount;

			DWORD bitSetCount = 0;
			ULONG_PTR bitTest = (ULONG_PTR) 1 << (sizeof(ULONG_PTR) * 8 - 1);

			for (DWORD j = 0; j <= sizeof(ULONG_PTR) * 8 - 1; ++j) {
				bitSetCount += ((info[i].ProcessorMask & bitTest) ? 1 : 0);
				bitTest /= 2;
			}

			f_cpuThreadCount += bitSetCount;
		}

		free(info);
	} else {
		f_cpuCount = f_cpuThreadCount = si.dwNumberOfProcessors;
	}

	LoadOSInfo();
	CalcCPUFreq();

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	k32_SetThreadDescription = (HRESULT (WINAPI *)(HANDLE, PCWSTR))GetProcAddress(f_k32, "SetThreadDescription");

#ifdef _M_IX86	// Enable low precision on x86 platforms
	_controlfp(_PC_24, _MCW_PC);
#endif

	return true;
}

void
Sys_TermPlatform(void)
{
	CoUninitialize();

#ifdef NE_NT5_SUPPORT
	FreeLibrary(f_u32);
	FreeLibrary(f_s32);

	if (f_xInput2)
		FreeLibrary(f_xInput2);
#endif

	if (f_dwmapi)
		FreeLibrary(f_dwmapi);

	FreeLibrary(f_k32);
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
#ifdef NE_NT5_SUPPORT
	if (!InitCompat())
		return;
#endif

	WCHAR *path = NULL;
	memset(out, 0x0, len);

	switch (sd) {
	case SD_SAVE_GAME: Win32_SHGetKnownFolderPath(&FOLDERID_SavedGames, 0, NULL, &path); break;
	case SD_APP_DATA: Win32_SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &path); break;
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

	for (char *p = strrchr(dir, '\\'); p > dir; --p) {
		if (*p != '\\' && *p != '/')
			continue;

		*p = 0x0;

		if (Sys_DirectoryExists(dir))
			break;

		if (!CreateDirectoryA(dir, NULL) && GetLastError() != ERROR_PATH_NOT_FOUND)
			return false;

		*p = '\\';
	}

	return !CreateDirectoryA(path, NULL) ? GetLastError() == ERROR_PATH_NOT_FOUND : true;
}

void *
Sys_CreateDirWatch(const char *path, enum NeFSEvent mask, NeDirWatchCallback callback, void *ud)
{
	struct DirWatch *dw = Sys_Alloc(sizeof(*dw), 1, MH_System);

	dw->ud = ud;
	dw->cb = callback;
	dw->dir = CreateFileW(NeWin32_UTF8toUCS2(path), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
							NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if ((mask & FE_Create))
		dw->filter |= FILE_NOTIFY_CHANGE_CREATION;

	if ((mask & FE_Create) == FE_Create || (mask & FE_Delete) == FE_Delete)
		dw->filter |= FILE_NOTIFY_CHANGE_FILE_NAME;

	if (mask & FE_Modify)
		dw->filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;

	dw->ol.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DirWatchThreadProc, dw, 0, NULL);

	return dw;
}

void
Sys_DestroyDirWatch(void *handle)
{
	if (!handle)
		return;

	struct DirWatch *dw = handle;

	dw->stop = true;

#ifndef NE_NT5_SUPPORT
	CancelIoEx(dw->dir, &dw->ol);
	WaitForSingleObject(dw->thread, INFINITE);
#else
	/* is there a better way of stopping ReadDirectoryChangesW on NT5.x and earlier ? */
	if (k32_CancelIoEx) {
		k32_CancelIoEx(dw->dir, &dw->ol);
		WaitForSingleObject(dw->thread, INFINITE);
	} else {
		TerminateThread(dw->thread);
	}
#endif

	CloseHandle(dw->ol.hEvent);
	CloseHandle(dw->dir);
	Sys_Free(dw);
}

void
Sys_ExecutableLocation(char *buff, size_t len)
{
	DWORD dLen = (DWORD)len;
	LPWSTR str = Sys_Alloc(sizeof(*str), len, MH_Transient);
	GetModuleFileNameW(NULL, str, dLen);
	strlcpy(buff, NeWin32_UCS2toUTF8(str), len);
	char *p = strrchr(buff, '\\');
	*p = 0x0;
}

void
Sys_GetWorkingDirectory(char *buff, size_t len)
{
	DWORD dLen = (DWORD)len;
	LPWSTR str = Sys_Alloc(sizeof(*str), len, MH_Transient);
	GetCurrentDirectoryW(dLen, str);
	strlcpy(buff, NeWin32_UCS2toUTF8(str), len);
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
	strlcpy(buff, NeWin32_UCS2toUTF8(str), len);
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
	char *cmdline = Sys_Alloc(sizeof(*cmdline), 32768, MH_Transient);
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
Sys_LockMemory(void *mem, size_t size)
{
	return VirtualLock(mem, size);
}

bool
Sys_UnlockMemory(void *mem, size_t size)
{
	return VirtualUnlock(mem, size);
}

void
Sys_DebugBreak(void)
{
	__debugbreak();
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
LoadOSInfo(void)
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

		(void)snprintf(spStr, sizeof(spStr), " %s", osvi.szCSDVersion);
	}

	switch (osvi.dwPlatformId) {
	case VER_PLATFORM_WIN32s: (void)strlcpy(f_osName, "Windows 3.x (Win32s)", sizeof(f_osName)); break;
	case VER_PLATFORM_WIN32_WINDOWS: (void)strlcpy(f_osName, "Windows", sizeof(f_osName)); break;
	case VER_PLATFORM_WIN32_NT: (void)snprintf(f_osName, sizeof(f_osName), "Windows NT%s%s",
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
		RegQueryValueExA(key, "CurrentVersion", 0, NULL, (LPBYTE)verStr, &regSize);

		regSize = sizeof(buildStr);
		RegQueryValueExA(key, "CurrentBuild", 0, NULL, (LPBYTE)buildStr, &regSize);

		/*
		 * At some point Microsoft changed the above registry
		 * key to have the value 6.3 for Win32 programs and
		 * 10.0 for UWP programs. The RTM build of Windows 10
		 * is 10240, so if the version is 6.3.x where x is >
		 * than 10240 set the appropriate verStr.
		 */
		if (!strncmp(verStr, "6.3", 3) && atoi(buildStr) > 10240)
			(void)strlcpy(verStr, "10.0", sizeof(verStr));

		(void)sscanf(verStr, "%u.%u", &f_osVersion.major, &f_osVersion.minor);
		f_osVersion.revision = atoi(buildStr);

		(void)snprintf(f_osVersionString, sizeof(f_osVersionString), "%s.%s%s", verStr, buildStr, spStr);
	} else {
		f_osVersion.major = osvi.dwMajorVersion;
		f_osVersion.minor = osvi.dwMinorVersion;
		f_osVersion.revision = osvi.dwBuildNumber;

		(void)snprintf(f_osVersionString, sizeof(f_osVersionString), "%lu.%lu.%lu%s", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, spStr);
	}
}

void
CalcCPUFreq(void)
{
	DWORD winFSB, winRes;
	double cpuFSB, cpuRes, freq;

	cpuFSB = (double)__rdtsc();
	winFSB = GetTickCount();

	Sleep(300);

	cpuRes = (double)__rdtsc();
	winRes = GetTickCount();

	freq = cpuRes - cpuFSB;
	freq /= winRes - winFSB;

	f_cpuFreq = (uint32_t)(freq / 1000);
}

static DWORD
DirWatchThreadProc(struct DirWatch *dw)
{
	uint8_t buff[4096] = { 0 };
	char pathBuff[MAX_PATH];

	while (!dw->stop) {
		ZeroMemory(buff, sizeof(buff));

#ifndef NE_NT5_SUPPORT
		ReadDirectoryChangesW(dw->dir, buff, sizeof(buff), TRUE, dw->filter, NULL, &dw->ol, NULL);
		if (WaitForSingleObject(dw->ol.hEvent, INFINITE) != WAIT_OBJECT_0)
			continue;
#else
		if (k32_CancelIoEx) {
			ReadDirectoryChangesW(dw->dir, buff, sizeof(buff), TRUE, dw->filter, NULL, &dw->ol, NULL);
			if (WaitForSingleObject(dw->ol.hEvent, INFINITE) != WAIT_OBJECT_0)
				continue;
		} else {
			DWORD bytes;
			if (!ReadDirectoryChangesW(dw->dir, buff, sizeof(buff), TRUE, dw->filter, &bytes, NULL, NULL))
				continue;
		}
#endif

		size_t offset = 0;
		while (offset < sizeof(buff)) {
			FILE_NOTIFY_INFORMATION *fni = (FILE_NOTIFY_INFORMATION *)&buff[offset];
			if (offset && offset == fni->NextEntryOffset)
				break;

			ZeroMemory(pathBuff, sizeof(pathBuff));
			WideCharToMultiByte(CP_UTF8, 0, fni->FileName, fni->FileNameLength, pathBuff, sizeof(pathBuff), NULL, NULL);

			switch (fni->Action) {
			case FILE_ACTION_ADDED: dw->cb(pathBuff, FE_Create, dw->ud); break;
			case FILE_ACTION_REMOVED: dw->cb(pathBuff, FE_Delete, dw->ud); break;
			case FILE_ACTION_MODIFIED: dw->cb(pathBuff, FE_Modify, dw->ud); break;
			}

			if (!(offset = fni->NextEntryOffset))
				break;
		}
	}

	return 0;
}

#ifdef NE_NT5_SUPPORT

HRESULT WINAPI
Win32c_SHGetKnownFolderPath(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath)
{
	*ppszPath = NULL;
	WCHAR *append = NULL;

	int csidl = 0;
	if (IsEqualGUID(rfid, &FOLDERID_SavedGames)) {
		csidl = CSIDL_PROFILE;
		append = L"\\Saved Games";
	} else if (IsEqualGUID(rfid, &FOLDERID_LocalAppData)) {
		csidl = CSIDL_LOCAL_APPDATA;
	} else {
		return E_INVALIDARG;
	}

	TCHAR path[MAX_PATH] = { 0 };
	const HRESULT hr = SHGetFolderPathW(HWND_DESKTOP, csidl, hToken, dwFlags, path);
	if (FAILED(hr))
		return hr;

	*ppszPath = CoTaskMemAlloc(sizeof(path) + (append ? wcslen(append) : 0));
	if (!*ppszPath)
		return E_OUTOFMEMORY;

	memcpy(*ppszPath, path, sizeof(path));
	if (append)
		wcsncat(*ppszPath, append, wcslen(append));

	return hr;
}

static bool
InitCompat(void)
{
	if (s32_SHGetKnownFolderPath)
		return true; // already initialized

	f_u32 = LoadLibrary(TEXT("user32"));
	if (!f_u32)
		return false;

	f_s32 = LoadLibrary(TEXT("shell32"));
	if (!f_s32)
		return false;

	f_xInput2 = LoadLibrary(TEXT("XInput2"));

	k32_CancelIoEx = (BOOL (WINAPI *)(HANDLE, LPOVERLAPPED))GetProcAddress(f_k32, "CancelIoEx");

	k32_InitializeSRWLock = (void (WINAPI *)(PSRWLOCK))GetProcAddress(f_k32, "InitializeSRWLock");
	k32_AcquireSRWLockExclusive = (void (WINAPI *)(PSRWLOCK))GetProcAddress(f_k32, "AcquireSRWLockExclusive");
	k32_ReleaseSRWLockExclusive = (void (WINAPI *)(PSRWLOCK))GetProcAddress(f_k32, "ReleaseSRWLockExclusive");

	k32_InitializeConditionVariable = (void (WINAPI *)(PCONDITION_VARIABLE))GetProcAddress(f_k32, "InitializeConditionVariable");
	k32_WakeConditionVariable = (void (WINAPI *)(PCONDITION_VARIABLE))GetProcAddress(f_k32, "WakeConditionVariable");
	k32_WakeAllConditionVariable = (void (WINAPI *)(PCONDITION_VARIABLE))GetProcAddress(f_k32, "WakeAllConditionVariable");
	k32_SleepConditionVariableCS = (BOOL (WINAPI *)(PCONDITION_VARIABLE, PCRITICAL_SECTION, DWORD))GetProcAddress(f_k32, "SleepConditionVariableCS");
	k32_SleepConditionVariableSRW = (BOOL (WINAPI *)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG))GetProcAddress(f_k32, "SleepConditionVariableSRW");

	u32_RegisterRawInputDevices = (BOOL (WINAPI *)(PCRAWINPUTDEVICE, UINT, UINT))GetProcAddress(f_u32, "RegisterRawInputDevices");
	u32_GetRawInputData = (UINT (WINAPI *)(HRAWINPUT, UINT, LPVOID, PUINT, UINT))GetProcAddress(f_u32, "GetRawInputData");
	u32_DefRawInputProc = (LRESULT (WINAPI *)(PRAWINPUT *, INT, UINT))GetProcAddress(f_u32, "DefRawInputProc");

	s32_SHGetKnownFolderPath = (HRESULT (WINAPI *)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *))GetProcAddress(f_s32, "SHGetKnownFolderPath");
	if (!s32_SHGetKnownFolderPath)
		s32_SHGetKnownFolderPath = Win32c_SHGetKnownFolderPath;

	if (f_dwmapi)
		dwmapi_DwmSetWindowAttribute = (HRESULT (WINAPI *)(HWND, DWORD, LPCVOID, DWORD))GetProcAddress(f_dwmapi, "DwmSetWindowAttribute");

	if (f_xInput2)
		xi2_XInputGetState = (DWORD (WINAPI *)(DWORD dwUserIndex, XINPUT_STATE *))GetProcAddress(f_xInput2, "XInputGetState");

	if (!k32_InitializeConditionVariable) {
		// k32_InitializeConditionVariable is used to determine if the system supports condition variables
		// so we'll leave it NULL.

		k32_WakeConditionVariable = Win32c_WakeConditionVariable;
		k32_WakeAllConditionVariable = Win32c_WakeAllConditionVariable;
		k32_SleepConditionVariableCS = Win32c_SleepConditionVariableCS;
		k32_SleepConditionVariableSRW = Win32c_SleepConditionVariableSRW;
	}

	return true;
}

#endif

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
