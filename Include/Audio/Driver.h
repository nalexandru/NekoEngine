#ifndef _AU_DRIVER_H_
#define _AU_DRIVER_H_

#include <Engine/Types.h>

#define NE_AUDIO_DRIVER_ID		0x0B16D1C5
#define NE_AUDIO_DRIVER_API		1

struct AudioDriver 
{
	uint32_t identifier;
	uint32_t apiVersion;
	wchar_t driverName[64];
	
	bool (*Init)(void);
	void (*Update)(void);
	void (*Term)(void);
	
	bool (*EnumerateDevices)(uint32_t *count, struct AudioDeviceInfo *devices);
	struct AudioDevice *(*CreateDevice)(struct AudioDeviceInfo *info,
										 struct AudioDeviceProcs *devProcs,
										 struct AudioSourceProcs *srcProcs);
	void (*DestroyDevice)(struct AudioDevice *dev);
};

#ifdef AUDIO_DRIVER_BUILTIN
const struct AudioDriver *Au_LoadBuiltinDriver(void);
#endif
typedef const struct AudioDriver *(*AuLoadDriverProc)(void);

ENGINE_API extern const struct AudioDriver *Au_driver;

#endif
