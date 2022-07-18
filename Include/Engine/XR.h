#ifndef _NE_ENGINE_XR_H_
#define _NE_ENGINE_XR_H_

#include <Engine/Types.h>

#include <Engine/BuildConfig.h>

bool E_InitXR(void);

bool E_CreateXrSession(void);
bool E_CreateXrSwapchain(void);

void E_XrAcquireImage(void);
void E_XrPresent(void);
bool E_ProcessXrEvents(void);

void E_TermXR(void);

#if ENABLE_OPENXR
#include <openxr/openxr.h>

extern XrInstance E_xrInstance;
extern XrSystemId E_xrSystemId;

extern uint32_t E_xrColorId, E_xrDepthId;
#else
extern void *E_xrInstance;
#endif

#endif /* _NE_ENGINE_XR_H_ */
