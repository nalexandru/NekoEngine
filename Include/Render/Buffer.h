#ifndef _RE_BUFFER_H_
#define _RE_BUFFER_H_

#include <Render/Types.h>
#include <Render/Device.h>

struct BufferDesc
{
	uint64_t size;
	enum BufferUsage usage;
	enum GPUMemoryType memoryType;
};

struct BufferCreateInfo
{
	struct BufferDesc desc;
	void *data;
	uint64_t dataSize;
	bool keepData, dedicatedAllocation;
};

bool Re_CreateBuffer(const struct BufferCreateInfo *bci, BufferHandle *handle);
void Re_UpdateBuffer(BufferHandle id, uint64_t offset, uint8_t *data, uint64_t size);
const struct BufferDesc *Re_BufferDesc(BufferHandle id);
void Re_DestroyBuffer(BufferHandle id);

//static inline struct Buffer *Re_CreateBuffer(const struct BufferCreateInfo *bci) { return Re_deviceProcs.CreateBuffer(Re_device, bci); };
//static inline void Re_UpdateBuffer(struct Buffer *buff, uint64_t offset, uint8_t *data, uint64_t size) { Re_deviceProcs.UpdateBuffer(Re_device, buff, offset, data, size); }
//static inline const struct BufferDesc *Re_BufferDesc(const struct Buffer *buff) { return Re_deviceProcs.BufferDesc(buff); }
//static inline void Re_DestroyBuffer(struct Buffer *buff) { Re_deviceProcs.DestroyBuffer(Re_device, buff); }

#endif /* _RE_BUFFER_H_ */
