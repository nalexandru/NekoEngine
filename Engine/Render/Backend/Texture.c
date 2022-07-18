#include <stdlib.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Render/Backend.h>

static NeHandle _placeholderTexture;
static struct NeSampler *_sceneSampler;

static bool _CreateTextureResource(const char *name, const struct NeTextureCreateInfo *ci, struct NeTextureResource *tex, NeHandle h);
static bool _LoadTextureResource(struct NeResourceLoadInfo *li, const char *args, struct NeTextureResource *tex, NeHandle h);
static void _UnloadTextureResource(struct NeTextureResource *tex, NeHandle h);
static inline bool _InitTexture(struct NeTexture *tex, const struct NeTextureCreateInfo *tci);
static inline uint32_t _RowSize(enum NeTextureFormat fmt, uint32_t width);
static inline uint32_t _ByteSize(enum NeTextureFormat fmt, uint32_t width, uint32_t height);

static bool
_CreateTextureResource(const char *name, const struct NeTextureCreateInfo *ci, struct NeTextureResource *tex, NeHandle h)
{
	if ((h & 0x00000000FFFFFFFF) > (uint64_t)RE_MAX_TEXTURES)
		return false;

	tex->texture = Re_BkCreateTexture(&ci->desc, E_ResHandleToGPU(h));
	if (!tex->texture)
		return false;

	if (!_InitTexture(tex->texture, ci))
		return false;
	
	if (!ci->keepData)
		Sys_Free(ci->data);

	return true;
}

static bool
_LoadTextureResource(struct NeResourceLoadInfo *li, const char *args, struct NeTextureResource *tex, NeHandle h)
{
	if ((h & 0x00000000FFFFFFFF) > (uint64_t)RE_MAX_TEXTURES)
		return false;

	bool rc = false, cube = args && strstr(args, "cube"), flip = args && strstr(args, "flip");
	struct NeTextureCreateInfo ci =
	{
		.desc =
		{
			.type = cube ? TT_Cube : TT_2D,
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
	else if (strstr(li->path, ".hdr"))
		rc = E_LoadHDRAsset(&li->stm, &ci, flip);
	else
		rc = E_LoadImageAsset(&li->stm, &ci, flip);

	if (cube && ci.desc.type == TT_2D) {
		// cubemap not loaded from a DDS file

		ci.desc.width /= 4;
		ci.desc.height /= 3;

		uint64_t rowSize = ci.desc.width * 4;
		uint64_t imageSize = ci.desc.width * rowSize;

		uint8_t *cubeData = Sys_Alloc(imageSize, 6, MH_Asset);
		uint64_t cubeOffset = 0;

		for (uint32_t i = 0; i < 6; ++i) {
			uint64_t offset = 0;

			switch (i) {
			case 0: offset = 4 * imageSize + rowSize * 2; break;
			case 1: offset = 4 * imageSize; break;
			case 2: offset = rowSize; break;
			case 3: offset = 8 * imageSize + rowSize; break;
			case 4: offset = 4 * imageSize + rowSize; break;
			case 5: offset = 4 * imageSize + rowSize * 3; break;
			}

			for (uint32_t j = 0; j < ci.desc.width; ++j) {
				const uint64_t dstOffset = cubeOffset + rowSize * j;
				const uint64_t srcOffset = offset + 4 * rowSize * j;

				memcpy(cubeData + dstOffset, (uint8_t *)ci.data + srcOffset, rowSize);
			}

			cubeOffset += imageSize;
		}

		Sys_Free(ci.data);

		ci.data = cubeData;
		ci.dataSize = imageSize * 6;
		ci.desc.arrayLayers = 6;
		ci.desc.type = TT_Cube;
	}

	if (rc)
		tex->texture = Re_BkCreateTexture(&ci.desc, E_ResHandleToGPU(h));

	if (!tex->texture || (ci.data && !_InitTexture(tex->texture, &ci)))
		goto error;
	
	Sys_Free(ci.data);

	return true;
	
error:
	if (tex->texture)
		Re_TDestroyNeTexture(tex->texture);
	
	Sys_Free(ci.data);
	
	return false;
}

static void
_UnloadTextureResource(struct NeTextureResource *tex, NeHandle h)
{
	Re_Destroy(tex->texture);
}

const struct NeTextureDesc *
Re_TextureDesc(NeTextureHandle tex)
{
	struct NeTextureResource *res = E_ResourcePtr(E_GPUHandleToRes(tex, RES_TEXTURE));
	return res ? &res->desc : NULL;
}

enum NeTextureLayout
Re_TextureLayout(NeTextureHandle tex)
{
	struct NeTextureResource *res = E_ResourcePtr(E_GPUHandleToRes(tex, RES_TEXTURE));
	return res ? Re_BkTextureLayout(res->texture) : TL_UNKNOWN;
}

bool
Re_InitTextureSystem(void)
{
	if (!E_RegisterResourceType(RES_TEXTURE, sizeof(struct NeTextureResource), (NeResourceCreateProc)_CreateTextureResource,
							(NeResourceLoadProc)_LoadTextureResource, (NeResourceUnloadProc)_UnloadTextureResource))
		return false;

	struct NeSamplerDesc desc =
	{
		.minFilter = IF_LINEAR,
		.magFilter = IF_LINEAR,
		.mipmapMode = SMM_LINEAR,
		.enableAnisotropy = E_GetCVarBln("Render_AnisotropicFiltering", true)->bln,
		.maxAnisotropy = E_GetCVarFlt("Render_Anisotropy", 16)->flt,
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
	struct NeTextureCreateInfo tci =
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
_InitTexture(struct NeTexture *tex, const struct NeTextureCreateInfo *tci)
{
	NeBufferHandle staging;
	struct NeBufferCreateInfo bci =
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

	for (uint32_t i = 0; i < tci->desc.arrayLayers; ++i) {
		for (uint32_t j = 0; j < tci->desc.mipLevels; ++j) {
			uint32_t w = tci->desc.width >> j; w = w ? w : 1;
			uint32_t h = tci->desc.height >> j; h = h ? h : 1;
			uint32_t d = tci->desc.depth >> j; d = d ? d : 1;

			struct NeBufferImageCopy bic =
			{
				.bufferOffset = offset,
				.bytesPerRow = _RowSize(tci->desc.format, w),
				.rowLength = w,
				.imageHeight = h,
				.subresource =
				{
					.aspect = IA_COLOR,
					.mipLevel = j,
					.baseArrayLayer = i,
					.layerCount = 1
				},
				.imageOffset = { 0, 0, 0 },
				.imageSize = { w, h, d }
			};
			Re_CmdCopyBufferToTexture(staging, tex, &bic);

			offset += _ByteSize(tci->desc.format, w, h);
		}
	}
	
	Re_ExecuteTransfer(Re_EndCommandBuffer());
	Re_DestroyBuffer(staging);
	
	return true;
}

static inline uint32_t
_RowSize(enum NeTextureFormat fmt, uint32_t width)
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
_ByteSize(enum NeTextureFormat fmt, uint32_t width, uint32_t height)
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
