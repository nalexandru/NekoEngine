#ifndef _NE_ENGINE_EVENT_H_
#define _NE_ENGINE_EVENT_H_

#include <Engine/Types.h>

typedef void (*NeEventHandlerProc)(void *user, void *args);

void E_Broadcast(const char *event, void *args);

uint64_t E_RegisterHandler(const char *event, NeEventHandlerProc handler, void *user);
void E_UnregisterHandler(uint64_t handler);

bool E_InitEventSystem(void);
void E_TermEventSystem(void);
void E_ProcessEvents(void);

#endif /* _NE_ENGINE_EVENT_H_ */
