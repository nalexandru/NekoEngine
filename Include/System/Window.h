#ifndef _NE_SYSTEM_WINDOW_H_
#define _NE_SYSTEM_WINDOW_H_

#include <wchar.h>
#include <stdbool.h>

bool Sys_CreateWindow(void);
void Sys_SetWindowTitle(const wchar_t *title);
void Sys_MoveWindow(int x, int y);
void Sys_DestroyWindow(void);

#endif /* _NE_SYSTEM_WINDOW_H_ */
