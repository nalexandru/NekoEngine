#ifndef _RE_MATERIAL_H_
#define _RE_MATERIAL_H_

#define MAP_DIFFUSE				0
#define MAP_NORMAL				1
#define MAP_EMISSIVE			2
#define MAP_METALLIC_ROUGHNESS	3
#define MAP_AO					4
#define MAP_ALPHA_MASK			5
#define MAP_CLEARCOAT			6
#define MAP_CLEARCOAT_NORMAL	7
#define MAP_CLEARCOAT_ROUGHNESS	8
#define MAP_TRANSMISSION		9

#define RE_MAX_TEXTURES			10

#define ALPHA_MODE_OPAQUE		0
#define ALPHA_MODE_MASK			1
#define ALPHA_MODE_BLEND		2

#ifndef __DXC_VERSION_RELEASE

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct MaterialProperties
{
	struct {
		float r, g, b, a;
	} color;

	struct {
		float r, g, b;
	} emissive;

	uint32_t alphaMode;

	float clearcoat, clearcoatRoughness;
	float roughness, metallic;
	float transmission, alphaCutoff;
};

struct Material
{
	uint64_t hash, shaderHash;
	const wchar_t *name;
	const wchar_t *shader;

	struct MaterialProperties props;

	const char *textures[RE_MAX_TEXTURES];

	uint8_t renderDataStart;
};

struct MaterialInstance
{
	struct MaterialProperties props;
	void *shader;
	Handle textures[RE_MAX_TEXTURES];
};

bool Re_LoadMaterials(void);
void Re_UnloadMaterials(void);

bool Re_CreateMaterial(const wchar_t *name, const wchar_t *shader, const struct MaterialProperties *props, const char *textures[]);

bool Re_InstantiateMaterial(const wchar_t *name, struct MaterialInstance *inst);
void Re_DestroyMaterialInstance(struct MaterialInstance *inst);

// Implemented in render library
bool Re_InitMaterial(struct Material *m);
struct MaterialInstance Re_InitMaterialInstance(struct Material *mat);
void Re_TermMaterial(struct Material *m);

#endif // _DXC_VERSION

#ifdef __cplusplus
}
#endif

#endif /* _RE_MATERIAL_H_ */