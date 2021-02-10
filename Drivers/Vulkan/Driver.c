#include <Render/Driver.h>
#include <Render/Device.h>
#include <Render/Context.h>

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

static bool _Init(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct RenderDeviceInfo *);
static struct RenderDevice *_CreateDevice(struct RenderDeviceInfo *info,
	struct RenderDeviceProcs *devProcs,
	struct RenderContextProcs *ctxProcs);

static struct RenderDriver _drv =
{
	NE_RENDER_DRIVER_ID,
	NE_RENDER_DRIVER_API,
	_Init,
	_Term,
	_EnumerateDevices,
	_CreateDevice
};

EXPORT const struct RenderDriver *Re_LoadDriver(void) { return &_drv; }

static bool
_Init(void)
{
	// create instance
}

static void
_Term(void)
{
	// destroy instance
}

static bool
_EnumerateDevices(uint32_t *count, struct RenderDeviceInfo *info)
{
	return false;
}

static struct RenderDevice *
_CreateDevice(struct RenderDeviceInfo *info,
	struct RenderDeviceProcs *devProcs,
	struct RenderContextProcs *ctxProcs)
{
	return NULL;
}

