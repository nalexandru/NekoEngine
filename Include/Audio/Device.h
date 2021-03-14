#ifndef _AU_DEVICE_H_
#define _AU_DEVICE_H_

#include <Engine/Types.h>
#include <Render/Context.h>

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

struct AudioDeviceProcs
{
	void *a;



};

extern struct AudioDevice *Au_device;
extern struct AudioDeviceInfo Au_deviceInfo;
extern struct AudioDeviceProcs Au_deviceProcs;

#endif /* _RE_DEVICE_H_ */
