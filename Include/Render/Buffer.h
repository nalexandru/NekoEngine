#ifndef _RE_BUFFER_H_
#define _RE_BUFFER_H_

#include <Engine/Types.h>
#include <Render/Device.h>

enum BufferUsage
{
	RE_VERTEX_BUFFER,
	RE_INDEX_BUFFER,
	RE_UNIFORM_BUFFER,
	RE_STORAGE_BUFFER
};

struct BufferDesc
{
	uint64_t size;
	enum BufferUsage usage;
};

struct BufferCreateInfo
{
	struct Buffer desc;
	void *data;
	uint64_t dataSize;
	bool keepData;
};

static inline struct Buffer *Re_CreateBuffer(const struct RenderDevice *dev, const struct BufferCreateInfo *bci) { return Re_DeviceProcs.CreateBuffer(dev, bci); };
static inline void Re_DestroyBuffer(const struct RenderDevice *dev, struct Buffer *buff) { Re_DeviceProcs.DestroyBuffer(dev, buff); }

//static inline void Re_UpdateTexture(struct Texture *tex, uint64_t offset, uint64_t size, void *data);

static inline const struct BufferDesc *Re_BufferDesc(const struct Buffer *buff) { return NULL; }

#endif /* _RE_BUFFER_H_ */
