#ifndef _E_EVENT_H_
#define _E_EVENT_H

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*EventHandlerProc)(void *user, void *args);

void E_Broadcast(const wchar_t *event, void *args);

uint64_t	E_RegisterHandler(const wchar_t *event, EventHandlerProc handler, void *user);
void		E_UnregisterHandler(uint64_t handler);

bool E_InitEventSystem(void);
void E_TermEventSystem(void);
void E_ProcessEvents(void);

#ifdef __cplusplus
}
#endif

#endif /* _E_EVENT_H */