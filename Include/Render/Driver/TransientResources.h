#ifndef _NE_RENDER_DRIVER_TRANSIENT_RESOURCES_H_
#define _NE_RENDER_DRIVER_TRANSIENT_RESOURCES_H_

#include <Render/Types.h>
#include <Render/Driver/Device.h>

static inline bool Re_InitTransientHeap(uint64_t size) { return Re_deviceProcs.InitTransientHeap(Re_device, size); }
static inline bool Re_ResizeTransientHeap(uint64_t size) { return Re_deviceProcs.ResizeTransientHeap(Re_device, size); }

static inline struct NeTexture *Re_CreateTransientTexture(const struct NeTextureDesc *desc, uint64_t offset, uint64_t *size)
{ return Re_deviceProcs.CreateTransientTexture(Re_device, desc, 0, offset, size); }
struct NeBuffer *Re_CreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location, uint64_t offset, uint64_t *size);

static inline void Re_TermTransientHeap(void) { Re_deviceProcs.TermTransientHeap(Re_device); }

#endif /* _NE_RENDER_DRIVER_TRANSIENT_RESOURCES_H_ */
