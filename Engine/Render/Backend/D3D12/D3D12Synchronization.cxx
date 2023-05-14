#include "D3D12Backend.h"

struct NeSemaphore *
Re_CreateSemaphore(void)
{
	struct NeSemaphore *s = (struct NeSemaphore *)Sys_Alloc(sizeof(*s), 1, MH_RenderBackend);
	if (!s)
		return NULL;

	if (!D3D12Bk_InitFence(&s->df, false)) {
		Sys_Free(s);
		return nullptr;
	}

	return s;
}

bool
Re_WaitSemaphore(struct NeSemaphore *s, uint64_t value, uint64_t timeout)
{
	return D3D12Bk_WaitForFenceCPUExplicit(&s->df, (DWORD)timeout, value);
}

bool
Re_WaitSemaphores(uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout)
{
	for (uint32_t i = 0; i < count; ++i)
		if (!D3D12Bk_WaitForFenceCPUExplicit(&s[i].df, (DWORD)timeout, values[i]))
			return false;
	return true;
}

bool
Re_SignalSemaphore(struct NeSemaphore *s, uint64_t value)
{
	return SUCCEEDED(D3D12Bk_SignalFenceCPU(&s->df));
}

void
Re_DestroySemaphore(struct NeSemaphore *s)
{
	D3D12Bk_TermFence(&s->df);
	Sys_Free(s);
}

struct NeFence *
Re_CreateFence(bool createSignaled)
{
	NeFence *f = (NeFence *)Sys_Alloc(sizeof(*f), 1, MH_RenderBackend);
	if (!f)
		return NULL;

	if (!D3D12Bk_InitFence(&f->df, createSignaled)) {
		Sys_Free(f);
		return nullptr;
	}

	return f;
}

void
Re_SignalFence(struct NeFence *f)
{
	D3D12Bk_SignalFenceCPU(&f->df);
}

bool
Re_WaitForFence(struct NeFence *f, uint64_t timeout)
{
	return D3D12Bk_WaitForFenceCPU(&f->df, (DWORD)timeout);
}

void
Re_DestroyFence(struct NeFence *f)
{
	D3D12Bk_TermFence(&f->df);
	Sys_Free(f);
}

/* NekoEngine
 *
 * D3D12Synchronization.cxx
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
