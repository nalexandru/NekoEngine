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

	info->buff = Re_deviceProcs.CreateBuffer(Re_device, &bci->desc, *handle);
	if (!info->buff)
		return false;

	memcpy(&info->desc, &bci->desc, sizeof(info->desc));
	
	if (bci->data) {
		Re_UpdateBuffer(*handle, 0, bci->data, bci->dataSize);
		if (!bci->keepData)
			Sys_Free(bci->data);
	}
	
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
Re_FlushBuffer(BufferHandle handle, uint64_t offset, uint64_t size)
{
	Re_deviceProcs.FlushBuffer(Re_device, _buffers[handle].buff, offset, size);
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

struct Buffer *
Re_CreateTransientBuffer(const struct BufferDesc *desc, BufferHandle location, uint64_t offset, uint64_t *size)
{
	_buffers[location].desc = *desc;
	_buffers[location].buff = Re_deviceProcs.CreateTransientBuffer(Re_device, desc, location, offset, size);

	return _buffers[location].buff;
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

void
Re_CmdCopyBufferToTexture(BufferHandle src, struct Texture *dst, const struct BufferImageCopy *bic)
{
	Re_contextProcs.CopyBufferToTexture(Re_CurrentContext(), _buffers[src].buff, dst, bic);
}

void
Re_CmdCopyTextureToBuffer(struct Texture *src, BufferHandle dst, const struct BufferImageCopy *bic)
{
	Re_contextProcs.CopyTextureToBuffer(Re_CurrentContext(), src, _buffers[dst].buff, bic);
}
