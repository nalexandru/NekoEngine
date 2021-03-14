#ifndef _RE_BUFFER_H_
#define _RE_BUFFER_H_

#include <Engine/Types.h>
#include <Render/Device.h>
#include <Render/Memory.h>

enum BufferUsage
{
	BU_TRANSFER_SRC			= 0x00000001,
	BU_TRANSFER_DST			= 0x00000002,
	BU_UNIFORM_BUFFER		= 0x00000010,
	BU_STORAGE_BUFFER		= 0x00000020,
	BU_INDEX_BUFFER			= 0x00000040,
	BU_INDIRECT_BUFFER		= 0x00000100,
	BU_AS_BUILD_INPUT		= 0x00080000,
	BU_AS_STORAGE			= 0x00100000,
	BU_SHADER_BINDING_TABLE	= 0x00000400
};

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

static inline struct Buffer *Re_CreateBuffer(const struct BufferCreateInfo *bci) { return Re_deviceProcs.CreateBuffer(Re_device, bci); };
static inline void Re_UpdateBuffer(struct Buffer *buff, uint64_t offset, uint8_t *data, uint64_t size) { Re_deviceProcs.UpdateBuffer(Re_device, buff, offset, data, size); }
static inline const struct BufferDesc *Re_BufferDesc(const struct Buffer *buff) { return Re_deviceProcs.BufferDesc(buff); }
static inline void Re_DestroyBuffer(struct Buffer *buff) { Re_deviceProcs.DestroyBuffer(Re_device, buff); }

#endif /* _RE_BUFFER_H_ */
