#ifdef _WIN32

#include <Engine/Engine.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include "../VulkanBackend.h"

#define COBJMACROS
#include <d3d12.h>

static ID3D12Device10 *_device;
//static IDStorageFactory *_factory;

bool
VkBk_InitDStorage(void)
{
//	HRESULT hr;

//	hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_1, &IID_ID3D12Device10, &_device);

//	hr = DStorageGetFactory(&_factory);
	return false;
}

NeDirectIOHandle
Vkd_DStorageOpenFile(const char *path)
{
//	IDStorageFile *file;
//	HRESULT hr = IDStorageFactory_OpenFile(_factory, /* wchar */, &IID_IDStorageFile, &file);

//	return file;
	return NULL;
}

void
VkBk_DStorageCloseFile(NeDirectIOHandle handle)
{
//	IDStorageFile_Release((IDStorageFile *)handle);
}

void
VkBk_TermDStorage(void)
{
//	IDStorageFactory_Release(_factory);
//	ID3D12Device10_Release(_device);
}

VkImage
_ConvertToImage(ID3D12Resource *res)
{
	VkExternalMemoryImageCreateInfo emiInfo =
	{
		.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
		.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT,
	};

	VkImageCreateInfo ici =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = &emiInfo
	};

	VkImage img;
	vkCreateImage(Re_device->dev, &ici, Vkd_allocCb, &img);
	
//	VkDedicatedAllocationImageCreateInfoNV

	VkImportMemoryWin32HandleInfoNV handleInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV,
		.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT,
		.handle = res,
	};

	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &handleInfo,
		.allocationSize = 0
	};

	return img;
}

#else

#include "../VulkanBackend.h"

bool VkBk_InitDStorage(void) { return false; }
NeDirectIOHandle VkBk_DStorageOpenFile(const char *path) { return NULL; }
void VkBk_DStorageCloseFile(NeDirectIOHandle handle) { }
void VkBk_TermDStorage(void) { }

#endif

/* NekoEngine
 *
 * VkDirectStorage.c
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
