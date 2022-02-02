#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <System/Log.h>
#include <System/Thread.h>
#include <System/Memory.h>
#include <System/System.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#define LOG_BUFF	4096

static const char *_logSeverityStr[4] =
{
	"Debug",
	"Information",
	"Warning",
	"Critical"
};
static const char *_logFile = NULL;
static uint32_t *_minLogSeverity = NULL;
static NeFutex _logFutex;

bool
Sys_InitLog(const char *file)
{
	_logFile = E_GetCVarStr("Engine_LogFile", file ? file : "Engine.log")->str;
	_minLogSeverity = &E_GetCVarU32("Engine_MinLogSeverity", LOG_WARNING)->u32;

	return Sys_InitFutex(&_logFutex);
}

void
Sys_LogEntry(const char *module, uint8_t severity, const char *format, ...)
{
	FILE *fp = NULL;
	va_list args;
	size_t msg_len = 0;
	char *buff = NULL;
	time_t t;
	struct tm *tm = NULL;

	if (!format)
		return;

#if !defined(_DEBUG)
	if (severity < *_minLogSeverity)
		return;
#endif

	Sys_LockFutex(_logFutex);

	buff = Sys_Alloc(sizeof(*buff), LOG_BUFF, MH_Transient);
	memset(buff, 0x0, LOG_BUFF);

	va_start(args, format);
	vsnprintf(buff, LOG_BUFF, format, args);
	va_end(args);

	msg_len = strnlen(buff, LOG_BUFF);

	// Log all messages in debug mode
#if !defined(_DEBUG)
	if (severity == LOG_CRITICAL || _logFile[0] == 0x0)
#else
	Sys_DbgOut(severity, module, _logSeverityStr[severity], buff);
#endif

	if (severity > LOG_ALL)
		severity = LOG_ALL;

	if (_logFile[0] == 0x0)
		goto exit;

	if ((fp = fopen(_logFile, "a+")) == NULL) {
		fprintf(stderr, "failed to open log file for append [%s]: %d\n", _logFile, errno);
		goto exit;
	}

	t = time(0);
	tm = localtime(&t);

	fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s]: %s",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec,
		module, _logSeverityStr[severity], buff);

	if (buff[msg_len - 1] != '\n')
		fprintf(fp, "\n");

	fclose(fp);

exit:
	Sys_UnlockFutex(_logFutex);
}

void
Sys_TermLog(void)
{
	Sys_TermFutex(_logFutex);
}
