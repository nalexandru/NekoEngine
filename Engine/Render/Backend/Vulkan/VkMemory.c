#include <System/Log.h>
#include <System/Memory.h>

#include "VulkanBackend.h"

static void * VKAPI_CALL Alloc(void *ud, size_t size, size_t align, VkSystemAllocationScope scope);
static void * VKAPI_CALL ReAlloc(void *ud, void *mem, size_t size, size_t align, VkSystemAllocationScope scope);
static void   VKAPI_CALL Free(void *ud, void *mem);

struct VkAllocationCallbacks f_transientAllocCB =
{
	.pfnAllocation = Alloc,
	.pfnReallocation = ReAlloc,
	.pfnFree = Free
};
VkAllocationCallbacks *Vkd_allocCb = NULL, *Vkd_transientAllocCb = &f_transientAllocCB;

static void * VKAPI_CALL
Alloc(void *ud, size_t size, size_t align, VkSystemAllocationScope scope)
{
	return Sys_AlignedAlloc(size, 1, align, MH_Frame);
}

static void * VKAPI_CALL
ReAlloc(void *ud, void *mem, size_t size, size_t align, VkSystemAllocationScope scope)
{
	return Sys_AlignedReAlloc(mem, size, 1, align, MH_Frame);
}

static void VKAPI_CALL
Free(void *ud, void *mem)
{
	(void)ud; (void)mem;
}

/* NekoEngine
 *
 * VkMemory.c
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
