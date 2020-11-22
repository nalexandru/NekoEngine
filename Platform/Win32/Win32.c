#include <stdio.h>

#if defined(_M_AMD64) || defined(_M_IX86) && _MSC_VER >= 1400
#	include <intrin.h>
#else
#	define IASM_IMPL
	static inline void __cpuid(int dst[4], int code);
	static inline uint64_t __rdtsc(void);
#endif

#if _MSC_VER < 1400 // This *will* break
#	define _aligned_malloc(x, y) malloc(x)
#	define _aligned_free(x) free(x)
#endif

// Not defined due to _WIN32_WINNT being set to 0x0500
#define WM_INPUT	0x00FF

#include <System/System.h>
#include <Engine/Engine.h>

#include "Win32Platform.h"

static int32_t _numCpus = 0, _cpuFreq = 0;
static HANDLE _stdout, _stderr, _k32;
static WORD _cpuArch, _colors[4] =
{
	13, 7, 14, 12
};
static WORD _defaultColor;
static char _hostname[MAX_COMPUTERNAME_LENGTH + 1], _cpu[50], _osName[128], _osVersion[48];

static inline void _LoadOSInfo(void);
static inline void _CalcCPUFreq(void);

static BOOL (WINAPI *k32_IsDebuggerPresent)(void) = NULL;
static BOOL (WINAPI *k32_FreeConsole)(void) = NULL;
static BOOL (WINAPI *k32_AllocConsole)(void) = NULL;
static BOOL (WINAPI *k32_AttachConsole)(DWORD) = NULL;
static BOOL (WINAPI *k32_GetFileSizeEx)(HANDLE, PLARGE_INTEGER) = NULL;

bool
Sys_InitDbgOut(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!k32_IsDebuggerPresent)
		return true;

	if (!k32_IsDebuggerPresent() && Sys_MachineType() == MT_PC) {
		if (k32_AttachConsole) {
			k32_FreeConsole();
			k32_AllocConsole();
			k32_AttachConsole(GetCurrentProcessId());
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
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	if (!k32_IsDebuggerPresent) {
		fwprintf(stdout, L"[%ls][%ls]: %ls\n", module, severity, text);
		return;
	}

	SetConsoleTextAttribute(_stdout, _colors[color]);

	if (k32_IsDebuggerPresent()) {
		wchar_t buff[4096];
		swprintf(buff, 4096, L"[%ls][%ls]: %ls\n", module, severity, text);
		OutputDebugStringW(buff);
	} else {
		fwprintf(stdout, L"[%ls][%ls]: %ls\n", module, severity, text);
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

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ctr);

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

	if (k32_GetFileSizeEx) {
		LARGE_INTEGER sz;
		k32_GetFileSizeEx(file, &sz);
		*size = sz.QuadPart;
	} else {
		DWORD sz;
		GetFileSize(file, &sz);
		*size = sz;
	}

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

int32_t
Sys_CpuFreq(void)
{
	return _cpuFreq;
}

int32_t
Sys_NumCpus(void)
{
	return _numCpus;
}

const char *
Sys_OperatingSystem(void)
{
	return _osName;
}

const char *
Sys_OperatingSystemVersion(void)
{
	return _osVersion;
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
	return !IsIconic(E_Screen);
}

bool
Sys_UniversalWindows(void)
{
	return false;
}

void
Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon)
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

	MessageBoxW((HWND)E_Screen, message, title, type);
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
	return LoadLibraryA(path);
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
	void (WINAPI *_GetSystemInfo)(LPSYSTEM_INFO) = NULL;

	_k32 = LoadLibraryW(L"kernel32");
	if (!_k32)
		return false;

	_GetSystemInfo = (void (WINAPI *)(LPSYSTEM_INFO))GetProcAddress(_k32, "GetNativeSystemInfo");
	if (!_GetSystemInfo)
		_GetSystemInfo = (void (WINAPI *)(LPSYSTEM_INFO))GetProcAddress(_k32, "GetSystemInfo");

	_GetSystemInfo(&si);
	_numCpus = si.dwNumberOfProcessors;
	_cpuArch = si.wProcessorArchitecture;

	_LoadOSInfo();
	_CalcCPUFreq();

	k32_IsDebuggerPresent = (BOOL (WINAPI *)(void))GetProcAddress(_k32, "IsDebuggerPresent");
	k32_FreeConsole = (BOOL (WINAPI *)(void))GetProcAddress(_k32, "FreeConsole");
	k32_AllocConsole = (BOOL (WINAPI *)(void))GetProcAddress(_k32, "AllocConsole");
	k32_AttachConsole = (BOOL (WINAPI *)(DWORD))GetProcAddress(_k32, "AttachConsole");
	k32_GetFileSizeEx = (BOOL (WINAPI *)(HANDLE, PLARGE_INTEGER))GetProcAddress(_k32, "GetFileSizeEx");

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	k32_SetThreadDescription = (HRESULT (WINAPI *)(HANDLE, PCWSTR))GetProcAddress(_k32, "SetThreadDescription");
	k32_InitializeSRWLock = (void (WINAPI *)(PSRWLOCK))GetProcAddress(_k32, "InitializeSRWLock");
	k32_AcquireSRWLockExclusive = (void (WINAPI *)(PSRWLOCK))GetProcAddress(_k32, "AcquireSRWLockExclusive");
	k32_ReleaseSRWLockExclusive = (void (WINAPI *)(PSRWLOCK))GetProcAddress(_k32, "ReleaseSRWLockExclusive");

	k32_InitializeConditionVariable = (void (WINAPI *)(PCONDITION_VARIABLE))GetProcAddress(_k32, "InitializeConditionVariable");

	if (k32_InitializeConditionVariable) {
		k32_SleepConditionVariableSRW = (BOOL (WINAPI *)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG))GetProcAddress(_k32, "SleepConditionVariableSRW");
		k32_SleepConditionVariableCS = (BOOL (WINAPI *)(PCONDITION_VARIABLE, PCRITICAL_SECTION, DWORD))GetProcAddress(_k32, "SleepConditionVariableCS");
		k32_WakeAllConditionVariable = (void (WINAPI *)(PCONDITION_VARIABLE))GetProcAddress(_k32, "WakeAllConditionVariable");
		k32_WakeConditionVariable = (void (WINAPI *)(PCONDITION_VARIABLE))GetProcAddress(_k32, "WakeConditionVariable");
	} else {
		k32_ConditionVariableSize = sizeof(struct Win32CompatCV);
		k32_InitializeConditionVariable = win32Compat_InitializeConditionVariable;
		k32_SleepConditionVariableSRW = win32Compat_SleepConditionVariableSRW;
		k32_SleepConditionVariableCS = win32Compat_SleepConditionVariableCS;
		k32_WakeAllConditionVariable = win32Compat_WakeAllConditionVariable;
		k32_WakeConditionVariable = win32Compat_WakeConditionVariable;
		k32_DeleteConditionVariable = win32Compat_DeleteConditionVariable;
	}

