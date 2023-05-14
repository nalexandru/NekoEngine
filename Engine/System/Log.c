#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <System/Log.h>
#include <System/Thread.h>
#include <System/Memory.h>
#include <System/System.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#define LOG_BUFF	4096

static const char *f_logSeverityStr[4] =
{
	"Debug",
	"Information",
	"Warning",
	"Critical"
};
static const char *f_logFile = NULL;
static NeFutex f_logFutex;

#if !defined(_DEBUG)
static uint32_t *f_minLogSeverity = NULL;
#endif

bool
Sys_InitLog(const char *file)
{
	f_logFile = E_GetCVarStr("Engine_LogFile", file ? file : "Engine.log")->str;

#if !defined(_DEBUG)
	f_minLogSeverity = &E_GetCVarU32("Engine_MinLogSeverity", LOG_WARNING)->u32;
#endif

	return Sys_InitFutex(&f_logFutex);
}

void
Sys_LogEntry(const char *module, uint8_t severity, const char *format, ...)
{
	FILE *fp = NULL;
	va_list args;
	char buff[LOG_BUFF];
	time_t t;
	struct tm *tm = NULL;

	if (!format || !f_logFile)
		return;

#if !defined(_DEBUG)
	if (severity < *f_minLogSeverity)
		return;
#endif

	Sys_LockFutex(f_logFutex);
	memset(buff, 0x0, sizeof(buff));

	va_start(args, format);
	vsnprintf(buff, sizeof(buff), format, args);
	va_end(args);

	const size_t msg_len = strnlen(buff, sizeof(buff));

	// Log all messages in debug mode
#if !defined(_DEBUG)
	if (severity == LOG_CRITICAL || f_logFile[0] == 0x0)
#else
	Sys_DbgOut(severity, module, f_logSeverityStr[severity], buff);
#endif

	if (severity > LOG_ALL)
		severity = LOG_ALL;

	if (f_logFile[0] == 0x0)
		goto exit;

	if ((fp = fopen(f_logFile, "a+")) == NULL) {
		fprintf(stderr, "failed to open log file for append [%s]: %d\n", f_logFile, errno);
		goto exit;
	}

	t = time(0);
	tm = localtime(&t);

	fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s]: %s",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			module, f_logSeverityStr[severity], buff);

	if (buff[msg_len - 1] != '\n')
		fprintf(fp, "\n");

	fclose(fp);

exit:
	Sys_UnlockFutex(f_logFutex);
}

void
Sys_TermLog(void)
{
	Sys_TermFutex(f_logFutex);
}

/* NekoEngine
 *
 * Log.c
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
