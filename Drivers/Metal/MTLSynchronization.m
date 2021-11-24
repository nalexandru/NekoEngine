#include "MTLDriver.h"

id<MTLFence>
MTL_CreateSemaphore(id<MTLDevice> dev)
{
	return [dev newFence];
}

/*void
MTL_SignalSemaphore(id<MTLDevice> dev, dispatch_semaphore_t s)
{
	(void)dev;
	dispatch_semaphore_signal(s);
}

bool
MTL_WaitSemaphore(id<MTLDevice> dev, dispatch_semaphore_t s, uint64_t timeout)
{
	(void)dev;
	return dispatch_semaphore_wait(s, timeout) == 0;
}*/

bool
MTL_WaitSemaphore(id<MTLDevice> dev, id<MTLFence> f, uint64_t value, uint64_t timeout)
{
	return false;
}

bool
MTL_WaitSemaphores(id<MTLDevice> dev, uint32_t count, id<MTLFence> *f, uint64_t *values, uint64_t timeout)
{
	return false;
}

bool
MTL_SignalSemaphore(id<MTLDevice> dev, id<MTLFence> f, uint64_t value)
{
	return false;
}

void
MTL_DestroySemaphore(id<MTLDevice> dev, id<MTLFence> fence)
{
	(void)dev;
	[fence release];
}

dispatch_semaphore_t
MTL_CreateFence(id<MTLDevice> dev, bool createSignaled)
{
	(void)dev;
	dispatch_semaphore_t ds = dispatch_semaphore_create(RE_NUM_FRAMES);
	
	if (createSignaled)
		dispatch_semaphore_signal(ds);
	
	return ds;
}

void
MTL_SignalFence(id<MTLDevice> dev, dispatch_semaphore_t ds)
{
	(void)dev;
	dispatch_semaphore_signal(ds);
}

bool
MTL_WaitForFence(id<MTLDevice> dev, dispatch_semaphore_t ds, uint64_t timeout)
{
	(void)dev;
	return dispatch_semaphore_wait(ds, timeout) == 0;
}


void
MTL_DestroyFence(id<MTLDevice> dev, dispatch_semaphore_t ds)
{
	(void)dev;
//	dispatch_release(ds);
}
