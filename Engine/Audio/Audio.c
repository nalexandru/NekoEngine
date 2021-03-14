#include <System/Log.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Audio/Audio.h>
#include <Audio/Driver.h>
#include <Audio/Device.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>

#define AU_MOD L"Audio"
#define CHK_FAIL(x, y) if (!x) { Sys_LogEntry(AU_MOD, LOG_CRITICAL, y); return false; }

struct AudioDevice *Au_device;
struct AudioDeviceInfo Au_deviceInfo = { 0 };
struct AudioDeviceProcs Au_deviceProcs = { 0 };
struct AudioSourceProcs Au_sourceProcs = { 0 };

const struct AudioDriver *Au_driver = NULL;

#ifndef AUDIO_DRIVER_BUILTIN
static void *_drvModule = NULL;
#endif

bool
Au_Init(void)
{
	uint32_t devCount = 0;
	struct AudioDeviceInfo *info = NULL, *selected = NULL;

#ifdef AUDIO_DRIVER_BUILTIN
	Au_driver = Au_LoadBuiltinDriver();
#else
	AuLoadDriverProc loadDriver;
	const char *drvPath = E_GetCVarStr(L"Audio_Driver", "OpenALDriver")->str;
	_drvModule = Sys_LoadLibrary(drvPath);
	CHK_FAIL(_drvModule, L"Failed to load driver module");

	loadDriver = Sys_GetProcAddress(_drvModule, "Au_LoadDriver");
	CHK_FAIL(loadDriver, L"The library is not a valid driver");

	Au_driver = loadDriver();
#endif
	
	CHK_FAIL(Au_driver, L"Failed to load driver");
	CHK_FAIL((Au_driver->identifier == NE_AUDIO_DRIVER_ID), L"The library is not a valid driver");
	CHK_FAIL((Au_driver->apiVersion == NE_AUDIO_DRIVER_API), L"Driver version mismatch");
	CHK_FAIL(Au_driver->Init(), L"Failed to initialize driver");
	CHK_FAIL(Au_driver->EnumerateDevices(&devCount, NULL), L"Failed to enumerate devices");

	info = Sys_Alloc(sizeof(*info), devCount, MH_Transient);
	CHK_FAIL(info, L"Failed to enumerate devices");
	CHK_FAIL(Au_driver->EnumerateDevices(&devCount, info), L"Failed to enumerate devices");

	/*uint64_t vramSize = 0;
	bool haveRt = false;
	for (uint32_t i = 0; i < devCount; ++i) {
		if (!info[i].features.canPresent)
			continue;
		
		if (info[i].features.rayTracing) {
			if (!haveRt || (haveRt && vramSize < info[i].localMemorySize))
				goto updateSelection;
		} else {
			if (vramSize < info[i].localMemorySize)
				goto updateSelection;
		}
		
		continue;
		
	updateSelection:
		selected = &info[i];
		vramSize = info[i].localMemorySize;
		haveRt = info[i].features.rayTracing;
	}
	
	CHK_FAIL(selected, L"No suitable device found");*/
	selected = &info[0];
	
	memcpy(&Au_deviceInfo, selected, sizeof(Au_deviceInfo));
	
	Au_device = Au_driver->CreateDevice(&Au_deviceInfo, &Au_deviceProcs, &Au_sourceProcs);
	CHK_FAIL(Au_device, L"Failed to create device");

	return true;

/*	E_RegisterResourceType(RES_AUDIO_CLIP, sizeof(struct AudioClip) - sizeof(uint8_t) + Au_audioClipDataSize,
		(ResourceCreateProc)Au_CreateClip, (ResourceLoadProc)Au_LoadClip, (ResourceUnloadProc)Au_UnloadClip);

	return E_RegisterComponent(AUDIO_SOURCE_COMP, Au_sourceSize, 1,
		(CompInitProc)Au_InitSourceComponent, (CompTermProc)Au_TermSourceComponent);*/
}

/*void
Au_Update(void)
{
//	X3DAudioCalculate(_au3DAudio, &_auListener, )
}*/

void
Au_Term(void)
{
//	Au_TermLib();
}
