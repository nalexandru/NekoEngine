#ifndef _RE_TRANSIENT_RESOURCES_H_
#define _RE_TRANSIENT_RESOURCES_H_

#include <Render/Types.h>
#include <Render/Device.h>

static inline bool Re_InitTransientHeap(uint64_t size) { return Re_deviceProcs.InitTransientHeap(Re_device, size); }
static inline bool Re_ResizeTransientHeap(uint64_t size) { return Re_deviceProcs.ResizeTransientHeap(Re_device, size); }

static inline struct Texture *Re_CreateTransientTexture(const struct TextureCreateInfo *tci, uint64_t offset)
{ return Re_deviceProcs.CreateTransientTexture(Re_device, tci, offset); }
static inline struct Buffer *Re_CreateTransientBuffer(const struct BufferCreateInfo *bci, uint64_t offset)
{ return Re_deviceProcs.CreateTransientBuffer(Re_device, bci, offset); }

static inline void Re_TermTransientHeap(void) { Re_deviceProcs.TermTransientHeap(Re_device); }

#endif /* _RE_TRANSIENT_RESOURCES_H_ */