#if WINVER < 0x0501
	SecureZeroMemory = (void (WINAPI *)(PVOID, SIZE_T))GetProcAddress(_k32, )
#endif

	return true;
}

void
Sys_TermPlatform(void)
{
	CoUninitialize();

	FreeLibrary(_k32);
}

void *
Sys_AlignedAlloc(size_t size, size_t alignment)
{
	return _aligned_malloc(size, alignment);
}

void
Sys_AlignedFree(void *mem)
{
	_aligned_free(mem);
}

void
Sys_ZeroMemory(void *mem, size_t size)
{
#if WINVER >= 0x0501
	SecureZeroMemory(mem, size);
#else
	// This branch is taken only when compiling with an SDK older than Windows XP / Server 2003,
	// because SecureZeroMemory is inlined. See:
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85)
	ZeroMemory(mem, size);
#endif
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
			(void)snprintf(spStr, sizeof(spStr), "Service Pack %d.%d", osvi.wServicePackMajor, osvi.wServicePackMinor);
		else if (osvi.wServicePackMajor)
			(void)snprintf(spStr, sizeof(spStr), "Service Pack %d", osvi.wServicePackMajor);
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
		 * than 10240 set the appropriate ver_str.
		 */
		if (!strncmp(verStr, "6.3", 3) && atoi(buildStr) > 10240)
			(void)snprintf(verStr, sizeof(verStr), "10.0");

		(void)snprintf(_osVersion, sizeof(_osVersion), "%s.%s%s", verStr, buildStr, spStr);
	} else {
		(void)snprintf(_osVersion, sizeof(_osVersion), "%lu.%lu.%lu%s", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, spStr);
	}
}

void
_CalcCPUFreq(void)
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

	_cpuFreq = (uint32_t)(freq / 1000);
}

#ifdef IASM_IMPL
void
__cpuid(int dst[4], int code)
{
	__asm {
		mov eax, code;
		cpuid;
		mov dst[0], eax
		mov dst[4], ebx
		mov dst[8], ecx
		mov dst[12], edx
	}
}

uint64_t
__rdtsc(void)
{
	unsigned long a, b;
	uint64_t ret;

	__asm {
		RDTSC
		mov [a], eax
		mov [b], edx
	}

	ret = b;
	ret *= 0x100000000;
	ret += a;

	return ret;
}
#endif

