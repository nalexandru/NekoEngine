#ifndef _SYS_SYSTEM_H_
#define _SYS_SYSTEM_H_

#include <wchar.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum MsgBoxIcon
{
	MSG_ICON_NONE = 0,
	MSG_ICON_INFO = 1,
	MSG_ICON_WARN = 2,
	MSG_ICON_ERROR = 3
};

enum MachineType
{
	MT_PC,
	MT_XBOX_360,
	MT_XBOX_ONE,
	MT_PS3,
	MT_XBOX
};

enum SystemCapabilityFlags
{
	SC_MMIO = 0x01
};

bool Sys_Init(void);
void Sys_Term(void);

bool Sys_InitDbgOut(void);
void Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text);
void Sys_TermDbgOut(void);

uint64_t Sys_Time(void);

bool Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size);
void Sys_UnmapFile(const void *ptr, uint64_t size);

uint32_t Sys_TlsAlloc(void);
void *Sys_TlsGet(uint32_t key);
void Sys_TlsSet(uint32_t key, void *data);
void Sys_TlsFree(uint32_t key);

void Sys_Yield(void);

int Sys_NumCpus(void);

enum MachineType Sys_MachineType(void);
uint32_t Sys_Capabilities(void);

bool Sys_ScreenVisible(void);
bool Sys_UniversalWindows(void);
void Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon);
bool Sys_ProcessEvents(void);

// Compatibility shivs
void *reallocarray(void *ptr, size_t nmemb, size_t size);

int getopt(int nargc, char *const nargv[], const char *ostr);

extern int opterr, optind, optopt, optreset;
extern char *optarg;

#ifdef SNPRINTF_COMPAT
int snprintf(char *, size_t, const char *, /*args*/ ...);
#endif

#ifdef WCSDUP_COMPAT
static inline wchar_t *wcsdup(const wchar_t *str)
{
	size_t len = wcslen(str) * sizeof(*str);
	wchar_t *copy = (wchar_t *)calloc(1, len + sizeof(*str));
	memmove(copy, str, len);
	return copy;
}
#endif

#ifdef STRTOULL_COMPAT
static inline uint64_t strtoull(const char *str, char **endptr, int base)
{
	return strtoul(str, endptr, base);
}
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1400)
#define vswprintf _vsnwprintf
#define swprintf _snwprintf
#define strtoll strtol

static inline float strtof(const char *str, const char **endptr)
{
	const char *end = str;
	bool dot = false;

	if (endptr) {
		for (end; *end; ++end) {
			if (*end > 0x2F && *end < 0x3A)	// 0x30 - 0x39 ASCII digits
				continue;

			if (!dot && *end == '.') {
				dot = true;
				continue;
			}

			break;
		}

		*endptr = end;
	}

	return (float)atof(str);
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SYSTEM_H_ */
