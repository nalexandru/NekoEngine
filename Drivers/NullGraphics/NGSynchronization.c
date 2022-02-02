#include "NullGraphicsDriver.h"

struct NeSemaphore *
NG_CreateSemaphore(struct NeRenderDevice *dev)
{
	struct NeSemaphore *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);
	if (!s)
		return NULL;
	return s;
}

bool
NG_WaitSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value, uint64_t timeout)
{
	return true;
}

bool
NG_WaitSemaphores(struct NeRenderDevice *dev, uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout)
{
	return true;
}

bool
NG_SignalSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value)
{
	return true;
}

void
NG_DestroySemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s)
{
	Sys_Free(s);
}

struct NeFence *
NG_CreateFence(struct NeRenderDevice *dev, bool createSignaled)
{
	struct NeFence *f = Sys_Alloc(sizeof(*f), 1, MH_RenderDriver);
	if (!f)
		return NULL;
	return f;
}

void
NG_SignalFence(struct NeRenderDevice *dev, struct NeFence *f)
{
}

bool
NG_WaitForFence(struct NeRenderDevice *dev, struct NeFence *f, uint64_t timeout)
{
	return true;
}

void
NG_DestroyFence(struct NeRenderDevice *dev, struct NeFence *f)
{
	Sys_Free(f);
}
