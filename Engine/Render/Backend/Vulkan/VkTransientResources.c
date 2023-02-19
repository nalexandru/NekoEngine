#include <Engine/Config.h>

#include "VulkanBackend.h"

#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

struct NeTexture *
Re_CreateTransientTexture(const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	tex->transient = true;

	if (!Vk_CreateImage(desc, tex, true))
		goto error;

	VkMemoryRequirements mr;
	vkGetImageMemoryRequirements(Re_device->dev, tex->image, &mr);

	uint64_t realOffset = ROUND_UP(offset, mr.alignment);
	*size = mr.size + realOffset - offset;

	vkBindImageMemory(Re_device->dev, tex->image, Re_device->transientHeap, realOffset);

	if (!Vk_CreateImageView(desc, tex))
		goto error;

	if (location)
			Vk_SetTexture(location, tex->imageView);

	return tex;

error:
	if (tex->image)
		vkDestroyImage(Re_device->dev, tex->image, Vkd_allocCb);
	
	Sys_Free(tex);

	return NULL;
}

struct NeBuffer *
Re_BkCreateTransientBuffer(const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	buff->transient = true;

	VkBufferCreateInfo buffInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = desc->size,
		.usage = desc->usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(Re_device->dev, &buffInfo, Vkd_transientAllocCb, &buff->buff);

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(Re_device->dev, buff->buff, &mr);

	uint64_t realOffset = ROUND_UP(offset, mr.alignment);
	*size = mr.size + realOffset - offset;

	vkBindBufferMemory(Re_device->dev, buff->buff, Re_device->transientHeap, realOffset);

#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(Re_device->dev, buff->buff, VK_OBJECT_TYPE_BUFFER, desc->name);
#endif

	return buff;
}

bool
Re_InitTransientHeap(uint64_t size)
{
	VkMemoryAllocateFlagsInfo fi =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	};
	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &fi,
		.allocationSize = size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(Re_device, 0x90, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // this *might* break
	};
	if (vkAllocateMemory(Re_device->dev, &ai, Vkd_allocCb, &Re_device->transientHeap) != VK_SUCCESS)
		return false;

#ifdef _DEBUG
	Vkd_SetObjectName(Re_device->dev, Re_device->transientHeap, VK_OBJECT_TYPE_DEVICE_MEMORY, "Transient Memory Heap");
#endif

	return true;
}

bool
Re_ResizeTransientHeap(uint64_t size)
{
	return false;
}

void
Re_TermTransientHeap(void)
{
	vkFreeMemory(Re_device->dev, Re_device->transientHeap, Vkd_allocCb);
}

/* NekoEngine
 *
 * VkTransientResources.c
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
