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

#include "OpenGLDriver.h"

static bool _Init(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct NeRenderDeviceInfo *);

static struct NeRenderDriver _drv =
{
	.identifier = NE_RENDER_DRIVER_ID,
	.apiVersion = NE_RENDER_DRIVER_API,
	.driverName = "OpenGL",
	.graphicsApiId = RE_API_OPENGL,
	.Init = _Init,
	.Term = _Term,
	.EnumerateDevices = _EnumerateDevices,
	.CreateDevice = GL_CreateDevice,
	.DestroyDevice = GL_DestroyDevice
};

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

EXPORT const struct NeRenderDriver *Re_LoadOpenGLDriver() { return &_drv; }

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

	const GLubyte *str = glGetString(GL_RENDERER);
	snprintf(info[0].deviceName, sizeof(info[0].deviceName), "%s", str);

	GLint val;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
	info[0].limits.maxTextureSize = val;

	info[0].features.unifiedMemory = false;
	info[0].features.rayTracing = false;
	info[0].features.indirectRayTracing = false;
	info[0].features.meshShading = false; // TODO
	info[0].features.discrete = true;
	info[0].features.canPresent = true;

	info[0].features.drawIndirectCount = false; // TODO
	info[0].features.bcTextureCompression = true; // TOOD
	info[0].features.astcTextureCompression = false; // TODO
	info[0].features.multiDrawIndirect = true;
	info[0].features.secondaryCommandBuffers = false;
	info[0].features.coherentMemory = true;

	GL_HardwareInfo(&info[0]);

	return true;
}
