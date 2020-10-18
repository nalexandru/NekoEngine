#include <stddef.h>

#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <System/Memory.h>

#include "VKRender.h"

bool
VK_InitModel(const char *name, struct Model *model)
{
	VkResult rc;
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;
	VkDeviceSize vertexSize = sizeof(struct Vertex) * model->numVertices;
	VkDeviceSize indexSize = sizeof(uint32_t) * model->numIndices;

	VkBufferCreateInfo vertexBci =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = vertexSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
	};
	rc = vkCreateBuffer(Re_Device.dev, &vertexBci, VK_CPUAllocator, &mrd->vertexBuffer);

	VkBufferCreateInfo indexBci =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = indexSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
	};
	rc = vkCreateBuffer(Re_Device.dev, &indexBci, VK_CPUAllocator, &mrd->indexBuffer);

	uint32_t type;
	VkDeviceSize vertexAlignment, indexOffset, allocationSize;
	VkMemoryRequirements memReq;

	vkGetBufferMemoryRequirements(Re_Device.dev, mrd->vertexBuffer, &memReq);
	vertexAlignment = memReq.alignment;
	indexOffset = memReq.size;
	type = memReq.memoryTypeBits;

	vkGetBufferMemoryRequirements(Re_Device.dev, mrd->indexBuffer, &memReq);
	indexOffset = ROUND_UP(indexOffset, memReq.alignment);
	type |= memReq.memoryTypeBits;

	allocationSize = indexOffset + memReq.size;

	VkDeviceSize asOffset = 0, asSize = 0, scratchAlignment = 0, scratchSize = 0, scratchOffset;
	VkDeviceSize *asOffsets = NULL;
	VkDeviceSize *scratchOffsets = NULL;

	if (Re.features.rayTracing) {
		mrd->structures = Sys_Alloc(sizeof(*mrd->structures), model->numMeshes, MH_Persistent);

		asOffsets = Sys_Alloc(sizeof(VkDeviceSize), model->numMeshes, MH_Transient);
		scratchOffsets = Sys_Alloc(sizeof(VkDeviceSize), model->numMeshes, MH_Transient);

		for (uint32_t i = 0; i < model->numMeshes; ++i) {
			struct Mesh m = model->meshes[i];

			VkAccelerationStructureCreateGeometryTypeInfoKHR asGeomInfo =
			{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.maxPrimitiveCount = m.indexCount,
				.indexType = VK_INDEX_TYPE_UINT32,
				.maxVertexCount = m.vertexCount,
				.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
				.allowsTransforms = VK_FALSE
			};
			VkAccelerationStructureCreateInfoKHR asci =
			{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
				.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
				.maxGeometryCount = 1,
				.pGeometryInfos = &asGeomInfo
			};
			rc = vkCreateAccelerationStructureKHR(Re_Device.dev, &asci, VK_CPUAllocator, &mrd->structures[i].as);

			VkAccelerationStructureMemoryRequirementsInfoKHR mri =
			{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR,
				.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
				.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
				.accelerationStructure = mrd->structures[i].as
			};
			VkMemoryRequirements2 memReq = { .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
			vkGetAccelerationStructureMemoryRequirementsKHR(Re_Device.dev, &mri, &memReq);

			if (!asOffset)
				asOffset = ROUND_UP(allocationSize, memReq.memoryRequirements.alignment);

			asOffsets[i] = ROUND_UP(asSize, memReq.memoryRequirements.alignment);
			asSize = asOffsets[i] + memReq.memoryRequirements.size;

			type |= memReq.memoryRequirements.memoryTypeBits;

			mri.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
			vkGetAccelerationStructureMemoryRequirementsKHR(Re_Device.dev, &mri, &memReq);

			// TODO: update support
			//mri.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_KHR;
			//VkMemoryRequirements2 memReq = { .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
			//vkGetAccelerationStructureMemoryRequirementsKHR(Re_Device.dev, &mri, &memReq);

			scratchAlignment = memReq.memoryRequirements.alignment;
			scratchOffsets[i] = ROUND_UP(scratchSize, memReq.memoryRequirements.alignment);
			scratchSize = scratchOffsets[i] + memReq.memoryRequirements.size;

			type |= memReq.memoryRequirements.memoryTypeBits;
		}

		allocationSize = ROUND_UP(asOffset + asSize, scratchAlignment);
		scratchOffset = allocationSize;

		allocationSize += scratchSize;

		VkBufferCreateInfo scratchCI = 
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.flags = 0,
			.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			.size = scratchSize
		};
		rc = vkCreateBuffer(Re_Device.dev, &scratchCI, VK_CPUAllocator, &mrd->scratchBuffer);
	}

	VkMemoryAllocateInfo mai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = allocationSize,
		.memoryTypeIndex = VK_MemoryTypeIndex(type, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	rc = vkAllocateMemory(Re_Device.dev, &mai, VK_CPUAllocator, &mrd->memory);

	VkBufferDeviceAddressInfo bdai = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	
	rc = vkBindBufferMemory(Re_Device.dev, mrd->vertexBuffer, mrd->memory, 0);
	bdai.buffer = mrd->vertexBuffer;
	mrd->vertexAddress = vkGetBufferDeviceAddress(Re_Device.dev, &bdai);

	rc = vkBindBufferMemory(Re_Device.dev, mrd->indexBuffer, mrd->memory, indexOffset);
	bdai.buffer = mrd->indexBuffer;
	mrd->indexAddress = vkGetBufferDeviceAddress(Re_Device.dev, &bdai);

	if (Re.features.rayTracing) {
		VkBindAccelerationStructureMemoryInfoKHR *basmi = Sys_Alloc(sizeof(*basmi), model->numMeshes, MH_Transient);
		memset(basmi, 0x0, sizeof(*basmi) * model->numMeshes);

		for (uint32_t i = 0; i < model->numMeshes; ++i) {
			basmi[i].sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
			basmi[i].accelerationStructure = mrd->structures[i].as;
			basmi[i].memory = mrd->memory;
			basmi[i].memoryOffset = asOffset + asOffsets[i];
		}
		rc = vkBindAccelerationStructureMemoryKHR(Re_Device.dev, model->numMeshes, basmi);

		rc = vkBindBufferMemory(Re_Device.dev, mrd->indexBuffer, mrd->memory, scratchOffset);
		bdai.buffer = mrd->scratchBuffer;
		mrd->scratchAddress = vkGetBufferDeviceAddress(Re_Device.dev, &bdai);

		for (uint32_t i = 0; i < model->numMeshes; ++i) {
			VkAccelerationStructureDeviceAddressInfoKHR asdai = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, NULL, mrd->structures[i].as };
			mrd->structures[i].address = vkGetAccelerationStructureDeviceAddressKHR(Re_Device.dev, &asdai);
			mrd->structures[i].scratchAddress = mrd->scratchAddress + scratchOffsets[i];
		}

		VK_StageBLASBuild(model, VK_FALSE);
	}

	return true;
}

void
VK_TermModel(struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;
	
/*	glDeleteBuffers(1, &mrd->vbo);
	glDeleteBuffers(1, &mrd->ibo);
	glDeleteVertexArrays(1, &mrd->vao);*/
}
