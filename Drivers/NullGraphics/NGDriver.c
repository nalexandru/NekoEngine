#include <stdio.h>
#include <stdlib.h>

#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>
#include <Engine/Version.h>
#include <Engine/Application.h>
#include <Render/Driver/Driver.h>

#include "NullGraphicsDriver.h"

static bool _Init(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct NeRenderDeviceInfo *);

static struct NeRenderDriver _drv =
{
	.identifier = NE_RENDER_DRIVER_ID,
	.apiVersion = NE_RENDER_DRIVER_API,
	.driverName = "NullGraphics",
	.graphicsApiId = RE_API_NULL,
	.Init = _Init,
	.Term = _Term,
	.EnumerateDevices = _EnumerateDevices,
	.CreateDevice = NG_CreateDevice,
	.DestroyDevice = NG_DestroyDevice
};

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

EXPORT const struct NeRenderDriver *Re_LoadNullDriver(void) { return &_drv; }

static bool
_Init(void)
{
	return true;
}

static void
_Term(void)
{
}

static bool
_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *info)
{
	if (!*count || !info) {
		*count = 1;
		return true;
	}

	snprintf(info[0].deviceName, sizeof(info[0].deviceName), "Null Graphics Device");
	info[0].features.unifiedMemory = true;
	info[0].features.rayTracing = false;
	info[0].features.indirectRayTracing = false;
	info[0].features.meshShading = false;
	info[0].features.discrete = false;
	info[0].features.canPresent = true;
	info[0].features.drawIndirectCount = false;
	info[0].features.bcTextureCompression = false;
	info[0].features.astcTextureCompression = false;
	info[0].features.multiDrawIndirect = false;
	info[0].features.secondaryCommandBuffers = false;
	info[0].features.coherentMemory = true;
	info[0].limits.maxTextureSize = 16384;
	info[0].hardwareInfo.deviceId = 0xB00B;
	info[0].hardwareInfo.vendorId = RE_VENDOR_ID_ATI_AMD;
	info[0].hardwareInfo.driverVersion = 1;
	info[0].localMemorySize = 1024 * 1024 * 1024;

	return true;
}
