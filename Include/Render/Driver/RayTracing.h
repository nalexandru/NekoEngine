#ifndef _NE_RENDER_DRIVER_RAY_TRACING_H_
#define _NE_RENDER_DRIVER_RAY_TRACING_H_

#include <Render/Types.h>
#include <Render/Driver/Device.h>

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
	enum NeAccelerationStructureFlags flags;
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

static inline struct NeAccelerationStructure *Re_CreateAccelerationStructure(const struct NeAccelerationStructureCreateInfo *aci)
{ return Re_deviceProcs.CreateAccelerationStructure(Re_device, aci); };
static inline uint64_t Re_AccelerationStructureHandle(struct NeRenderDevice *dev, const struct NeAccelerationStructure *as)
{ return Re_deviceProcs.AccelerationStructureHandle(Re_device, as); }
static inline void Re_DestroyAccelerationStructure(struct NeAccelerationStructure *as)
{ Re_deviceProcs.DestroyAccelerationStructure(Re_device, as); }

struct NeShaderBindingTable *Re_CreateShaderBindingTable(void);
void Re_SBTAddShader(struct NeShaderBindingTable *sbt, enum NeShaderEntryType type, struct NeShader *sh);
void Re_BuildShaderBindingTable(struct NeShaderBindingTable *sbt);
void Re_DestroyShaderBindingTable(struct NeShaderBindingTable *sbt);

#endif /* _NE_RENDER_DRIVER_RAY_TRACING_H_ */
