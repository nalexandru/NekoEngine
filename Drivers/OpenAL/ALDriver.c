#include <stdio.h>
#include <stdlib.h>

#include <Audio/Audio.h>
#include <Audio/Driver.h>

#include "OpenALDriver.h"

#define VKDMOD L"OpenALDrv"

static bool _Init(void);
static void _Update(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct AudioDeviceInfo *);

static struct AudioDriver _drv =
{
	.identifier = NE_AUDIO_DRIVER_ID,
	.apiVersion = NE_AUDIO_DRIVER_API,
	.driverName = L"OpenAL",
	.Init = _Init,
	.Update = _Update,
	.Term = _Term,
	.EnumerateDevices = _EnumerateDevices,
	.CreateDevice = AL_CreateDevice,
	.DestroyDevice = AL_DestroyDevice
};

#ifdef AUDIO_DRIVER_BUILTIN
const struct AudioDriver *Au_LoadBuiltinDriver() { return &_drv; }
#else
#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

EXPORT const struct AudioDriver *Au_LoadDriver(void) { return &_drv; }
#endif

static bool
_Init(void)
{
	return true;
}

static void
_Update(void)
{
}

static void
_Term(void)
{
}

static bool
_EnumerateDevices(uint32_t *count, struct AudioDeviceInfo *info)
{
	if (!*count || !info) {
		*count = 1;
		return true;
	}

/*	VkPhysicalDevice *dev = Sys_Alloc(sizeof(VkPhysicalDevice), *count, MH_Transient);
	if (!dev)
		return false;

	if (vkEnumeratePhysicalDevices(Vkd_inst, count, dev) != VK_SUCCESS)
		return false;

	for (uint32_t i = 0; i < *count; ++i) {
		snprintf(info[i].deviceName, sizeof(info[i].deviceName), "%s", props->deviceName);

		info[i].features.meshShading = msFeatures->meshShader;
		info[i].features.rayTracing = rtFeatures->rayTracingPipeline && vk12Features->bufferDeviceAddress;
		info[i].features.discrete = props->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		info[i].features.drawIndirectCount = vk12Features->drawIndirectCount;
		info[i].features.textureCompression = features->features.textureCompressionBC;


		info[i].private = dev[i];
	}*/
	
	return true;
}
