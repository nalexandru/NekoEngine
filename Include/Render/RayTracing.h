#ifndef NE_RENDER_RAY_TRACING_H
#define NE_RENDER_RAY_TRACING_H

#include <Render/Types.h>
#include <Render/Device.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeAccelerationStructureAABB
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
};

struct NeAccelerationStructureInstance
{
	float transform[3][4];
	uint32_t instanceIndex : 24;
	uint32_t mask : 8;
	uint32_t shaderBindingTableOffset : 24;
	uint32_t flags : 8;
	uint64_t accelerationStructureHandle;
};

struct NeAccelerationStructureGeometryDesc
{
	enum NeAccelerationStructureGeometryType type;

	union {
		struct {
			// vertexFormat; VkFormat
			uint64_t vertexBufferAddress;
			uint64_t stride;
			uint32_t vertexCount;
			enum NeIndexType indexType;
			uint64_t indexBufferAddress;
			uint64_t transformBufferAddress;
		} triangles;
		struct {
			uint64_t address;
			uint64_t stride;
		} aabbs;
		struct {
			uint64_t address;
		} instances;
	};
};

struct NeAccelerationStructureBuildInfo
{
	enum NeAccelerationStructureType type;
	NeAccelerationStructureFlags flags;
	enum NeAccelerationStructureBuildMode mode;
	struct NeAccelerationStructure *src, *dst;
	uint32_t geometryCount;
	struct NeAccelerationStructureGeometryDesc *geometries;
	uint64_t scratchAddress;
};

struct NeAccelerationStructureRangeInfo
{
	uint32_t primitiveCount;
	uint32_t primitiveOffset;
	uint32_t firstVertex;
	uint32_t transformOffset;
};

struct NeAccelerationStructureDesc
{
	enum NeAccelerationStructureType type;
	struct NeAccelerationStructureGeometryDesc geometryDesc;
	enum NeGPUMemoryType memoryType;
};

struct NeAccelerationStructureCreateInfo
{
	struct NeAccelerationStructureDesc desc;
};

struct NeAccelerationStructure *Re_CreateAccelerationStructure(const struct NeAccelerationStructureCreateInfo *aci);
uint64_t Re_AccelerationStructureHandle(const struct NeAccelerationStructure *as);
void Re_DestroyAccelerationStructure(struct NeAccelerationStructure *as);

struct NeShaderBindingTable *Re_CreateShaderBindingTable(void);
void Re_SBTAddShader(struct NeShaderBindingTable *sbt, enum NeShaderEntryType type, struct NeShader *sh);
void Re_BuildShaderBindingTable(struct NeShaderBindingTable *sbt);
void Re_DestroyShaderBindingTable(struct NeShaderBindingTable *sbt);

void Re_CmdBuildAccelerationStructures(uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo);

#ifdef __cplusplus
}
#endif

#endif /* NE_RENDER_RAY_TRACING_H */

/* NekoEngine
 *
 * RayTracing.h
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
