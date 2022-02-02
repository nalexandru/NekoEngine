#include <stdio.h>

#if defined(_M_AMD64) || defined(_M_IX86) && _MSC_VER >= 1400
#	include <intrin.h>
#endif

#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Application.h>

#include "Win32Platform.h"
#include <ShlObj.h>

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

bool
Sys_InitDbgOut(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!IsDebuggerPresent() && Sys_MachineType() == MT_PC) {
		if (AttachConsole) {
			FreeConsole();
			AllocConsole();
			AttachConsole(GetCurrentProcessId());
		}

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
	int prot = 0, flags = 0;

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
			memcpy(_cpu, "Unknown", 7);
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
Sys_ExecutableLocation(char *buff, uint32_t len)
{
	GetModuleFileNameA(NULL, buff, len);
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
			(void)snprintf(spStr, sizeof(spStr), " Service Pack %d.%d", osvi.wServicePackMajor, osvi.wServicePackMinor);
		else if (osvi.wServicePackMajor)
			(void)snprintf(spStr, sizeof(spStr), " Service Pack %d", osvi.wServicePackMajor);
	} else {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
		GetVersionExA((LPOSVERSIONINFOA)&osvi);

		(void)snprintf(spStr, sizeof(spStr), "%s", osvi.szCSDVersion);
	}

	switch (osvi.dwPlatformId) {
	case VER_PLATFORM_WIN32s: (void)snprintf(_osName, sizeof(_osName), "%s", "Windows 3.x (Win32s)"); break;
	case VER_PLATFORM_WIN32_WINDOWS: (void)snprintf(_osName, sizeof(_osName), "%s", "Windows"); break;
	case VER_PLATFORM_WIN32_NT: (void)snprintf(_osName, sizeof(_osName), "%s%s%s", "Windows NT",
		(osvi.wSuiteMask & VER_SUITE_EMBEDDEDNT) ? " Embedded" : "",
		(osvi.wProductType & VER_NT_WORKSTATION) ? " Workstation" : " Server"); break;
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

		sscanf(verStr, "%d.%d", &_osVersion.major, &_osVersion.minor);
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
