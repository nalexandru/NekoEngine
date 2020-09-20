#ifndef _SYS_WINDOW_H_
#define _SYS_WINDOW_H_

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

int Sys_CreateWindow(void);
void Sys_SetWindowTitle(const wchar_t *title);
void Sys_DestroyWindow(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_WINDOW__H_ */