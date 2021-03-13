#ifndef _RE_DESCRIPTOR_SET_H_
#define _RE_DESCRIPTOR_SET_H_

#include <Engine/Types.h>
#include <Render/Device.h>
#include <Render/Shader.h>
#include <Render/Texture.h>

struct BufferBindInfo
{
	struct Buffer *buff;
	uint64_t offset, size;
};

struct TextureBindInfo
{
	struct Texture *tex;
	enum TextureLayout layout;
};

struct AccelerationStructureBindInfo
{
	struct AccelerationStructure *as;
};

enum DescriptorType
{
	DT_UNIFORM_BUFFER,
	DT_STORAGE_BUFFER,
	DT_TEXTURE,
	DT_ACCELERATION_STRUCTURE
};

struct DescriptorBinding
{
	enum DescriptorType type;
	uint32_t count;
	enum ShaderStage stage;
};

struct DescriptorSetLayoutDesc
{
	uint32_t bindingCount;
	struct DescriptorBinding *bindings;
};

struct DescriptorWrite
{
	enum DescriptorType type;
	uint32_t binding, arrayElement, count;
	union {
		struct BufferBindInfo *bufferInfo;
		struct TextureBindInfo *textureInfo;
		struct AccelerationStructureBindInfo *accelerationStructureInfo;
	};
};

struct DescriptorSet;
struct DescriptorSetLayout;

static inline struct DescriptorSetLayout *Re_CreateDescriptorSetLayout(const struct DescriptorSetLayoutDesc *desc) { return Re_deviceProcs.CreateDescriptorSetLayout(Re_device, desc); }
static inline void Re_DestroyDescriptorSetLayout(struct DescriptorSetLayout *dsl) { Re_deviceProcs.DestroyDescriptorSetLayout(Re_device, dsl); }

static inline struct DescriptorSet *Re_CreateDescriptorSet(const struct DescriptorSetLayout *layout) { return Re_deviceProcs.CreateDescriptorSet(Re_device, layout); }
static inline void Re_WriteDescriptorSet(struct DescriptorSet *ds, const struct DescriptorWrite *writes, uint32_t writeCount) { Re_deviceProcs.WriteDescriptorSet(Re_device, ds, writes, writeCount); }
static inline void Re_DestroyDescriptorSet(struct DescriptorSet *ds) { Re_deviceProcs.DestroyDescriptorSet(Re_device, ds); }

#endif /* _RE_DESCRIPTOR_SET_H_ */
