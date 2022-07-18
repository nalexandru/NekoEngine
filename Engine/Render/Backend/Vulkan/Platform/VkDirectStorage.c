#if 0

#include <Engine/Engine.h>

#define VOLK_IMPLEMENTATION
#define VK_USE_PLATFORM_WIN32_KHR
#include "../VulkanBackend.h"

#define COBJMACROS
#include <d3d12.h>

static ID3D12Device10 *_device;
//static IDStorageFactory *_factory;

bool
Vkd_InitDStorage(void)
{
//	HRESULT hr;

//	hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_1, &IID_ID3D12Device10, &_device);

//	hr = DStorageGetFactory(&_factory);
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
Vkd_DStorageCloseFile(NeDirectIOHandle handle)
{
//	IDStorageFile_Release((IDStorageFile *)handle);
}

void
Vkd_TermDStorage(void)
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

#endif
