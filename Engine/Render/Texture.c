#include <stdlib.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Render/Texture.h>
#include <Render/Sampler.h>
#include <Render/DescriptorSet.h>

static struct DescriptorSet *_textureDescriptorSet;
static struct DescriptorSetLayout *_textureSetLayout;
static uint32_t _usedSlots, _textureSlots;
static struct Sampler *_sceneSampler;

static inline struct Texture *_CreateTexture(const struct TextureCreateInfo *tci) { return Re_deviceProcs.CreateTexture(Re_device, tci); };
static inline void _DestroyTexture(struct Texture *tex) { Re_deviceProcs.DestroyTexture(Re_device, tex); }

static inline void _ResizeDescriptorSet(void);

bool
Re_CreateTextureResource(const char *name, const struct TextureCreateInfo *ci, struct TextureResource *tex, Handle h)
{
	tex->texture = _CreateTexture(ci);
	if (!tex->texture)
		return false;

	if (!ci->keepData)
		free(ci->data);

	return true;
}

bool
Re_LoadTextureResource(struct ResourceLoadInfo *li, const char *args, struct TextureResource *tex, Handle h)
{
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
		tex->texture = _CreateTexture(&ci);
	
	struct TextureBindInfo textureInfo =
	{
		.tex = tex->texture,
		.layout = TL_SHADER_READ_ONLY
	};
	struct DescriptorWrite dw =
	{
		.type = DT_TEXTURE,
		.binding = _usedSlots++,
		.count = 1,
		.textureInfo = &textureInfo
	};
	Re_WriteDescriptorSet(_textureDescriptorSet, &dw, 1);

	free(ci.data);

	return tex->texture != NULL;
}

void
Re_UnloadTextureResource(struct TextureResource *tex, Handle h)
{
	_DestroyTexture(tex->texture);
}

struct Sampler *
Re_SceneTextureSampler(void)
{
	return _sceneSampler;
}

const struct DescriptorSetLayout *
Re_TextureDescriptorSetLayout(void)
{
	return _textureSetLayout;
}

void
Re_BindTextureDescriptorSet(struct PipelineLayout *layout, uint32_t pos)
{
	Re_BindDescriptorSets(layout, pos, 1, (const struct DescriptorSet **)&_textureDescriptorSet);
}

bool
Re_InitTextureSystem(void)
{
	struct SamplerDesc desc =
	{
		.minFilter = TFL_LINEAR,
		.magFilter = TFL_LINEAR,
		.mipmapMode = TMM_LINEAR,
		.enableAnisotropy = E_GetCVarBln(L"Render_AnisotropicFiltering", true)->bln,
		.maxAnisotropy = E_GetCVarFlt(L"Render_Anisotropy", 16)->flt,
		.addressModeU = TAM_CLAMP_TO_EDGE,
		.addressModeV = TAM_CLAMP_TO_EDGE,
		.addressModeW = TAM_CLAMP_TO_EDGE
	};
	_sceneSampler = Re_CreateSampler(&desc);
	if (!_sceneSampler)
		return false;

	_ResizeDescriptorSet();
	
	return _textureSetLayout && _textureDescriptorSet;
}

void
Re_TermTextureSystem(void)
{
	Re_DestroySampler(_sceneSampler);

	Re_DestroyDescriptorSet(_textureDescriptorSet);
	Re_DestroyDescriptorSetLayout(_textureSetLayout);
}

static inline void _ResizeDescriptorSet(void)
{
	uint32_t newCount = _textureSlots ? _textureSlots * 2 : E_GetCVarI32(L"Render_InitialTextureSlots", 16)->i32;
	
	struct DescriptorSetLayout *oldLayout = _textureSetLayout;
	struct DescriptorSet *oldSet = _textureDescriptorSet;
	
	struct DescriptorBinding binding =
	{
		.type = DT_TEXTURE,
		.flags = DBF_PARTIALLY_BOUND | DBF_UPDATE_AFTER_BIND,
		.count = newCount,
		.stage = SS_FRAGMENT
	};
	struct DescriptorSetLayoutDesc desc =
	{
		.bindingCount = 1,
		.bindings = &binding
	};
	
	_textureSetLayout = Re_CreateDescriptorSetLayout(&desc);
	if (!_textureSetLayout)
		goto error;
	
	_textureDescriptorSet = Re_CreateDescriptorSet(_textureSetLayout);
	if (!_textureDescriptorSet)
		goto error;
	
	if (_usedSlots)
		Re_CopyDescriptorSet(oldSet, 0, _textureDescriptorSet, 0, _textureSlots);
	
	if (oldLayout) {
		// queue layout destroy
	}
	
	if (oldSet) {
		// queue ds destroy
	}
	
	_textureSlots = newCount;
	
	return;
	
error:
	_textureSetLayout = oldLayout;
	_textureDescriptorSet = oldSet;
}
