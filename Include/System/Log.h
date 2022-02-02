#ifndef _NE_SYSTEM_LOG_H_
#define _NE_SYSTEM_LOG_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#define LOG_DEBUG			0
#define LOG_INFORMATION		1
#define LOG_WARNING			2
#define LOG_CRITICAL		3

#define LOG_ALL			LOG_INFORMATION

bool Sys_InitLog(const char *file);
void Sys_LogEntry(const char *module, uint8_t severity, const char *format, ...);
void Sys_TermLog(void);

#endif /* _SYSTEM_LOG_H_ */
