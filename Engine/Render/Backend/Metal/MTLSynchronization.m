#include "MTLBackend.h"

struct NeSemaphore *
Re_CreateSemaphore(void)
{
	struct NeSemaphore *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);
	if (!s)
		return NULL;

	s->event = [MTL_device newEvent];
	s->value = 0;

	return s;
}

bool
Re_WaitSemaphore(struct NeSemaphore *s, uint64_t value, uint64_t timeout)
{
	return false;
}

bool
Re_WaitSemaphores(uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout)
{
	return false;
}

bool
Re_SignalSemaphore(struct NeSemaphore *s, uint64_t value)
{
	return false;
}

void
Re_DestroySemaphore(struct NeSemaphore *s)
{
	[s->event release];
	Sys_Free(s);
}

struct NeFence *
Re_CreateFence(bool createSignaled)
{
	dispatch_semaphore_t ds = dispatch_semaphore_create(RE_NUM_FRAMES);
	
	if (createSignaled)
		dispatch_semaphore_signal(ds);
	
	return (struct NeFence *)ds;
}

void
Re_SignalFence(struct NeRenderDevice *dev, struct NeFence *f)
{
	dispatch_semaphore_signal((dispatch_semaphore_t)f);
}

bool
Re_WaitForFence(struct NeFence *f, uint64_t timeout)
{
	return dispatch_semaphore_wait((dispatch_semaphore_t)f, timeout) == 0;
}


void
Re_DestroyFence(struct NeFence *f)
{
	dispatch_release((dispatch_semaphore_t)f);
}
