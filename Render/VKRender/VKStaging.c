#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Material.h>
#include <System/Memory.h>
#include <System/Log.h>
#include <Engine/Config.h>

#include "VKRender.h"

#define VKSTMOD	L"VulkanStaging"

struct ASBuildInfo
{
	VkAccelerationStructureCreateGeometryTypeInfoKHR geomTypeInfo;
	VkAccelerationStructureGeometryKHR asGeom;
	VkAccelerationStructureBuildGeometryInfoKHR buildGeomInfo;
	VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo;
	VkAccelerationStructureKHR as;
	VkAccelerationStructureGeometryKHR *asGeomPtr;
	VkAccelerationStructureBuildOffsetInfoKHR *buildOffsetInfoPtr;
};

static Array _blasBuildInfo;
static VkDeviceSize _offset;
static VkDeviceAddress _stagingAddress;
static VkBuffer _stagingBuffer;
static VkDeviceMemory _stagingMemory;
static uint8_t *_bufferPtr;
static VkDeviceSize *_size;
static VkDeviceSize _peakSize = 0;

static inline VkDeviceSize _TextureSize(VkFormat format, VkDeviceSize width, VkDeviceSize height);

bool
VK_InitStaging(void)
{
	VkResult rc;

	_size = &E_GetCVarU64(L"Vulkan_StagingBufferSize", 64ULL * 1024ULL * 1024ULL)->u64;

	VkBufferCreateInfo bci =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = *_size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
	};
	rc = vkCreateBuffer(Re_Device.dev, &bci, VK_CPUAllocator, &_stagingBuffer);

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(Re_Device.dev, _stagingBuffer, &memReq);

	VkMemoryAllocateInfo mai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = *_size,
		.memoryTypeIndex = VK_MemoryTypeIndex(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};
	rc = vkAllocateMemory(Re_Device.dev, &mai, VK_CPUAllocator, &_stagingMemory);

	rc = vkBindBufferMemory(Re_Device.dev, _stagingBuffer, _stagingMemory, 0);

	if (Re_Features.rayTracing)
		Rt_InitArray(&_blasBuildInfo, 10, sizeof(struct ASBuildInfo));

	return true;
}

void
VK_StageImage(const void *data, VkImage dst, VkFormat fmt, uint32_t width, uint32_t height, uint32_t depth, uint32_t levels, uint32_t layers, bool generateMipmaps)
{
	VkDeviceSize dataSize = 0, offset = _offset;

	for (uint32_t i = 0; i < levels; ++i) {
		dataSize += _TextureSize(fmt, width, height);
		if (!generateMipmaps && levels > 1) {
			for (uint32_t j = 1; j < levels; ++j)
				dataSize += _TextureSize(fmt, width >> j, height >> j);
		}
	}

	_CheckFlush(dataSize, 1);

	memcpy(_bufferPtr + _offset, data, dataSize);

	VkCommandBuffer cmdBuff = VK_TransferCommandBuffer();

	VkCommandBufferBeginInfo cbbi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(cmdBuff, &cbbi);

	VK_TransitionImage(dst, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmdBuff);

	for (uint32_t i = 0; i < layers; ++i) {
		VkBufferImageCopy r =
		{
			.bufferOffset = offset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = i,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { width, height, depth}
		};

		vkCmdCopyBufferToImage(cmdBuff, _stagingBuffer, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &r);
		r.bufferOffset += _TextureSize(fmt, width, height);

		if (generateMipmaps) {
			for (uint32_t j = 1; j < levels; ++i) {
			}
		} else {
			for (uint32_t j = 1; j < levels; ++i) {
				r.bufferRowLength = width >> j;
				r.bufferImageHeight = height >> j;
				r.imageSubresource.mipLevel = j;

				vkCmdCopyBufferToImage(cmdBuff, _stagingBuffer, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &r);
				r.bufferOffset += _TextureSize(fmt, width >> j, height >> j);
			}
		}

		offset = r.bufferOffset;
	}

	VK_TransitionImage(dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmdBuff);

	vkEndCommandBuffer(cmdBuff);

	VkPipelineStageFlags wait = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmdBuff,
		.pWaitDstStageMask = &wait
	};
	vkQueueSubmit(Re_Device.transferQueue, 1, &si, VK_NULL_HANDLE);

	_offset += dataSize;
}

/*void
VK_StageUpload(ID3D12Resource *dest, UINT64 size, const void *data, UINT64 alignment, UINT64 rowPitch, UINT64 slicePitch)
{
	assert(_offset + size < STAGING_BUFFER_SIZE);

	_cmdList->Reset(_cmdAllocator, NULL);

	D3D12_SUBRESOURCE_DATA srd = { data, (LONG_PTR)size, (LONG_PTR)size };

	if (rowPitch)
		srd.RowPitch = rowPitch;
	
	if (slicePitch)
		srd.SlicePitch = slicePitch;

	_offset = ROUND_UP(_offset, alignment);
	UpdateSubresources(_cmdList, dest, _stagingBuffer, _offset, 0, 1, &srd);
	_offset += size;

	_cmdList->Close();

	Re_Device.transferQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&_cmdList);
}*/

void *
VK_AllocStagingArea(VkDeviceSize size, VkDeviceSize alignment, VkDeviceAddress *gpuHandle)
{
	_offset = ROUND_UP(_offset, alignment);
	_CheckFlush(size, alignment);

	void *ret = _bufferPtr + _offset;

	if (gpuHandle)
		*gpuHandle = _stagingAddress + _offset;

	_offset += size;

	return ret;
}

