#ifndef _DBG_LOGGER_H_
#define _DBG_LOGGER_H_

#include <stdbool.h>
#include <Windows.h>

#define MSG_BUFF_LEN	8192

void LogMessage(const wchar_t *msg);

bool InitServer(void);
void TermServer(void);

#endif /* _DBG_LOGGER_H_ */

