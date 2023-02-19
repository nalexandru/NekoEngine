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

/* NekoEngine
 *
 * MTLSynchronization.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
