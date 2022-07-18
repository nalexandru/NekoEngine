#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <Render/Render.h>

#include "MTLBackend.h"

#define MTL_BACKEND	"MetalBackend"

const char *Re_backendName = "Metal";

bool
Re_InitBackend(void)
{
	return MTLDrv_InitMemory();
}

void
Re_TermBackend(void)
{
	MTLDrv_TermMemory();
}

NeDirectIOHandle
Re_BkOpenFile(const char *path)
{
	if (@available(macOS 13, iOS 16, *)) {
		NSError *err;
		id<MTLIOFileHandle> handle = [Re_device->dev newIOHandleWithURL: [NSURL fileURLWithPath: [NSString stringWithUTF8String: path]] error: &err];
		if (!handle)
			Sys_LogEntry(MTL_BACKEND, LOG_CRITICAL, "Failed to open file [%s]: ", path, [[err description] UTF8String]);
		return handle;
	}

	return NULL;
}

void
Re_BkCloseFile(NeDirectIOHandle handle)
{
}

#if TARGET_OS_OSX

bool
Re_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *info)
{
	if (!count)
		return false;
	
	NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
	
	if (!*count || !info) {
		for (NSUInteger i = 0; i < [devices count]; ++i) {
			if ([devices[0] supportsFamily: MTLGPUFamilyMac2])
				++*count;
		}
		[devices release];
		return true;
	}
	
	for (uint32_t i = 0; i < *count; ++i) {
		id<MTLDevice> dev = devices[i];
		if (![dev supportsFamily: MTLGPUFamilyMac2])
			continue;
		
		snprintf(info[i].deviceName, sizeof(info[i].deviceName), "%s", [[dev name] UTF8String]);
		info[i].localMemorySize = [dev recommendedMaxWorkingSetSize];

		info[i].features.rayTracing = [dev supportsRaytracing];
		info[i].features.indirectRayTracing = false;
		info[i].features.unifiedMemory = [dev hasUnifiedMemory];
		info[i].features.discrete = ![dev isLowPower];
		info[i].features.canPresent = ![dev isHeadless];
		info[i].limits.maxTextureSize = 16384;
		info[i].features.drawIndirectCount = false;
		info[i].features.bcTextureCompression = true;
		info[i].features.astcTextureCompression = false;
		info[i].features.secondaryCommandBuffers = false;
		info[i].features.coherentMemory = true;

		if (@available(macOS 13, *)) {
			info[i].features.directIO = true;
			info[i].features.meshShading = true;
		} else {
			info[i].features.directIO = false;
			info[i].features.meshShading = false;
		}

		if ([dev supportsFamily: MTLGPUFamilyApple5]) {
			// Apple GPU
			info->hardwareInfo.vendorId = 0x106B;

			/*if ([dev supportsFamily: MTLGPUFamilyApple8])
				info->hardwareInfo.deviceId = 0xA150;
			else*/ if ([dev supportsFamily: MTLGPUFamilyApple7])
				info->hardwareInfo.deviceId = 0xA140;
			else if ([dev supportsFamily: MTLGPUFamilyApple6])
				info->hardwareInfo.deviceId = 0xA130;
			else
				info->hardwareInfo.deviceId = 0xA120;
		} else {
			// TODO
		}

		info[i].private = (void *)devices[i];
	}
	
	[devices release];
	
	return true;
}

#else

bool
Re_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *info)
{
	if (!count)
		return false;
	
	id<MTLDevice> dev = MTLCreateSystemDefaultDevice();
	
	if (!*count || !info) {
		*count = [dev supportsFamily: MTLGPUFamilyApple6] ? 1 : 0;
		[dev release];
		return true;
	}
	
	snprintf(info->deviceName, sizeof(info->deviceName), "%s", [[dev name] UTF8String]);
	info->localMemorySize = 1024 * 1024 * 1024;

	info->features.rayTracing = [dev supportsRaytracing];
	info->features.indirectRayTracing = false;
	info->features.unifiedMemory = [dev hasUnifiedMemory];
	info->features.discrete = false;
	info->features.canPresent = true;
	info->features.drawIndirectCount = false;
	info->features.bcTextureCompression = false;
	info->features.astcTextureCompression = true;
	info->features.secondaryCommandBuffers = false;
	info->limits.maxTextureSize = 16384;
	info->features.coherentMemory = true;

	info->hardwareInfo.vendorId = 0x106B;

	if (@available(iOS 16, *)) {
		info[i].features.directIO = true;
		info[i].features.meshShading = true;
	} else {
		info[i].features.directIO = false;
		info[i].features.meshShading = false;
	}

	if ([dev supportsFamily: MTLGPUFamilyApple8])
		info->hardwareInfo.deviceId = 0xA150;
	else if ([dev supportsFamily: MTLGPUFamilyApple7])
		info->hardwareInfo.deviceId = 0xA140;
	else
		info->hardwareInfo.deviceId = 0xA130;

	info->private = dev;

	return true;
}

#endif
