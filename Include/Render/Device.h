#ifndef _NE_RENDER_DRIVER_DEVICE_H_
#define _NE_RENDER_DRIVER_DEVICE_H_

#include <Render/Types.h>

#define	RE_VENDOR_ID_NVIDIA			0x10DE
#define RE_VENDOR_ID_ATI_AMD		0x1002
#define RE_VENDOR_ID_INTEL			0x8086
#define RE_VENDOR_ID_S3				0x5333
#define RE_VENDOR_ID_MATROX			0x102B
#define RE_VENDOR_ID_3DLABS			0x3D3D
#define RE_VENDOR_ID_APPLE			0x106B
#define RE_VENDOR_ID_ARM			0x13B5
#define RE_VENDOR_ID_QUALCOMM		0x5143
#define RE_VENDOR_ID_IMAGINATION	0x1010
#define RE_VENDOR_ID_NEC			0x1033
#define RE_VENDOR_ID_STMICRO		0x104A
#define RE_VENDOR_ID_BROADCOM		0x14E4

struct NeRenderDeviceInfo
{
	char deviceName[256];
	uint64_t localMemorySize;

	struct {
		bool unifiedMemory;
		bool rayTracing;
		bool indirectRayTracing;
		bool meshShading;
		bool discrete;
		bool canPresent;
		bool drawIndirectCount;
		bool bcTextureCompression;
		bool astcTextureCompression;
		bool multiDrawIndirect;
		bool secondaryCommandBuffers;
		bool coherentMemory;
		bool directIO;
	} features;

	struct {
		uint32_t maxTextureSize;
	} limits;

	struct {
		uint32_t vendorId;
		uint32_t deviceId;
		uint32_t driverVersion;
	} hardwareInfo;

	void *private;
};

ENGINE_API extern struct NeRenderDevice *Re_device;
ENGINE_API extern struct NeRenderDeviceInfo Re_deviceInfo;

struct NeRenderContext *Re_CreateContext(void);
void Re_ResetContext(struct NeRenderContext *ctx);
void Re_DestroyContext(struct NeRenderContext *ctx);

struct NeSurface *Re_CreateSurface(void *window);
void Re_DestroySurface(struct NeSurface *surface);

void Re_WaitIdle(void);

uint64_t Re_OffsetAddress(uint64_t addr, uint64_t offset);

void *Re_XrGraphicsBinding(void);

#endif /* _NE_RENDER_DRIVER_DEVICE_H_ */
