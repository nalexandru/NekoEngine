#include <assert.h>
#include <stdlib.h>

#include <System/Memory.h>

#include "GLBackend.h"

struct NeRenderPassDesc *
Re_CreateRenderPassDesc(const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment,
						const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount)
{
	struct NeRenderPassDesc *rp = NULL;

	// sanity checks
	if (count > NE_ARRAY_SIZE(rp->color) || inputCount > NE_ARRAY_SIZE(rp->input))
		return NULL;

	rp = Sys_Alloc(sizeof(*rp), 1, MH_RenderBackend);
	if (!rp)
		return NULL;

	for (uint32_t i = 0; i < count; ++i) {
		struct GLBkRenderPassAttachment *at = &rp->color[i];

		at->clear = attachments[i].loadOp == ATL_CLEAR;
		at->store = attachments[i].storeOp == ATS_STORE;

		memcpy(at->clearColor, attachments[i].clearColor, sizeof(at->clearColor));
	}

	if (depthAttachment) {
		rp->hasDepth = true;
		rp->depth.clear = depthAttachment->loadOp == ATL_CLEAR;
		rp->depth.store = depthAttachment->storeOp == ATS_STORE;

		memcpy(rp->depth.clearColor, depthAttachment->clearColor, sizeof(rp->depth.clearColor));
	} else {
		rp->hasDepth = false;
	}

	for (uint32_t i = 0; i < inputCount; ++i) {
		struct GLBkRenderPassAttachment *at = &rp->input[i];

		at->clear = inputAttachments[i].loadOp == ATL_CLEAR;
		at->store = inputAttachments[i].storeOp == ATS_STORE;

		memcpy(at->clearColor, inputAttachments[i].clearColor, sizeof(at->clearColor));
	}

	return rp;
}

void
Re_DestroyRenderPassDesc(struct NeRenderPassDesc *rp)
{
	Sys_Free(rp);
}

/* NekoEngine
 *
 * GLRenderPass.c
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
