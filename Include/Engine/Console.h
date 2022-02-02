#ifndef _NE_ENGINE_CONSOLE_H_
#define _NE_ENGINE_CONSOLE_H_

#include <stdarg.h>

#include <Engine/Types.h>
#include <Input/Input.h>

bool E_InitConsole(void);

void E_ConsolePuts(const char *s);
void E_ConsolePrint(const char *fmt, ...);
void E_ClearConsole(void);
void E_ToggleConsole(void);
void E_DrawConsole(void);

bool E_ConsoleKey(enum NeButton key, bool down);

void E_TermConsole(void);

#endif /* _NE_ENGINE_CONSOLE_H_ */
