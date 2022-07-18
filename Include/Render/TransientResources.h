#ifndef _NE_RENDER_DRIVER_TRANSIENT_RESOURCES_H_
#define _NE_RENDER_DRIVER_TRANSIENT_RESOURCES_H_

#include <Render/Types.h>
#include <Render/Device.h>

bool Re_InitTransientHeap(uint64_t size);
bool Re_ResizeTransientHeap(uint64_t size);

struct NeTexture *Re_CreateTransientTexture(const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct NeBuffer *Re_CreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location, uint64_t offset, uint64_t *size);

void Re_TermTransientHeap(void);

#endif /* _NE_RENDER_DRIVER_TRANSIENT_RESOURCES_H_ */
