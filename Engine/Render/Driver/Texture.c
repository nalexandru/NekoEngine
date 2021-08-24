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
static inline struct Texture *_CreateTexture(const struct TextureDesc *desc, uint16_t location) { return Re_deviceProcs.CreateTexture(Re_device, desc, location); };
static inline bool _InitTexture(struct Texture *tex, const struct TextureCreateInfo *tci);
static inline uint32_t _RowSize(enum TextureFormat fmt, uint32_t width);
static inline uint32_t _ByteSize(enum TextureFormat fmt, uint32_t width, uint32_t height);

static bool
_CreateTextureResource(const char *name, const struct TextureCreateInfo *ci, struct TextureResource *tex, Handle h)
{
	if ((h & 0x00000000FFFFFFFF) > (uint64_t)65535)
		return false;

	tex->texture = _CreateTexture(&ci->desc, E_ResHandleToGPU(h));
	if (!tex->texture)
		return false;

	if (!_InitTexture(tex->texture, ci))
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
		rc = E_LoadDDSAsset(&li->stm, &ci);
	else if (strstr(li->path, ".tga"))
		rc = E_LoadTGAAsset(&li->stm, &ci);
	else
		rc = E_LoadImageAsset(&li->stm, &ci);

	if (rc)
		tex->texture = _CreateTexture(&ci.desc, E_ResHandleToGPU(h));

	if (!tex->texture || (ci.data && !_InitTexture(tex->texture, &ci)))
		goto error;
	
	Sys_Free(ci.data);

	return true;
	
error:
	if (tex->texture)
		Re_TDestroyTexture(tex->texture);
	
	Sys_Free(ci.data);
	
	return false;
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
		.addressModeW = SAM_REPEAT,
		.maxLod = RE_SAMPLER_LOD_CLAMP_NONE
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

static inline bool
_InitTexture(struct Texture *tex, const struct TextureCreateInfo *tci)
{
	BufferHandle staging;
	struct BufferCreateInfo bci =
	{
		.desc.size = tci->dataSize,
		.desc.usage = BU_TRANSFER_SRC | BU_TRANSFER_DST,
		.desc.memoryType = MT_CPU_WRITE
	};
	if (!Re_CreateBuffer(&bci, &staging))
		return false;

	void *ptr = Re_MapBuffer(staging);
	memcpy(ptr, tci->data, tci->dataSize);
	Re_UnmapBuffer(staging);

	Re_BeginTransferCommandBuffer();

	uint64_t offset = 0;
	for (uint32_t i = 0; i < tci->desc.mipLevels; ++i) {
		uint32_t w = tci->desc.width >> i; w = w ? w : 1;
		uint32_t h = tci->desc.height >> i; h = h ? h : 1;
		uint32_t d = tci->desc.depth >> i; d = d ? d : 1;
		
		struct BufferImageCopy bic =
		{
			.bufferOffset = offset,
			.bytesPerRow = _RowSize(tci->desc.format, w),
			.rowLength = w,
			.imageHeight = h,
			.subresource =
			{
				.aspect = IA_COLOR,
				.mipLevel = i,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageSize = { w, h, d }
		};
		Re_CmdCopyBufferToTexture(staging, tex, &bic);
		
		offset += _ByteSize(tci->desc.format, w, h);
	}
	
	Re_EndCommandBuffer();
	
	struct Fence *f = Re_CreateFence(false);
	Re_SubmitTransfer(f);
	Re_WaitForFence(f, UINT64_MAX);
	
	Re_DestroyBuffer(staging);
	Re_DestroyFence(f);
	
	return true;
}

static inline uint32_t
_RowSize(enum TextureFormat fmt, uint32_t width)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM:
	case TF_R8G8B8A8_SRGB:
	case TF_B8G8R8A8_UNORM:
	case TF_B8G8R8A8_SRGB:
	case TF_A2R10G10B10_UNORM:
	case TF_D24_STENCIL8:
	case TF_D32_SFLOAT: return width * 4;
	case TF_R16G16B16A16_SFLOAT: return width * 8;
	case TF_R32G32B32A32_SFLOAT: return width * 16;
	case TF_R8G8_UNORM: return width * 2;
	case TF_R8_UNORM: return width * 1;
	/*case TF_ETC2_R8G8B8_UNORM: return MTLPixelFormatETC2_RGB8;
	case TF_ETC2_R8G8B8_SRGB: return MTLPixelFormatETC2_RGB8_sRGB;
	case TF_ETC2_R8G8B8A1_UNORM: return MTLPixelFormatETC2_RGB8A1;
	case TF_ETC2_R8G8B8A1_SRGB: return MTLPixelFormatETC2_RGB8A1_sRGB;
	case TF_EAC_R11_UNORM: return MTLPixelFormatEAC_R11Unorm;
	case TF_EAC_R11_SNORM: return MTLPixelFormatEAC_R11Snorm;
	case TF_EAC_R11G11_UNORM: return MTLPixelFormatEAC_RG11Unorm;
	case TF_EAC_R11G11_SNORM: return MTLPixelFormatEAC_RG11Snorm;*/
	case TF_BC5_UNORM:
	case TF_BC5_SNORM:
	case TF_BC6H_UF16:
	case TF_BC6H_SF16:
	case TF_BC7_UNORM:
	case TF_BC7_SRGB: return ((width + 3) / 4) * 16;
	default: return 0;
	}
	
	return 0; // this should cause a crash
}

static inline uint32_t
_ByteSize(enum TextureFormat fmt, uint32_t width, uint32_t height)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM:
	case TF_R8G8B8A8_SRGB:
	case TF_B8G8R8A8_UNORM:
	case TF_B8G8R8A8_SRGB:
	case TF_A2R10G10B10_UNORM:
	case TF_D24_STENCIL8:
	case TF_D32_SFLOAT: return width * height * 4;
	case TF_R16G16B16A16_SFLOAT: return width * height * 8;
	case TF_R32G32B32A32_SFLOAT: return width * height * 16;
	case TF_R8G8_UNORM: return width * height * 2;
	case TF_R8_UNORM: return width * height * 1;
	/*case TF_ETC2_R8G8B8_UNORM: return MTLPixelFormatETC2_RGB8;
	case TF_ETC2_R8G8B8_SRGB: return MTLPixelFormatETC2_RGB8_sRGB;
	case TF_ETC2_R8G8B8A1_UNORM: return MTLPixelFormatETC2_RGB8A1;
	case TF_ETC2_R8G8B8A1_SRGB: return MTLPixelFormatETC2_RGB8A1_sRGB;
	case TF_EAC_R11_UNORM: return MTLPixelFormatEAC_R11Unorm;
	case TF_EAC_R11_SNORM: return MTLPixelFormatEAC_R11Snorm;
	case TF_EAC_R11G11_UNORM: return MTLPixelFormatEAC_RG11Unorm;
	case TF_EAC_R11G11_SNORM: return MTLPixelFormatEAC_RG11Snorm;*/
	case TF_BC5_UNORM:
	case TF_BC5_SNORM:
	case TF_BC6H_UF16:
	case TF_BC6H_SF16:
	case TF_BC7_UNORM:
	case TF_BC7_SRGB: return ((width + 3) / 4) * ((height + 3) / 4) * 16;
	default: return 0;
	}
	
	return 0; // this should cause a crash
}
