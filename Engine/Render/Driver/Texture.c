#include <stdlib.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Render.h>

static Handle _placeholderTexture;
static struct Sampler *_sceneSampler;

static bool _CreateTextureResource(const char *name, const struct TextureCreateInfo *ci, struct TextureResource *tex, Handle h);
static bool _LoadTextureResource(struct ResourceLoadInfo *li, const char *args, struct TextureResource *tex, Handle h);
static void _UnloadTextureResource(struct TextureResource *tex, Handle h);
static inline struct Texture *_CreateTexture(const struct TextureCreateInfo *tci, uint16_t location) { return Re_deviceProcs.CreateTexture(Re_device, tci, location); };

static bool
_CreateTextureResource(const char *name, const struct TextureCreateInfo *ci, struct TextureResource *tex, Handle h)
{
	if ((h & 0x00000000FFFFFFFF) > (uint64_t)65535)
		return false;

	tex->texture = _CreateTexture(ci, E_ResHandleToGPU(h));
	if (!tex->texture)
		return false;

	if (!ci->keepData)
		Sys_Free(ci->data);

	return true;
}

static bool
_LoadTextureResource(struct ResourceLoadInfo *li, const char *args, struct TextureResource *tex, Handle h)
{
	if ((h & 0x00000000FFFFFFFF) > (uint64_t)65535)
		return false;

	bool rc = false;
	struct TextureCreateInfo ci =
	{
		.desc =
		{
			.type = TT_2D,
			.depth = 1,
			.usage = TU_SAMPLED | TU_TRANSFER_DST,
			.gpuOptimalTiling = true,
			.memoryType = MT_GPU_LOCAL
		}
	};

	if (strstr(li->path, ".dds"))
		rc = false;
	else if (strstr(li->path, ".tga"))
		rc = E_LoadTGAAsset(&li->stm, &ci);
	else
		rc = E_LoadImageAsset(&li->stm, &ci);

	if (rc)
		tex->texture = _CreateTexture(&ci, E_ResHandleToGPU(h));

	Sys_Free(ci.data);

	return tex->texture != NULL;
}

static void
_UnloadTextureResource(struct TextureResource *tex, Handle h)
{
	Re_Destroy(tex->texture);
}

const struct TextureDesc *
Re_TextureDesc(TextureHandle tex)
{
	struct TextureResource *res = E_ResourcePtr(E_GPUHandleToRes(tex, RES_TEXTURE));
	return res ? &res->desc : NULL;
}

enum TextureLayout
Re_TextureLayout(TextureHandle tex)
{
	struct TextureResource *res = E_ResourcePtr(E_GPUHandleToRes(tex, RES_TEXTURE));
	return res ? Re_deviceProcs.TextureLayout(res->texture) : TL_UNKNOWN;
}

bool
Re_InitTextureSystem(void)
{
	if (!E_RegisterResourceType(RES_TEXTURE, sizeof(struct TextureResource), (ResourceCreateProc)_CreateTextureResource,
							(ResourceLoadProc)_LoadTextureResource, (ResourceUnloadProc)_UnloadTextureResource))
		return false;

	struct SamplerDesc desc =
	{
		.minFilter = IF_LINEAR,
		.magFilter = IF_LINEAR,
		.mipmapMode = SMM_LINEAR,
		.enableAnisotropy = E_GetCVarBln(L"Render_AnisotropicFiltering", true)->bln,
		.maxAnisotropy = E_GetCVarFlt(L"Render_Anisotropy", 16)->flt,
		.addressModeU = SAM_REPEAT,
		.addressModeV = SAM_REPEAT,
		.addressModeW = SAM_REPEAT
	};
	_sceneSampler = Re_CreateSampler(&desc);
	if (!_sceneSampler)
		return false;

	// hot pink so it's ugly and it stands out
	uint8_t texData[] = { 255, 105, 180, 255 };
	struct TextureCreateInfo tci =
	{
		.desc =
		{
			.width = 1,
			.height = 1,
			.depth = 1,
			.type = TT_2D,
			.usage = TU_SAMPLED | TU_TRANSFER_DST,
			.format = TF_R8G8B8A8_UNORM,
			.arrayLayers = 1,
			.mipLevels = 1,
			.gpuOptimalTiling = true,
			.memoryType = MT_GPU_LOCAL
		},
		.data = texData,
		.dataSize = sizeof(texData),
		.keepData = true
	};
	_placeholderTexture = E_CreateResource("__PlaceholderTexture", RES_TEXTURE, &tci);
	E_ReleaseResource(_placeholderTexture);

	return true;
}

void
Re_TermTextureSystem(void)
{
	Re_DestroySampler(_sceneSampler);
}
