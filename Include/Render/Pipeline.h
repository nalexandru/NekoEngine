#ifndef _RE_PIPELINE_H_
#define _RE_PIPELINE_H_

#include <Engine/Types.h>
#include <Render/Device.h>

#define RE_TOPOLOGY_POINTS			((uint64_t)  0 <<  0)
#define RE_TOPOLOGY_LINES			((uint64_t)  1 <<  0)
#define RE_TOPOLOGY_TRIANGLES		((uint64_t)  2 <<  0)
#define RE_TOPOLOGY_BITS			((uint64_t)  3 <<  0)
#define RE_TOPOLOGY_OFFSET			0

#define RE_POLYGON_FILL				((uint64_t)  0 <<  2)
#define RE_POLYGON_LINE				((uint64_t)  1 <<  2)
#define RE_POLYGON_POINT			((uint64_t)  2 <<  2)
#define RE_POLYGON_BITS				((uint64_t)  3 <<  2)
#define RE_POLYGON_OFFSET			2

#define RE_CULL_NONE				((uint64_t)  0 <<  4)
#define RE_CULL_BACK				((uint64_t)  1 <<  4)
#define RE_CULL_FRONT				((uint64_t)  2 <<  4)
#define RE_CULL_FRONT_AND_BACK		((uint64_t)  3 <<  4)
#define RE_CULL_BITS				((uint64_t)  3 <<  4)
#define RE_CULL_OFFSET				4

#define RE_FRONT_FACE_CCW			((uint64_t)  0 <<  6)
#define RE_FRONT_FACE_CW			((uint64_t)  1 <<  6)
#define RE_FRONT_FACE_BITS			((uint64_t)  1 <<  6)
#define RE_FRONT_FACE_OFFSET		6

#define RE_DISCARD					((uint64_t)  1 <<  7)
#define RE_DEPTH_TEST				((uint64_t)  1 <<  8)
#define RE_DEPTH_WRITE				((uint64_t)  1 <<  9)
#define RE_DEPTH_BIAS				((uint64_t)  1 << 10)
#define RE_DEPTH_CLAMP				((uint64_t)  1 << 11)
#define RE_DEPTH_BOUNDS				((uint64_t)  1 << 12)

#define RE_DEPTH_OP_NEVER			((uint64_t)  0 << 13)
#define RE_DEPTH_OP_LESS			((uint64_t)  1 << 13)
#define RE_DEPTH_OP_EQUAL			((uint64_t)  2 << 13)
#define RE_DEPTH_OP_LESS_EQUAL		((uint64_t)  3 << 13)
#define RE_DEPTH_OP_GREATER			((uint64_t)  4 << 13)
#define RE_DEPTH_OP_NOT_EQUAL		((uint64_t)  5 << 13)
#define RE_DEPTH_OP_GREATER_EQUAL	((uint64_t)  6 << 13)
#define RE_DEPTH_OP_ALWAYS			((uint64_t)  7 << 13)
#define RE_DEPTH_BITS				((uint64_t)  7 << 13)
#define RE_DEPTH_OFFSET				13

#define RE_MULTISAMPLE				((uint64_t)  1 << 15)
#define RE_SAMPLE_SHADING			((uint64_t)  1 << 16)
#define RE_ALPHA_TO_COVERAGE		((uint64_t)  1 << 17)
#define RE_ALPHA_TO_ONE				((uint64_t)  1 << 18)
#define RE_MS_2_SAMPLES				((uint64_t)  0 << 19)
#define RE_MS_4_SAMPLES				((uint64_t)  1 << 19)
#define RE_MS_8_SAMPLES				((uint64_t)  2 << 19)
#define RE_MS_16_SAMPLES			((uint64_t)  3 << 19)
#define RE_SAMPLES_BITS				((uint64_t)  3 << 19)
#define RE_SAMPLES_OFFSET			19

#define RE_WRITE_MASK_R				((uint64_t)  1 << 22)
#define RE_WRITE_MASK_G				((uint64_t)  1 << 23)
#define RE_WRITE_MASK_B				((uint64_t)  1 << 24)
#define RE_WRITE_MASK_A				((uint64_t)  1 << 25)
#define RE_WRITE_MASK_ALL			(RE_WRITE_MASK_R | RE_WRITE_MASK_G | RE_WRITE_MASK_B | RE_WRITE_MASK_A)

enum BlendFactor
{
	RE_BF_ZERO						=  0,
	RE_BF_ONE						=  1,
	RE_BF_SRC_COLOR					=  2,
	RE_BF_ONE_MINUS_SRC_COLOR		=  3,
	RE_BF_DST_COLOR					=  4,
	RE_BF_ONE_MINUS_DST_COLOR		=  5,
	RE_BF_SRC_ALPHA					=  6,
	RE_BF_ONE_MINUS_SRC_ALPHA		=  7,
	RE_BF_DST_ALPHA					=  8,
	RE_BF_ONE_MINUS_DST_ALPHA		=  9,
	RE_BF_CONSTANT_COLOR			= 10,
	RE_BF_ONE_MINUS_CONSTANT_COLOR	= 11,
	RE_BF_CONSTANT_ALPHA			= 12,
	RE_BF_ONE_MINUS_CONSTANT_ALPHA	= 13,
	RE_BF_SRC_ALPHA_SATURATE		= 14,
	RE_BF_SRC1_COLOR				= 15,
	RE_BF_ONE_MINUS_SRC1_COLOR		= 16,
	RE_BF_SRC1_ALPHA				= 17,
	RE_BF_ONE_MINUS_SRC1_ALPHA		= 18,
};

enum BlendOperation
{
	RE_BOP_ADD					= 0,
	RE_BOP_SUBTRACT				= 1,
	RE_BOP_REVERSE_SUBTRACT		= 2,
	RE_BOP_MIN					= 3,
	RE_BOP_MAX					= 4
};
// TODO: See VK_EXT_blend_operation_advanced

struct BlendAttachmentDesc
{
	bool enableBlend;
	enum BlendFactor srcColor;
	enum BlendFactor dstColor;
	enum BlendOperation colorOp;
	enum BlendFactor srcAlpha;
	enum BlendFactor dstAlpha;
	enum BlendOperation alphaOp;
};
// TODO: is independent blend supported in Metal/D3D12 ?

struct PipelineLayoutDesc
{
	uint32_t setLayoutCount;
	const struct DescriptorSetLayoutDesc *setLayouts;
	uint32_t pushConstantSize;
};

static inline struct PipelineLayout *Re_CreatePipelineLayout(const struct PipelineLayoutDesc *desc) { return Re_deviceProcs.CreatePipelineLayout(Re_device, desc); }
static inline void Re_DestroyPipelineLayout(struct PipelineLayout *layout) { Re_deviceProcs.DestroyPipelineLayout(Re_device, layout); }

struct Pipeline *Re_GraphicsPipeline(struct Shader *sh, uint64_t flags, struct BlendAttachmentDesc *at, uint32_t atCount);
struct Pipeline *Re_ComputePipeline(struct Shader *sh);
struct Pipeline *Re_RayTracingPipeline(struct ShaderBindingTable *sbt);

bool Re_InitPipelines(void);
void Re_TermPipelines(void);

#endif /* _RE_PIPELINE_H_ */
