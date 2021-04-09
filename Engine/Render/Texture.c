#include <stdlib.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Render/Texture.h>
#include <Render/Sampler.h>
#include <Render/Context.h>
#include <Render/DestroyResource.h>

static struct Sampler *_sceneSampler;

static inline struct Texture *_CreateTexture(const struct TextureCreateInfo *tci, uint16_t location) { return Re_deviceProcs.CreateTexture(Re_device, tci, location); };

bool
Re_CreateTextureResource(const char *name, const struct TextureCreateInfo *ci, struct TextureResource *tex, Handle h)
{
	if ((h & 0x00000000FFFFFFFF) > (uint64_t)65535)
		return false;
	
	tex->texture = _CreateTexture(ci, E_ResHandleToGPU(h));
	if (!tex->texture)
		return false;

	if (!ci->keepData)
		free(ci->data);

	return true;
}

bool
Re_LoadTextureResource(struct ResourceLoadInfo *li, const char *args, struct TextureResource *tex, Handle h)
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

	free(ci.data);

	return tex->texture != NULL;
}

void
Re_UnloadTextureResource(struct TextureResource *tex, Handle h)
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
	struct SamplerDesc desc =
	{
		.minFilter = IF_LINEAR,
		.magFilter = IF_LINEAR,
		.mipmapMode = SMM_LINEAR,
		.enableAnisotropy = E_GetCVarBln(L"Render_AnisotropicFiltering", true)->bln,
		.maxAnisotropy = E_GetCVarFlt(L"Render_Anisotropy", 16)->flt,
		.addressModeU = SAM_CLAMP_TO_EDGE,
		.addressModeV = SAM_CLAMP_TO_EDGE,
		.addressModeW = SAM_CLAMP_TO_EDGE
	};
	_sceneSampler = Re_CreateSampler(&desc);
	if (!_sceneSampler)
		return false;
	
	return true;
}

void
Re_TermTextureSystem(void)
{
	Re_DestroySampler(_sceneSampler);
}
