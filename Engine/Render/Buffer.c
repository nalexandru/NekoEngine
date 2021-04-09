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
	if (_freeList.count)
		*handle = *(uint16_t *)Rt_QueuePop(&_freeList);
	else if (_nextId < UINT16_MAX)
		*handle = _nextId++;
	else
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

const struct BufferDesc *
Re_BufferDesc(BufferHandle handle)
{
	return &_buffers[handle].desc;
}

void
Re_DestroyBuffer(BufferHandle handle)
{
	Re_deviceProcs.DestroyBuffer(Re_device, _buffers[handle].buff);
	Rt_QueuePush(&_freeList, &handle);
}

void
Re_BindIndexBuffer(BufferHandle handle, uint64_t offset, enum IndexType type)
{
	Re_contextProcs.BindIndexBuffer(Re_CurrentContext(), _buffers[handle].buff, offset, type);
}

bool
Re_InitBuffers(void)
{
	return Rt_InitQueue(&_freeList, UINT16_MAX, sizeof(struct BufferInfo));
}

void
Re_TermBuffers(void)
{
	Rt_TermQueue(&_freeList);
}

