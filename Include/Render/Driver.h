#ifndef _RE_DRIVER_H_
#define _RE_DRIVER_H_

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NE_RENDER_DRIVER_ID		0xB15B00B5
#define NE_RENDER_DRIVER_API	1

struct RenderDriver
{
	uint32_t identifier;
	uint32_t apiVersion;
	
	bool (*Init)(void);
	void (*Term)(void);
	
	bool (*EnumerateDevices)(uint32_t *count, struct RenderDeviceInfo *devices);
	struct RenderDevice *(*CreateDevice)(struct RenderDeviceInfo *info,
										 struct RenderDeviceProcs *devProcs,
										 struct RenderContextProcs *ctxProcs);
};

#ifdef RENDER_DRIVER_BUILTIN
const struct RenderDriver *Re_LoadBuiltinDriver(void);
#endif
typedef const struct RenderDriver *(*ReLoadDriverProc)(void);
#ifdef __cplusplus
}
#endif

#endif