void
VK_StageBLASBuild(struct Model *model, VkBool32 update)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;

	for (uint32_t i = 0; i < model->numMeshes; ++i) {
		struct Mesh m = model->meshes[i];

		struct ASBuildInfo *bi = (struct ASBuildInfo *)Rt_ArrayAllocate(&_blasBuildInfo);
		bi->geomTypeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		bi->geomTypeInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		bi->geomTypeInfo.maxPrimitiveCount = m.indexCount / 3;
		bi->geomTypeInfo.indexType = VK_INDEX_TYPE_UINT32;
		bi->geomTypeInfo.maxVertexCount = m.vertexCount;
		bi->geomTypeInfo.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		bi->geomTypeInfo.allowsTransforms = VK_FALSE;

		bi->asGeom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		bi->asGeom.flags = model->materialInstances[i].props.alphaMode =! ALPHA_MODE_OPAQUE ? 0 : VK_GEOMETRY_OPAQUE_BIT_KHR;
		bi->asGeom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		bi->asGeom.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		bi->asGeom.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
		bi->asGeom.geometry.triangles.vertexStride = sizeof(struct Vertex);
		bi->asGeom.geometry.triangles.vertexData.deviceAddress = mrd->vertexAddress + (sizeof(struct Vertex) * m.firstVertex);
		bi->asGeom.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		bi->asGeom.geometry.triangles.indexData.deviceAddress = mrd->indexAddress + (sizeof(uint32_t) * m.firstIndex);
		bi->asGeom.geometry.triangles.transformData.deviceAddress = 0;
		bi->asGeomPtr = &bi->asGeom;

		bi->buildGeomInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		bi->buildGeomInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		bi->buildGeomInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		bi->buildGeomInfo.update = update;
		bi->buildGeomInfo.srcAccelerationStructure = update ? mrd->structures[i].as : VK_NULL_HANDLE;
		bi->buildGeomInfo.dstAccelerationStructure = mrd->structures[i].as;
		bi->buildGeomInfo.geometryArrayOfPointers = VK_FALSE;
		bi->buildGeomInfo.geometryCount = 1;
		bi->buildGeomInfo.ppGeometries = &bi->asGeomPtr;
		bi->buildGeomInfo.scratchData.deviceAddress = mrd->structures[i].scratchAddress;

		bi->buildOffsetInfo.primitiveCount = 1;
		bi->buildOffsetInfo.primitiveOffset = 0x0;
		bi->buildOffsetInfo.firstVertex = 0;
		bi->buildOffsetInfo.transformOffset = 0x0;
		bi->buildOffsetInfoPtr = &bi->buildOffsetInfo;
	
		bi->as = mrd->structures[i].as;
	}
}

/*void
D3D12_Upload(void)
{
	SignalFence(&Re_UploadFence, Re_Device.transferQueue);

#ifdef _DEBUG
	_peakSize = max(_peakSize, _offset - _frameStart);
	_frameStart = STAGING_BUFFER_SIZE * Re_Device.frame;
#endif

	_offset = STAGING_BUFFER_SIZE * Re_Device.frame;
}*/

void
VK_BuildBLAS(VkCommandBuffer cmdBuffer)
{
	if (!_blasBuildInfo.count)
		return;

	VkAccelerationStructureBuildGeometryInfoKHR *geomInfo = Sys_Alloc(sizeof(*geomInfo), _blasBuildInfo.count, MH_Transient);
	VkAccelerationStructureBuildOffsetInfoKHR **buildOffsets = Sys_Alloc(sizeof(*buildOffsets), _blasBuildInfo.count, MH_Transient);

	for (size_t i = 0; i < _blasBuildInfo.count; ++i) {
		struct ASBuildInfo *bi = Rt_ArrayGet(&_blasBuildInfo, i);
		geomInfo[i] = bi->buildGeomInfo;
		buildOffsets[i] = bi->buildOffsetInfoPtr;
	}

	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, NULL, 0, NULL);

	vkCmdBuildAccelerationStructureKHR(cmdBuffer, (uint32_t)_blasBuildInfo.count, geomInfo, buildOffsets);

	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 0, NULL, 0, NULL, 0, NULL);;

	Rt_ClearArray(&_blasBuildInfo, false);
}

static inline VkDeviceSize
_TextureSize(VkFormat format, VkDeviceSize width, VkDeviceSize height)
{
	switch (format) {
	case VK_FORMAT_R8G8_UNORM:
		return width * height * 2;
	break;
	case VK_FORMAT_B8G8R8_UNORM:
	case VK_FORMAT_R8G8B8_UNORM:
		return width * height * 3;
	break;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_UNORM:
		return width * height * 4;
	break;
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC4_SNORM_BLOCK:
		return ((width + 3) / 4) * ((height + 3) / 4) * 8;
	break;
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
		return ((width + 3) / 4) * ((height + 3) / 4) * 16;
	break;
	case VK_FORMAT_R8_UNORM:
	default:
		return width * height;
	break;
	}
}

void
_CheckFlush(VkDeviceSize size, VkDeviceSize alignment)
{
	if (_offset + size <= *_size)
		return;

	Sys_LogEntry(VKSTMOD, LOG_WARNING, L"Flushing staging buffer, %llu bytes overflow", _offset + size - *_size);
	vkQueueWaitIdle(Re_Device.transferQueue);
	_offset = ROUND_UP(0, alignment);
}

