#include <Render/Render.h>
#include <Runtime/Runtime.h>

struct NeBufferInfo
{
	struct NeBuffer *buff;
	struct NeBufferDesc desc;
};

static uint16_t _nextId;
static struct NeBufferInfo _buffers[UINT16_MAX];
static struct NeQueue _freeList;

bool
Re_CreateBuffer(const struct NeBufferCreateInfo *bci, NeBufferHandle *handle)
{
	if (!Re_ReserveBufferId(handle))
		return false;

	struct NeBufferInfo *info = &_buffers[*handle];

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
Re_UpdateBuffer(NeBufferHandle handle, uint64_t offset, uint8_t *data, uint64_t size)
{
	Re_deviceProcs.UpdateBuffer(Re_device, _buffers[handle].buff, offset, data, size);
}

void
Re_CmdCopyBuffer(NeBufferHandle src, uint64_t srcOffset, NeBufferHandle dst, uint64_t dstOffset, uint64_t size)
{
	Re_contextProcs.CopyBuffer(Re_CurrentContext(), _buffers[src].buff, srcOffset, _buffers[dst].buff, dstOffset, size);
}

void *
Re_MapBuffer(NeBufferHandle handle)
{
	return Re_deviceProcs.MapBuffer(Re_device, _buffers[handle].buff);
}

void
Re_FlushBuffer(NeBufferHandle handle, uint64_t offset, uint64_t size)
{
	Re_deviceProcs.FlushBuffer(Re_device, _buffers[handle].buff, offset, size);
}

void
Re_UnmapBuffer(NeBufferHandle handle)
{
	Re_deviceProcs.UnmapBuffer(Re_device, _buffers[handle].buff);
}

uint64_t
Re_BufferAddress(NeBufferHandle handle, uint64_t offset)
{
	return Re_deviceProcs.BufferAddress(Re_device, _buffers[handle].buff, offset);
}

const struct NeBufferDesc *
Re_BufferDesc(NeBufferHandle handle)
{
	return &_buffers[handle].desc;
}

void
Re_DestroyBuffer(NeBufferHandle handle)
{
	Re_deviceProcs.DestroyBuffer(Re_device, _buffers[handle].buff);
	Re_ReleaseBufferId(handle);
}

bool
Re_ReserveBufferId(NeBufferHandle *handle)
{
	if (_freeList.count) {
		*handle = *(NeBufferHandle*)Rt_QueuePop(&_freeList);
	} else if (_nextId < UINT16_MAX) {
		*handle = _nextId++;
	} else {
		*handle = (uint16_t)-1;
		return false;
	}

	return true;
}

void
Re_ReleaseBufferId(NeBufferHandle handle)
{
	Rt_QueuePush(&_freeList, &handle);
}

void
Re_CmdBindIndexBuffer(NeBufferHandle handle, uint64_t offset, enum NeIndexType type)
{
	Re_contextProcs.BindIndexBuffer(Re_CurrentContext(), _buffers[handle].buff, offset, type);
}

struct NeBuffer *
Re_CreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location, uint64_t offset, uint64_t *size)
{
	_buffers[location].desc = *desc;
	_buffers[location].buff = Re_deviceProcs.CreateTransientBuffer(Re_device, desc, location, offset, size);

	return _buffers[location].buff;
}

bool
Re_InitBufferSystem(void)
{
	return Rt_InitQueue(&_freeList, UINT16_MAX, sizeof(NeBufferHandle), MH_Render);
}

void
Re_TermBufferSystem(void)
{
	Rt_TermQueue(&_freeList);
}

void
Re_CmdCopyBufferToTexture(NeBufferHandle src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
	Re_contextProcs.CopyBufferToTexture(Re_CurrentContext(), _buffers[src].buff, dst, bic);
}

void
Re_CmdCopyTextureToBuffer(struct NeTexture *src, NeBufferHandle dst, const struct NeBufferImageCopy *bic)
{
	Re_contextProcs.CopyTextureToBuffer(Re_CurrentContext(), src, _buffers[dst].buff, bic);
}
