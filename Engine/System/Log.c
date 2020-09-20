#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <System/System.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#define LOG_BUFF	4096

static const wchar_t *_logSeverityStr[4] =
{
	L"Debug",
	L"Information",
	L"Warning",
	L"Critical"
};
static const char *_logFile = NULL;
static uint32_t *_minLogSeverity = NULL;

void
Sys_LogEntry(const wchar_t *module, uint8_t severity, const wchar_t *format, ...)
{
	FILE *fp = NULL;
	va_list args;
	size_t msg_len = 0;
	wchar_t *buff = NULL;
	time_t t;
	struct tm *tm = NULL;

	if (!format)
		return;

	if (!_logFile) {
		_logFile = E_GetCVarStr(L"Engine_LogFile", "Claire.log")->str;
		_minLogSeverity = &E_GetCVarU32(L"Engine_MinLogSeverity", LOG_WARNING)->u32;
	}

#if !defined(_DEBUG)
	if (severity < *_minLogSeverity)
		return;
#endif

	buff = Sys_Alloc(sizeof(wchar_t), LOG_BUFF, MH_Transient);
	memset(buff, 0x0, LOG_BUFF);

	va_start(args, format);
	vswprintf(buff, LOG_BUFF, format, args);
	va_end(args);

	msg_len = wcslen(buff);

	// Log all messages in debug mode
#if !defined(_DEBUG)
	if (severity == LOG_CRITICAL || _logFile[0] == 0x0)
#else
	Sys_DbgOut(severity, module, _logSeverityStr[severity], buff);
#endif

	if (severity > LOG_ALL)
		severity = LOG_ALL;

	if (_logFile[0] == 0x0)
		return;

	if ((fp = fopen(_logFile, "a+")) == NULL) {
		fprintf(stderr, "failed to open log file for append [%s]: %d\n", _logFile, errno);
		return;
	}

	t = time(0);
	tm = localtime(&t);

	fwprintf(fp, L"%d-%d-%d-%d:%d:%d [%s][%s]: %s",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec,
		module, _logSeverityStr[severity], buff);

	if (buff[msg_len - 1] != '\n')
		fwprintf(fp, L"\n");

	fclose(fp);
}

