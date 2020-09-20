#include <Scene/Scene.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Material.h>
#include <Render/ModelRender.h>
#include <Scene/Transform.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>

#include "VKRender.h"

/*struct Drawable
{
	GLuint vao;
	GLuint program;
	GLsizei count;
	void *indices;
	GLint baseVertex;
};

struct GetDrawablesArgs
{
	Array *drawables;

};*/

const size_t Re_SceneRenderDataSize = sizeof(struct SceneRenderData);

bool
Re_InitScene(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	return Rt_InitArray(&srd->instanceData, 10, sizeof(VkAccelerationStructureInstanceKHR));

	return true;
}

void
Re_TermScene(struct Scene *scene)
{
/*	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	Rt_TermArray(&srd->drawables);*/
}

void
VK_BuildTLAS(VkCommandBuffer cmdBuffer, struct Scene *scene, struct Camera *cam)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;

	Rt_ClearArray(&srd->instanceData, false);
	Rt_ClearArray(&srd->materialData, false);
	E_ExecuteSystemS(scene, PREPARE_SCENE_DATA_SYS, srd);

//	D3D12_StageUpload(srd->materialBuffer, srd->materialData.elem_size * srd->materialData.count,
//		srd->materialData.data, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

	VkAccelerationStructureCreateGeometryTypeInfoKHR geomInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.maxPrimitiveCount = (uint32_t)srd->instanceData.count,
		.allowsTransforms = VK_TRUE
	};
	VkAccelerationStructureCreateInfoKHR asci =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR,
		.maxGeometryCount = 1,
		.pGeometryInfos = &geomInfo,
	};
	VkAccelerationStructureKHR tlas = VK_CreateTransientAccelerationStructure(&asci);

	VkAccelerationStructureMemoryRequirementsInfoKHR mri =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR,
		.accelerationStructure = tlas,
		.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
	};

	VkMemoryRequirements2 memReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
	vkGetAccelerationStructureMemoryRequirementsKHR(Re_Device.dev, &mri, &memReq);

	VkBufferCreateInfo bci =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = memReq.memoryRequirements.size,
		.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
	};
	VkBuffer scratch = VK_CreateTransientBuffer(&bci);

	VkBufferDeviceAddressInfo bdai =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = scratch
	};
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(Re_Device.dev, &bdai);

	VkDeviceSize instanceSize = srd->instanceData.count * srd->instanceData.elem_size;
	VkDeviceAddress instanceAddress;

	void *instanceBuffer = VK_AllocStagingArea(instanceSize, 64, &instanceAddress);
	memcpy(instanceBuffer, srd->instanceData.data, instanceSize);

	VkAccelerationStructureGeometryKHR asGeom =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = { .instances = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.arrayOfPointers = VK_FALSE,
			.data.deviceAddress = instanceAddress
		} }
	};
	const VkAccelerationStructureGeometryKHR *asGeomPtr = &asGeom;

	VkAccelerationStructureBuildGeometryInfoKHR buildGeomInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = asci.type,
		.flags = asci.flags,
		.update = VK_FALSE,
		.dstAccelerationStructure = tlas,
		.geometryArrayOfPointers = VK_FALSE,
		.geometryCount = 1,
		.ppGeometries = &asGeomPtr,
		.scratchData = scratchAddress
	};
	
	VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo =
	{
		.primitiveCount = (uint32_t)srd->instanceData.count,
		.primitiveOffset = 0x0,
		.firstVertex = 0,
		.transformOffset = 0
	};
	const VkAccelerationStructureBuildOffsetInfoKHR *buildOffsetInfoPtr = &buildOffsetInfo;

	VkMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR
	};
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, NULL, 0, NULL);

	vkCmdBuildAccelerationStructureKHR(cmdBuffer, 1, &buildGeomInfo, &buildOffsetInfoPtr);

	//

	srd->topLevelAS = tlas;
}

void
D3D12_PrepareSceneData(void **comp, struct SceneRenderData *args)
{
	struct Transform *xform = (struct Transform *)comp[0];
	struct ModelRender *modelRender = (struct ModelRender *)comp[1];

	struct Model *model = (struct Model *)E_ResourcePtr(modelRender->model);
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;

	VkAccelerationStructureInstanceKHR *inst = Rt_ArrayAllocate(&args->instanceData);

/*	struct mat4 mat;
	m4_transpose(&mat, &xform->mat);*/
	memcpy(&inst->transform, &xform->mat, sizeof(inst->transform));

	inst->instanceShaderBindingTableRecordOffset = 0;
	inst->mask = 0xFF;

	for (uint32_t i = 0; i < model->numMeshes; ++i) {
		inst->instanceCustomIndex = i;
		inst->flags = 0;
		inst->accelerationStructureReference = mrd->structures[i].address;

		Rt_ArrayAdd(&args->materialData, &model->materialInstances[i].props);
	}
}

