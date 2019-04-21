/* Miwa Portable Runtime
 *
 * log.c
 * Author: Alexandru Naiman
 *
 * Logger
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019, Alexandru Naiman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <system/log.h>
#include <system/compat.h>
#include <system/platform.h>

#define LOG_BUFF	4096

#ifdef SYS_PLATFORM_WINDOWS
	#include <windows.h>
#endif

char _log_file[1024];
static uint8_t _log_min_severity = 0;

static const char *_log_severity_str[4] =
{
	"Debug",
	"Information",
	"Warning",
	"Critical"
};

int
log_init(const char *file,
	uint8_t min_severity)
{
	size_t len = 0;

	if (!file)
		return SYS_INVALID_ARGS;

	len = strlen(file);
	if (!len)
		return SYS_INVALID_ARGS;

	strlcpy(_log_file, file, 1024);
	_log_min_severity = min_severity;

	return SYS_OK;
}

void
log_entry(const char *module,
	uint8_t severity,
	const char *format,
	...)
{
	FILE *fp = NULL;
	va_list args;
	size_t msg_len = 0;
	char buff[LOG_BUFF];
	time_t t;
	struct tm *tm = NULL;
	
	if (!format)
		return;

	memset(buff, 0x0, LOG_BUFF);

	va_start(args, format);
	vsnprintf(buff, LOG_BUFF, format, args);
	va_end(args);

	msg_len = strlen(buff);

	// Log all messages in debug mode
#if !defined(_DEBUG)
	if (severity < _log_min_severity)
		return;

	if (severity == LOG_CRITICAL)
#else
	{
#ifdef SYS_PLATFORM_WINDOWS
		char dbg_buff[2048];
		snprintf(dbg_buff, 2048, "[%s][%s]: %s\n", module, _log_severity_str[severity], buff);
		OutputDebugStringA(dbg_buff);
#else
		fprintf(stderr, "[%s][%s]: %s", module, _log_severity_str[severity], buff);
#endif
		if (buff[msg_len - 1] != '\n')
			fprintf(stderr, "\n");
	}
#endif

	if (severity > LOG_ALL)
		severity = LOG_ALL;

	if ((fp = fopen(_log_file, "a+")) == NULL) {
		fprintf(stderr, "failed to open log file for append [%s]: %d\n", _log_file, errno);
		return;
	}

	t = time(0);
	tm = localtime(&t);

	fprintf(fp, "%d-%d-%d-%d:%d:%d [%s][%s]: %s",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec,
		module, _log_severity_str[severity], buff);

	if (buff[msg_len - 1] != '\n')
		fprintf(fp, "\n");

	fclose(fp);
}

