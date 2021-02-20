#ifndef _SYS_WINDOW_H_
#define _SYS_WINDOW_H_

#include <wchar.h>

int Sys_CreateWindow(void);
void Sys_SetWindowTitle(const wchar_t *title);
void Sys_DestroyWindow(void);

#endif /* _SYS_WINDOW__H_ */
