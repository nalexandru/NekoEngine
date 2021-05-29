#include <assert.h>

#include <Render/Render.h>
#include <Runtime/Runtime.h>

struct BufferInfo
{
	struct Buffer *buff;
	struct BufferDesc desc;
};

static uint16_t _nextId;
static struct BufferInfo _buffers[UINT16_MAX];
static struct Queue _freeList;

bool
Re_CreateBuffer(const struct BufferCreateInfo *bci, BufferHandle *handle)
{
	if (!Re_ReserveBufferId(handle))
		return false;

	struct BufferInfo *info = &_buffers[*handle];

	info->buff = Re_deviceProcs.CreateBuffer(Re_device, bci, *handle);
	if (!info->buff)
		return false;

	memcpy(&info->desc, &bci->desc, sizeof(info->desc));

	return true;
}

void
Re_UpdateBuffer(BufferHandle handle, uint64_t offset, uint8_t *data, uint64_t size)
{
	Re_deviceProcs.UpdateBuffer(Re_device, _buffers[handle].buff, offset, data, size);
}

void
Re_CmdCopyBuffer(BufferHandle src, uint64_t srcOffset, BufferHandle dst, uint64_t dstOffset, uint64_t size)
{
	Re_contextProcs.CopyBuffer(Re_CurrentContext(), _buffers[src].buff, srcOffset, _buffers[dst].buff, dstOffset, size);
}

void *
Re_MapBuffer(BufferHandle handle)
{
	return Re_deviceProcs.MapBuffer(Re_device, _buffers[handle].buff);
}

void
Re_UnmapBuffer(BufferHandle handle)
{
	Re_deviceProcs.UnmapBuffer(Re_device, _buffers[handle].buff);
}

uint64_t
Re_BufferAddress(BufferHandle handle, uint64_t offset)
{
	return Re_deviceProcs.BufferAddress(Re_device, _buffers[handle].buff, offset);
}

const struct BufferDesc *
Re_BufferDesc(BufferHandle handle)
{
	return &_buffers[handle].desc;
}

void
Re_DestroyBuffer(BufferHandle handle)
{
	Re_deviceProcs.DestroyBuffer(Re_device, _buffers[handle].buff);
	Re_ReleaseBufferId(handle);
}

bool
Re_ReserveBufferId(BufferHandle *handle)
{
	if (_freeList.count)
		*handle = *(BufferHandle *)Rt_QueuePop(&_freeList);
	else if (_nextId < UINT16_MAX)
		*handle = _nextId++;
	else
		return false;

	return true;
}

void
Re_ReleaseBufferId(BufferHandle handle)
{
	Rt_QueuePush(&_freeList, &handle);
}

void
Re_CmdBindIndexBuffer(BufferHandle handle, uint64_t offset, enum IndexType type)
{
	Re_contextProcs.BindIndexBuffer(Re_CurrentContext(), _buffers[handle].buff, offset, type);
}

bool
Re_InitBufferSystem(void)
{
	return Rt_InitQueue(&_freeList, UINT16_MAX, sizeof(BufferHandle), MH_Render);
}

void
Re_TermBufferSystem(void)
{
	Rt_TermQueue(&_freeList);
}
