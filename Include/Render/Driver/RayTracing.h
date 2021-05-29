#ifndef _NE_RENDER_DRIVER_RAY_TRACING_H_
#define _NE_RENDER_DRIVER_RAY_TRACING_H_

#include <Render/Types.h>
#include <Render/Driver/Device.h>

struct AccelerationStructureAABB
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
};

struct AccelerationStructureInstance
{
	float transform[3][4];
	uint32_t instanceIndex : 24;
	uint32_t mask : 8;
	uint32_t shaderBindingTableOffset : 24;
	uint32_t flags : 8;
	uint64_t accelerationStructureHandle;
};

struct AccelerationStructureGeometryDesc
{
	enum AccelerationStructureGeometryType type;

	union {
		struct {
			// vertexFormat; VkFormat
			uint64_t vertexBufferAddress;
			uint64_t stride;
			uint32_t vertexCount;
			enum IndexType indexType;
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

struct AccelerationStructureBuildInfo
{
	enum AccelerationStructureType type;
	enum AccelerationStructureFlags flags;
	enum AccelerationStructureBuildMode mode;
	struct AccelerationStructure *src, *dst;
	uint32_t geometryCount;
	struct AccelerationStructureGeometryDesc *geometries;
	uint64_t scratchAddress;
};

struct AccelerationStructureRangeInfo
{
	uint32_t primitiveCount;
	uint32_t primitiveOffset;
	uint32_t firstVertex;
	uint32_t transformOffset;
};

struct AccelerationStructureDesc
{
	enum AccelerationStructureType type;
	struct AccelerationStructureGeometryDesc geometryDesc;
	enum GPUMemoryType memoryType;
};

struct AccelerationStructureCreateInfo
{
	struct AccelerationStructureDesc desc;
};

static inline struct AccelerationStructure *Re_CreateAccelerationStructure(const struct AccelerationStructureCreateInfo *aci)
{ return Re_deviceProcs.CreateAccelerationStructure(Re_device, aci); };
static inline uint64_t Re_AccelerationStructureHandle(struct RenderDevice *dev, const struct AccelerationStructure *as)
{ return Re_deviceProcs.AccelerationStructureHandle(Re_device, as); }
static inline void Re_DestroyAccelerationStructure(struct AccelerationStructure *as)
{ Re_deviceProcs.DestroyAccelerationStructure(Re_device, as); }

struct ShaderBindingTable *Re_CreateShaderBindingTable(void);
void Re_SBTAddShader(struct ShaderBindingTable *sbt, enum ShaderEntryType type, struct Shader *sh);
void Re_BuildShaderBindingTable(struct ShaderBindingTable *sbt);
void Re_DestroyShaderBindingTable(struct ShaderBindingTable *sbt);

#endif /* _NE_RENDER_DRIVER_RAY_TRACING_H_ */
