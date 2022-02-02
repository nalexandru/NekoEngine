#ifndef _NE_RENDER_MATERIAL_H_
#define _NE_RENDER_MATERIAL_H_

#include <Math/defs.h>
#include <Render/Types.h>
#include <Runtime/Runtime.h>

#define MATERIAL_META_VER	1
#define MATERIAL_META_ID	"NeMaterial"

typedef bool (*NeMaterialInitProc)(const char **args, void *data);
typedef void (*NeMaterialTermProc)(void *data);

struct NeMaterialType
{
	uint64_t hash;
	uint32_t dataSize;
	struct NeShader *shader;

	NeMaterialInitProc init;
	NeMaterialTermProc term;

	char name[64];
};

struct NeMaterialResource
{
	size_t typeId;
	bool alphaBlend;
	struct NeArray args;
	char name[64];
	enum NePrimitiveType primitiveType;
	void *data;
};

struct NeMaterial
{
	struct NePipeline *pipeline;
	uint64_t offset;
	void *data;
	uint32_t type;
	bool alphaBlend;
	char name[64];
};

struct NeDefaultMaterial
{
	float diffuseColor[4];

	float emissionColor[3];
	float metallic;

	float roughness;
	float alphaCutoff;
	float clearCoat;
	float clearCoatRoughness;

	float transmission;
	float specularWeight;
	uint32_t diffuseMap;
	uint32_t normalMap;

	uint32_t metallicRoughnessMap;
	uint32_t occlusionMap;
	uint32_t transmissionMap;
	uint32_t emissionMap;

	uint32_t clearCoatNormalMap;
	uint32_t clearCoatRoughnessMap;
	uint32_t clearCoatMap;
	uint32_t alphaMaskMap;
};

struct NeMaterialResourceCreateInfo
{
	enum NePrimitiveType primitiveType;
	bool alphaBlend;
	char name[64];
	char type[64];
	const char **args;
};

struct NeMaterialRenderConstants
{
	uint64_t vertexAddress;
	uint64_t sceneAddress;
	uint64_t visibleIndicesAddress;
	uint64_t instanceAddress;
	uint64_t materialAddress;
};

extern struct NeRenderPassDesc *Re_MaterialRenderPassDesc;
extern struct NeRenderPassDesc *Re_TransparentMaterialRenderPassDesc;

bool Re_InitMaterial(NeHandle res, struct NeMaterial *mat);
void Re_TermMaterial(struct NeMaterial *mat);

uint64_t Re_MaterialAddress(struct NeMaterial *mat);
bool Re_RegisterMaterialType(const char *name, const char *shader, uint32_t dataSize, NeMaterialInitProc init, NeMaterialTermProc term);

bool Re_InitMaterialSystem(void);
void Re_TransferMaterials(void);
void Re_TermMaterialSystem(void);

#endif /* _NE_RENDER_MATERIAL_H_ */
