#ifndef _NE_AUDIO_DEVICE_H_
#define _NE_AUDIO_DEVICE_H_

#include <Engine/Types.h>

struct AudioDeviceInfo
{
	char deviceName[256];
	
	/*struct {
		bool unifiedMemory;
		bool rayTracing;
		bool indirectRayTracing;
		bool meshShading;
		bool discrete;
		bool canPresent;
		bool drawIndirectCount;
		bool textureCompression;
	} features;
	
	struct {
		uint32_t maxTextureSize;
	} limits;*/
		
	void *private;
};

extern struct AudioDeviceInfo Au_deviceInfo;

#endif /* _NE_AUDIO_DEVICE_H_ */
