#ifndef _RE_MATERIAL_H_
#define _RE_MATERIAL_H_

#include <Render/Types.h>
#include <Runtime/Runtime.h>

#define MATERIAL_META_VER	1
#define MATERIAL_META_ID	"NeMaterial"

typedef bool (*MaterialInitProc)(const char **args, void *data);
typedef void (*MaterialTermProc)(void *data);

struct MaterialType
{
	uint64_t hash;
	uint32_t dataSize;
	struct Shader *shader;

	MaterialInitProc init;
	MaterialTermProc term;

	char name[64];
};

struct MaterialResource
{
	size_t typeId;
	bool alphaBlend;
	struct Array args;
	char name[64];
	void *data;
};

struct Material
{
	struct Pipeline *pipeline;
	uint64_t offset;
	void *data;
	uint32_t type;
	char name[64];
};

struct DefaultMaterial
{
	float diffuseColor[4];

	float emissionColor[3];
	float metallic;

	float roughness;
	float _padding[3];

	uint32_t diffuseMap;
	uint32_t normalMap;
	uint32_t metallicMap;
	uint32_t roughnessMap;
};

struct MaterialResourceCreateInfo
{
	bool alphaBlend;
	char name[64];
	char type[64];
	const char **args;
};

extern struct RenderPassDesc *Re_MaterialRenderPassDesc;

bool Re_InitMaterial(Handle res, struct Material *mat);
void Re_TermMaterial(struct Material *mat);

uint64_t Re_MaterialAddress(struct Material *mat);

bool Re_CreateMaterialResource(const char *name, const struct MaterialResourceCreateInfo *ci, struct MaterialResource *tex, Handle h);
bool Re_LoadMaterialResource(struct ResourceLoadInfo *li, const char *args, struct MaterialResource *tex, Handle h);
void Re_UnloadMaterialResource(struct MaterialResource *tex, Handle h);

bool Re_RegisterMaterialType(const char *name, const char *shader, uint32_t dataSize, MaterialInitProc init, MaterialTermProc term);

bool Re_InitMaterialSystem(void);
void Re_TransferMaterials(void);
void Re_TermMaterialSystem(void);

#endif /* _RE_MATERIAL_H_ */
