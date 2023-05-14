#include "MTLBackend.h"

id<MTLDevice> MTL_device = nil;

struct NeRenderDevice *
Re_CreateDevice(struct NeRenderDeviceInfo *info)
{
	MTL_device = (id<MTLDevice>)info->reserved;

	MTL_InitLibrary((id<MTLDevice>)info->reserved);
	MTL_InitArgumentBuffer((id<MTLDevice>)info->reserved);

	return info->reserved;
}

bool
Re_Execute(id<MTLDevice> dev, struct NeRenderContext *ctx, bool wait)
{
	return false;
}

void
Re_WaitIdle(void)
{
	dispatch_semaphore_wait(MTL_frameSemaphore, dispatch_time(DISPATCH_TIME_NOW, 20000000));

	id<MTLEvent> event = [MTL_device newEvent];
	uint64_t value = 0;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i) {
		id<MTLCommandBuffer> cb = [Re_contexts[i]->queue commandBufferWithUnretainedReferences];
		[cb encodeSignalEvent: event value: ++value];
		[cb commit];
	}

	id<MTLCommandBuffer> cb = [Re_contexts[0]->queue commandBufferWithUnretainedReferences];
	[cb encodeWaitForEvent: event value: value];

	__block dispatch_semaphore_t bds = dispatch_semaphore_create(1);
	[cb addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull cmdBuff) {
		dispatch_semaphore_signal(bds);
	}];
	[cb commit];

	dispatch_semaphore_wait(bds, DISPATCH_TIME_FOREVER);
}

void
Re_DestroyDevice(struct NeRenderDevice *dev)
{
	MTL_TermArgumentBuffer((id<MTLDevice>)dev);
	MTL_TermLibrary();
	[(id<MTLDevice>)dev release];
}

/* NekoEngine
 *
 * MTLDevice.m
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
