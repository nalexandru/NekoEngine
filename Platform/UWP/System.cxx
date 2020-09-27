#include <wrl.h>
#include <wrl/client.h>

#include <intrin.h>

#include <System/System.h>
#include <System/Memory.h>

using namespace Platform;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::Foundation;

extern bool _UWPWindowVisible;
extern bool _UWPWindowClosed;

static int _numCpus = 0;
static int _xboxOne = -1;

bool
Sys_InitDbgOut(void)
{
	return true;
}

void
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	if (IsDebuggerPresent()) {
		wchar_t buff[4096];
		swprintf(buff, 4096, L"[%ls][%ls]: %ls\n", module, severity, text);
		OutputDebugString(buff);
	} else {
		fwprintf(stdout, L"[%ls][%ls]: %ls\n", module, severity, text);
	}
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

	wchar_t *buff = (wchar_t *)Sys_Alloc(sizeof(wchar_t), strlen(path) + 1, MH_Transient);
	mbstowcs(buff, path, strlen(path) + 1);

	CREATEFILE2_EXTENDED_PARAMETERS cf2ep =
	{
		sizeof(cf2ep),
		FILE_ATTRIBUTE_NORMAL,
		FILE_FLAG_SEQUENTIAL_SCAN
	};
	HANDLE file = CreateFile2(buff, fileAccess, fileShare, OPEN_EXISTING, &cf2ep);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	LARGE_INTEGER liSize;
	GetFileSizeEx(file, &liSize);

	*size = liSize.QuadPart;

	HANDLE map = CreateFileMappingW(file, NULL, protect, 0, 0, NULL);
	if (map == INVALID_HANDLE_VALUE) {
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

uint32_t
Sys_TlsAlloc(void)
{
	return TlsAlloc();
}

void *
Sys_TlsGet(uint32_t key)
{
	return TlsGetValue(key);
}

void
Sys_TlsSet(uint32_t key, void *data)
{
	TlsSetValue(key, data);
}

void
Sys_TlsFree(uint32_t key)
{
	TlsFree(key);
}

void
Sys_Yield(void)
{
	SwitchToThread();
}

int
Sys_NumCpus(void)
{
	return _numCpus;
}

enum MachineType
Sys_MachineType(void)
{
	if (_xboxOne < 0) {
		int cpu_info[4] = { 0 };
		char cpu_name[48] = { 0 };

		__cpuid(cpu_info, 0x80000002);
		memcpy(cpu_name, cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000003);
		memcpy(cpu_name + 16, cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000004);
		memcpy(cpu_name + 32, cpu_info, sizeof(cpu_info));

		_xboxOne = strstr(cpu_name, "Microsoft Xbox One") != NULL;
	}

	return _xboxOne ? MT_XBOX_ONE : MT_PC;
}

uint32_t
Sys_Capabilities(void)
{
	return SC_MMIO;
}

bool
Sys_ScreenVisible(void)
{
	return _UWPWindowVisible;
}

bool
Sys_UniversalWindows(void)
{
	return true;
}

void
Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon)
{
	MessageDialog ^md = ref new MessageDialog(ref new String(message), ref new String(title));
	IAsyncOperation<IUICommand ^> ^op = md->ShowAsync();
	
	while (op->Status == AsyncStatus::Started)
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
}

bool
Sys_ProcessEvents(void)
{
	if (_UWPWindowClosed)
		return false;

	CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

	return true;
}

extern "C" bool
Sys_InitPlatform(void)
{
	SYSTEM_INFO si = { 0 };

	GetNativeSystemInfo(&si);

	_numCpus = si.dwNumberOfProcessors;

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	return true;
}

extern "C" void
Sys_TermPlatform(void)
{
	CoUninitialize();
}

