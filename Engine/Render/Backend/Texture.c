#include <stdatomic.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Render/Backend.h>

static atomic_uint_fast32_t f_nextId;
static struct NeQueue f_freeList;

static NeHandle f_placeholderTexture;
static struct NeSampler *f_sceneSampler;

struct uint128
{
	uint64_t l, h;
};

static bool CreateTextureResource(const char *name, const struct NeTextureCreateInfo *ci, struct NeTextureResource *tex, NeHandle h);
static bool LoadTextureResource(struct NeResourceLoadInfo *li, const char *args, struct NeTextureResource *tex, NeHandle h);
static void UnloadTextureResource(struct NeTextureResource *tex, NeHandle h);
static inline bool InitTexture(struct NeTexture *tex, const struct NeTextureCreateInfo *tci);
static void Flip8(uint32_t *data, uint32_t width, uint32_t height);
static void Flip16(uint64_t *data, uint32_t width, uint32_t height);
static void Flip32(struct uint128 *data, uint32_t width, uint32_t height);

static bool
CreateTextureResource(const char *name, const struct NeTextureCreateInfo *ci, struct NeTextureResource *tex, NeHandle h)
{
	if ((h & 0x00000000FFFFFFFF) > (uint64_t)RE_MAX_TEXTURES)
		return false;

	tex->texture = Re_BkCreateTexture(&ci->desc, E_ResHandleToGPU(h));
	if (!tex->texture)
		return false;

	if (!InitTexture(tex->texture, ci))
		return false;

	if (!ci->keepData)
		Sys_Free(ci->data);

	return true;
}

static bool
LoadTextureResource(struct NeResourceLoadInfo *li, const char *args, struct NeTextureResource *tex, NeHandle h)
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
		rc = Asset_LoadDDS(&li->stm, &ci);
	else if (strstr(li->path, ".tga"))
		rc = Asset_LoadTGA(&li->stm, &ci);
	else if (strstr(li->path, ".hdr"))
		rc = Asset_LoadHDR(&li->stm, &ci);
	else
		rc = Asset_LoadImage(&li->stm, &ci, flip);

	if (flip && ci.desc.format < TF_D32_SFLOAT) {
		if (ci.desc.format < TF_R16G16B16A16_UNORM)
			Flip8(ci.data, ci.desc.width, ci.desc.height);
		else if (ci.desc.format < TF_R32G32B32A32_UINT)
			Flip16(ci.data, ci.desc.width, ci.desc.height);
		else
			Flip32(ci.data, ci.desc.width, ci.desc.height);
	}

	if (cube && ci.desc.type == TT_2D) {
		// cubemap not loaded from a DDS file

		ci.desc.width /= 4;
		ci.desc.height /= 3;

		uint64_t rowSize = (uint64_t)ci.desc.width * 4;
		uint64_t imageSize = (uint64_t)ci.desc.width * rowSize;

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

	if (!tex->texture || (ci.data && !InitTexture(tex->texture, &ci)))
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
UnloadTextureResource(struct NeTextureResource *tex, NeHandle h)
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
Re_ReserveTextureId(NeTextureHandle *handle)
{
	if (f_freeList.count) {
		*handle = *(NeBufferHandle*)Rt_QueuePop(&f_freeList);
	} else {
		const uint32_t nextId = atomic_fetch_add(&f_nextId, 1);
		if (nextId < UINT16_MAX) {
			*handle = nextId;
		} else {
			atomic_fetch_add(&f_nextId, -1);
			*handle = (uint16_t)-1;
			return false;
		}
	}

	return true;
}

void
Re_ReleaseTextureId(NeTextureHandle handle)
{
	Rt_QueuePush(&f_freeList, &handle);
}

bool
Re_InitTextureSystem(void)
{
	if (!E_RegisterResourceType(RES_TEXTURE, sizeof(struct NeTextureResource), (NeResourceCreateProc)CreateTextureResource,
							(NeResourceLoadProc)LoadTextureResource, (NeResourceUnloadProc)UnloadTextureResource))
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
	f_sceneSampler = Re_CreateSampler(&desc);
	if (!f_sceneSampler)
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
	f_placeholderTexture = E_CreateResource("__PlaceholderTexture", RES_TEXTURE, &tci);
	E_ReleaseResource(f_placeholderTexture);

	atomic_store(&f_nextId, UINT16_MAX - E_GetCVarU32("Render_TransientTextureHandles", 35)->u32);
	return Rt_InitQueue(&f_freeList, UINT16_MAX, sizeof(NeBufferHandle), MH_Render);
}

void
Re_TermTextureSystem(void)
{
	Re_DestroySampler(f_sceneSampler);
	Rt_TermQueue(&f_freeList);
}

static inline bool
InitTexture(struct NeTexture *tex, const struct NeTextureCreateInfo *tci)
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

	Re_BeginTransferCommandBuffer(NULL);

	uint64_t offset = 0;

	for (uint32_t i = 0; i < tci->desc.arrayLayers; ++i) {
		for (uint32_t j = 0; j < tci->desc.mipLevels; ++j) {
			uint32_t w = tci->desc.width >> j; w = w ? w : 1;
			uint32_t h = tci->desc.height >> j; h = h ? h : 1;
			uint32_t d = tci->desc.depth >> j; d = d ? d : 1;

			struct NeBufferImageCopy bic =
			{
				.bufferOffset = offset,
				.bytesPerRow = Re_TextureRowSize(tci->desc.format, w),
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

			offset += Re_TextureByteSize(tci->desc.format, w, h);
		}
	}

	Re_ExecuteTransfer(Re_EndCommandBuffer());
	Re_DestroyBuffer(staging);

	return true;
}

static void
Flip8(uint32_t *data, uint32_t width, uint32_t height)
{
	for (uint32_t i = 0; i < height / 2; ++i) {
		uint32_t *sptr = data + width * i;
		uint32_t *dptr = data + width * (height - i - 1);

		for (uint32_t j = 0; j < width; ++j) {
			*sptr = *sptr ^ *dptr;
			*dptr = *sptr ^ *dptr;
			*sptr = *sptr ^ *dptr++;
			++sptr;
		}
	}
}

static void
Flip16(uint64_t *data, uint32_t width, uint32_t height)
{
	for (uint32_t i = 0; i < height / 2; ++i) {
		uint64_t *sptr = data + width * i;
		uint64_t *dptr = data + width * (height - i - 1);

		for (uint32_t j = 0; j < width; ++j) {
			*sptr = *sptr ^ *dptr;
			*dptr = *sptr ^ *dptr;
			*sptr = *sptr ^ *dptr++;
			++sptr;
		}
	}
}

static void
Flip32(struct uint128 *data, uint32_t width, uint32_t height)
{
	for (uint32_t i = 0; i < height / 2; ++i) {
		struct uint128 *sptr = data + width * i;
		struct uint128 *dptr = data + width * (height - i - 1);

		for (uint32_t j = 0; j < width; ++j) {
			sptr->l = sptr->l ^ dptr->l;
			sptr->h = sptr->h ^ dptr->h;

			dptr->l = sptr->l ^ dptr->l;
			dptr->h = sptr->h ^ dptr->h;

			sptr->l = sptr->l ^ dptr->l;
			sptr->h = sptr->h ^ dptr->h;

			++sptr; ++dptr;
		}
	}
}

/* NekoEngine
 *
 * Texture.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
