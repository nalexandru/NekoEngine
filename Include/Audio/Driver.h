#ifndef _NE_AUDIO_DRIVER_H_
#define _NE_AUDIO_DRIVER_H_

#include <Engine/Types.h>

#define NE_AUDIO_DRIVER_ID		0x0B16D1C5
#define NE_AUDIO_DRIVER_API		1

struct NeAudioDriver 
{
	uint32_t identifier;
	uint32_t apiVersion;
	char driverName[64];
	
	bool (*Init)(void);
	void (*Update)(void);
	void (*Term)(void);
	
	bool (*EnumerateDevices)(uint32_t *count, struct NeAudioDeviceInfo *devices);
	struct NeAudioDevice *(*CreateDevice)(struct NeAudioDeviceInfo *info,
										 struct NeAudioDeviceProcs *devProcs,
										 struct NeAudioSourceProcs *srcProcs);
	void (*DestroyDevice)(struct NeAudioDevice *dev);
};

#ifdef AUDIO_DRIVER_BUILTIN
const struct NeAudioDriver *Au_LoadBuiltinDriver(void);
#endif
typedef const struct NeAudioDriver *(*AuLoadDriverProc)(void);

ENGINE_API extern const struct NeAudioDriver *Au_driver;

#endif /* _NE_AUDIO_DRIVER_H_ */
