#ifndef _RE_DRIVER_H_
#define _RE_DRIVER_H_

#include <Render/Types.h>

#define NE_RENDER_DRIVER_ID		0xB15B00B5
#define NE_RENDER_DRIVER_API	3

struct RenderDriver
{
	uint32_t identifier;
	uint32_t apiVersion;
	wchar_t driverName[64];
	
	bool (*Init)(void);
	void (*Term)(void);
	
	bool (*EnumerateDevices)(uint32_t *count, struct RenderDeviceInfo *devices);
	struct RenderDevice *(*CreateDevice)(struct RenderDeviceInfo *info,
										 struct RenderDeviceProcs *devProcs,
										 struct RenderContextProcs *ctxProcs);
	void (*DestroyDevice)(struct RenderDevice *dev);
};

#ifdef RENDER_DRIVER_BUILTIN
const struct RenderDriver *Re_LoadBuiltinDriver(void);
#endif
typedef const struct RenderDriver *(*ReLoadDriverProc)(void);

ENGINE_API extern const struct RenderDriver *Re_driver;

#endif
