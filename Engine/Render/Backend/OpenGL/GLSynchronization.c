#include "GLBackend.h"

struct NeSemaphore *Re_CreateSemaphore(void) { return (struct NeSemaphore *)1; }

bool Re_SignalSemaphore(struct NeSemaphore *s, uint64_t value) { return true; }
void Re_DestroySemaphore(struct NeSemaphore *s) { }

struct NeFence *Re_CreateFence(bool createSignaled) { return (struct NeFence *)1; }
void Re_SignalFence(struct NeRenderDevice *dev, struct NeFence *f) { }
void Re_DestroyFence(struct NeFence *f) { }

bool
Re_WaitForFence(struct NeFence *f, uint64_t timeout)
{
	if (!glFenceSync)
		return true;

	GLsync sync = GL_TRACE(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
	GLenum rc = GL_TRACE(glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout));

	return rc == GL_ALREADY_SIGNALED || rc == GL_CONDITION_SATISFIED;
}

bool Re_WaitSemaphore(struct NeSemaphore *s, uint64_t value, uint64_t timeout) { return Re_WaitForFence(NULL, 0); }
bool Re_WaitSemaphores(uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout) { return Re_WaitForFence(NULL, 0); }


/* NekoEngine
 *
 * GLSynchronization.c
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
