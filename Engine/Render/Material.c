#include <System/Log.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Material.h>
#include <Runtime/Json.h>
#include <Runtime/Runtime.h>
#include <Engine/Resource.h>
#include <System/AtomicLock.h>

#define MAT_MOD				"Material"
#define MAT_BUFFER_SIZE		2 * 1024 * 1024

struct NeMaterialBlock
{
	uint64_t offset;
	uint32_t size;
};

struct NeRenderPassDesc *Re_MaterialRenderPassDesc;
struct NeRenderPassDesc *Re_TransparentMaterialRenderPassDesc;

static struct NeArray f_types, f_freeList;
static NeBufferHandle f_materialBuffer;
static uint8_t *f_materialData, *f_materialPtr, *f_gpuBufferPtr;
static uint64_t f_bufferSize;
static struct NeAtomicLock f_lock = { 0, 0 };

// default material
static bool InitDefaultMaterial(const char **args, struct NeDefaultMaterial *data);
static void TermDefaultMaterial(struct NeDefaultMaterial *data);

static inline struct NePipeline *CreatePipeline(const struct NeMaterialResource *mr, struct NeShader *shader);

// material type
static inline struct NeMaterialType *FindMaterialType(const char *name, uint32_t *id);
static inline bool AllocMaterial(uint32_t size, uint64_t *offset, void **data);
static inline void FreeMaterial(uint64_t offset, uint32_t size);

static int32_t BlockCmpFunc(const struct NeMaterialBlock *b, const uint32_t *size);

static bool CreateMaterialResource(const char *name, const struct NeMaterialResourceCreateInfo *ci, struct NeMaterialResource *mr, NeHandle h);
static bool LoadMaterialResource(struct NeResourceLoadInfo *li, const char *args, struct NeMaterialResource *mr, NeHandle h);
static void UnloadMaterialResource(struct NeMaterialResource *mr, NeHandle h);

bool
Re_InitMaterial(NeHandle res, struct NeMaterial *mat)
{
	struct NeMaterialResource *mr = E_ResourcePtr(res);
	if (!mr)
		return false;

	struct NeMaterialType *mt = Rt_ArrayGet(&f_types, mr->typeId);
	if (!mt)
		return false;

	Sys_ZeroMemory(mat, sizeof(*mat));

	if (!AllocMaterial(mt->dataSize, &mat->offset, &mat->data))
		return false;

	mat->pipeline = CreatePipeline(mr, mt->shader);
	if (!mat->pipeline)
		goto error;

	if (!mt->init((const char **)mr->args.data, mat->data))
		goto error;

	memcpy(mat->name, mr->name, sizeof(mat->name));
	mat->alphaBlend = mr->alphaBlend;

	return true;

error:
	FreeMaterial(mat->offset, mt->dataSize);

	return false;
}

void
Re_TermMaterial(struct NeMaterial *mat)
{
	if (mat->type >= f_types.count) {
		Sys_LogEntry(MAT_MOD, LOG_CRITICAL, "Attempt to free invalid material type for material %s", mat->name);
		return;
	}

	struct NeMaterialType *mt = Rt_ArrayGet(&f_types, mat->type);
	mt->term(mat->data);

	FreeMaterial(mat->offset, mt->dataSize);
}

uint64_t
Re_MaterialAddress(struct NeMaterial *mat)
{
	return Re_BufferAddress(f_materialBuffer, Re_frameId * f_bufferSize + mat->offset);
}

static bool
CreateMaterialResource(const char *name, const struct NeMaterialResourceCreateInfo *ci, struct NeMaterialResource *mr, NeHandle h)
{
	mr->alphaBlend = ci->alphaBlend;
	mr->primitiveType = ci->primitiveType;
	strlcpy(mr->name, ci->name, sizeof(mr->name));

	uint32_t id;
	if (!FindMaterialType(ci->type, &id)) {
		Sys_LogEntry(MAT_MOD, LOG_CRITICAL, "Failed to create material resource %s, material type %s does not exist", name, ci->type);
		return false;
	}
	mr->typeId = id;

	uint32_t argc = 0;
	size_t dataSize = 0;
	char **args = (char **)ci->args;
	for (; args && *args; ++args) {
		const char *key = *args;
		const char *val = *(++args);

		dataSize += strlen(key) + strlen(val) + 2;
		argc += 2;
	}

	if (!argc)
		return true;

	if (!Rt_InitPtrArray(&mr->args, (uint64_t)argc + 1, MH_Asset))
		return false;

	uint8_t *data = Sys_Alloc(1, dataSize, MH_Asset);
	if (!data)
		return false;

	mr->data = data;

	args = (char **)ci->args;
	for (; args && *args; ++args) {
		const char *key = *args;
		const char *val = *(++args);

		size_t len = strlen(key);
		Rt_ArrayAddPtr(&mr->args, data);
		memcpy(data, key, len);
		data += len + 1;

		len = strlen(val);
		Rt_ArrayAddPtr(&mr->args, data);
		memcpy(data, val, len);
		data += len + 1;
	}

	return true;
}

static bool
LoadMaterialResource(struct NeResourceLoadInfo *li, const char *args, struct NeMaterialResource *mr, NeHandle h)
{
	struct NeMetadata meta =
	{
		.version = MATERIAL_META_VER,
		.id = MATERIAL_META_ID
	};

	if (!Asset_LoadMetadataFromStream(&meta, &li->stm))
		return false;

	uint8_t *data = Sys_Alloc(1, meta.jsonSize, MH_Asset);
	if (!data)
		return false;

	mr->data = data;
	mr->primitiveType = args ? atoi(args) : PT_TRIANGLES;

	if (mr->primitiveType < PT_TRIANGLES || mr->primitiveType > PT_LINES)
		return false;

	if (!Rt_InitPtrArray(&mr->args, 10, MH_Asset))
		return false;

	for (uint32_t i = 0; i < meta.tokenCount; ++i) {
		jsmntok_t key = meta.tokens[i];
		jsmntok_t val = meta.tokens[++i];

		if (JSON_STRING("name", key, meta.json)) {
			strlcpy(mr->name, meta.json + val.start, sizeof(mr->name));
		} else if (JSON_STRING("type", key, meta.json)) {
			char *type = meta.json + val.start;
			meta.json[val.end] = 0x0;

			uint32_t id;
			if (!FindMaterialType(type, &id)) {
				Sys_LogEntry(MAT_MOD, LOG_CRITICAL, "Failed to load material resource %s, material type %s does not exist", li->path, type);
				return false;
			}
			mr->typeId = id;
		} else if (JSON_STRING("alphaBlend", key, meta.json)) {
			char *alphaBlend = meta.json + val.start;
			meta.json[val.end] = 0x0;

			mr->alphaBlend = strstr(alphaBlend, "true") != NULL;
		} else {
			Rt_ArrayAddPtr(&mr->args, data);
			size_t len = (size_t)key.end - key.start;
			memcpy(data, meta.json + key.start, len);
			data += len + 1;

			Rt_ArrayAddPtr(&mr->args, data);
			len = (size_t)val.end - val.start;
			memcpy(data, meta.json + val.start, len);
			data += len + 1;
		}
	}

	data = NULL;
	Rt_ArrayAddPtr(&mr->args, data);

	return true;
}

static void
UnloadMaterialResource(struct NeMaterialResource *mr, NeHandle h)
{
	Sys_Free(mr->data);
	Rt_TermArray(&mr->args);
}

bool
Re_RegisterMaterialType(const char *name, const char *shader, uint32_t dataSize, NeMaterialInitProc init, NeMaterialTermProc term)
{
	struct NeMaterialType mt =
	{
		.hash = Rt_HashString(name),
		.dataSize = dataSize,
		.shader = Re_GetShader(shader),
		.init = init,
		.term = term
	};

	if (!mt.shader) {
		Sys_LogEntry(MAT_MOD, LOG_CRITICAL, "Shader %s not found for material type %s", shader, name);
		return false;
	}

	strlcpy(mt.name, name, sizeof(mt.name));

	if (!Rt_ArrayAdd(&f_types, &mt))
		return false;

	Rt_ArraySort(&f_types, Rt_U64CmpFunc);

	return true;
}

bool
Re_InitMaterialSystem(void)
{
	if (!E_RegisterResourceType(RES_MATERIAL, sizeof(struct NeMaterialResource), (NeResourceCreateProc)CreateMaterialResource,
						(NeResourceLoadProc)LoadMaterialResource, (NeResourceUnloadProc)UnloadMaterialResource))
		return false;

	f_bufferSize = E_GetCVarU64("Render_MaterialBufferSize", MAT_BUFFER_SIZE)->u64;
	struct NeBufferCreateInfo bci =
	{
		.desc =
		{
			.size = f_bufferSize * RE_NUM_FRAMES,
			.usage = BU_TRANSFER_DST | BU_STORAGE_BUFFER,
			.memoryType = MT_CPU_COHERENT
		},
	};
	if (!Re_CreateBuffer(&bci, &f_materialBuffer))
		return false;

	f_gpuBufferPtr = Re_MapBuffer(f_materialBuffer);
	if (!f_gpuBufferPtr) {
		Re_Destroy(f_materialBuffer);
		return false;
	}

	f_materialData = Sys_Alloc(1, f_bufferSize, MH_Render);
	f_materialPtr = f_materialData;

	Rt_InitArray(&f_types, 10, sizeof(struct NeMaterialType), MH_Render);

	Re_RegisterMaterialType("Default", "DefaultPBR_MR", sizeof(struct NeDefaultMaterial),
							(NeMaterialInitProc)InitDefaultMaterial, (NeMaterialTermProc)TermDefaultMaterial);

	enum NeAttachmentSampleCount samples = ASC_1_SAMPLE;// E_GetCVarI32("Render_Samples", ASC_1_SAMPLE)->i32;

	struct NeAttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = Re_SwapchainFormat(Re_swapchain),
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = samples,
		.initialLayout = TL_UNKNOWN,
		.layout = TL_COLOR_ATTACHMENT,
		.finalLayout = TL_COLOR_ATTACHMENT,
		.clearColor = { .3f, .0f, .4f, 1.f }
	};
	struct NeAttachmentDesc depthDesc =
	{
		.mayAlias = true,
		.format = TF_D32_SFLOAT,
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = samples,
		.initialLayout = TL_DEPTH_ATTACHMENT,
		.layout = TL_DEPTH_ATTACHMENT,
		.finalLayout = TL_DEPTH_ATTACHMENT
	};
	struct NeAttachmentDesc normalDesc =
	{
		.mayAlias = false,
		.format = TF_R16G16B16A16_SFLOAT,
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = samples,
		.initialLayout = TL_SHADER_READ_ONLY,
		.layout = TL_SHADER_READ_ONLY,
		.finalLayout = TL_SHADER_READ_ONLY,
		.clearColor = { .3f, .0f, .4f, 1.f }
	};
	Re_MaterialRenderPassDesc = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc, &normalDesc, 1);
	
	atDesc.loadOp = ATL_LOAD;
	atDesc.initialLayout = TL_COLOR_ATTACHMENT;
	atDesc.finalLayout = TL_PRESENT_SRC;
	Re_TransparentMaterialRenderPassDesc = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc, &normalDesc, 1);

	Rt_InitArray(&f_freeList, 10, sizeof(struct NeMaterialBlock), MH_Render);

	return true;
}

void
Re_TransferMaterials(void)
{
	Sys_AtomicLockRead(&f_lock);
	memcpy(f_gpuBufferPtr + f_bufferSize * Re_frameId, f_materialData, f_bufferSize);
	Sys_AtomicUnlockRead(&f_lock);
}

void
Re_TermMaterialSystem(void)
{
	Re_DestroyRenderPassDesc(Re_TransparentMaterialRenderPassDesc);
	Re_DestroyRenderPassDesc(Re_MaterialRenderPassDesc);

	Sys_Free(f_materialData);
	Rt_TermArray(&f_types);
	Rt_TermArray(&f_freeList);
	Re_Destroy(f_materialBuffer);
}

static bool
InitDefaultMaterial(const char **args, struct NeDefaultMaterial *data)
{
	NeHandle diffuseMap = NE_INVALID_HANDLE, normalMap = NE_INVALID_HANDLE,
			metallicRoughnessMap = NE_INVALID_HANDLE, occlusionMap = NE_INVALID_HANDLE,
			clearCoatMap = NE_INVALID_HANDLE, clearCoatRoughnessMap = NE_INVALID_HANDLE,
			clearCoatNormalMap = NE_INVALID_HANDLE, emissiveMap = NE_INVALID_HANDLE,
			transmissionMap = NE_INVALID_HANDLE, opacityMap = NE_INVALID_HANDLE;

	data->metallic = 1.f;
	data->roughness = 1.f;
	data->specularWeight = 1.f;
	data->diffuseColor[0] = data->diffuseColor[1] = data->diffuseColor[2] = data->diffuseColor[3] = 1.f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!len)
			continue;

		if (!strncmp(arg, "Metallic", len)) {
			data->metallic = strtof((char *)*(++args), NULL);
		} else if (!strncmp(arg, "Roughness", len)) {
			data->roughness = strtof((char *)*(++args), NULL);
		} else if (!strncmp(arg, "AlphaCutoff", len)) {
			data->alphaCutoff = strtof((char *)*(++args), NULL);
		} else if (!strncmp(arg, "ClearCoat", len)) {
			data->clearCoat = strtof((char *)*(++args), NULL);
		} else if (!strncmp(arg, "ClearCoatRoughness", len)) {
			data->clearCoatRoughness = strtof((char *)*(++args), NULL);
		} else if (!strncmp(arg, "SpecularWeight", len)) {
			data->specularWeight = strtof((char *)*(++args), NULL);
		} else if (!strncmp(arg, "DiffuseMap", len)) {
			diffuseMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "NormalMap", len)) {
			normalMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "MetallicRoughnessMap", len)) {
			metallicRoughnessMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "OcclusionMap", len)) {
			occlusionMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "ClearCoatMap", len)) {
			clearCoatMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "ClearCoatNormalMap", len)) {
			clearCoatNormalMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "ClearCoatRoughnessMap", len)) {
			clearCoatRoughnessMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "EmissiveMap", len)) {
			emissiveMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "TransmissionMap", len)) {
			transmissionMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "OpacityMap", len)) {
			opacityMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "DiffuseColor", len)) {
			char *ptr = (char *)*(++args);
			data->diffuseColor[0] = strtof(ptr, &ptr);
			data->diffuseColor[1] = strtof(ptr + 2, &ptr);
			data->diffuseColor[2] = strtof(ptr + 2, &ptr);
			data->diffuseColor[3] = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "EmissiveColor", len)) {
			char *ptr = (char *)*(++args);
			data->emissionColor[0] = strtof(ptr, &ptr);
			data->emissionColor[1] = strtof(ptr + 2, &ptr);
			data->emissionColor[2] = strtof(ptr + 2, &ptr);
		} else {
			Sys_LogEntry("DefaultMaterial", LOG_WARNING, "Unknown property %hs", arg);
			++args;
		}
	}

	data->diffuseMap = diffuseMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(diffuseMap) : 0;
	data->normalMap = normalMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(normalMap) : 0;
	data->metallicRoughnessMap = metallicRoughnessMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(metallicRoughnessMap) : 0;
	data->occlusionMap = occlusionMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(occlusionMap) : 0;
	data->transmissionMap = transmissionMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(transmissionMap) : 0;
	data->emissionMap = emissiveMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(emissiveMap) : 0;
	data->clearCoatRoughnessMap = clearCoatRoughnessMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(clearCoatRoughnessMap) : 0;
	data->clearCoatNormalMap = clearCoatNormalMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(clearCoatNormalMap) : 0;
	data->clearCoatMap = clearCoatMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(clearCoatMap) : 0;
	data->alphaMaskMap = opacityMap != NE_INVALID_HANDLE ? E_ResHandleToGPU(opacityMap) : 0;

	return true;
}

static void
TermDefaultMaterial(struct NeDefaultMaterial *data)
{
#define UNLOAD_TEX(x) if (x) E_UnloadResource(E_GPUHandleToRes(x, RES_TEXTURE))
	
	UNLOAD_TEX(data->diffuseMap);
	UNLOAD_TEX(data->normalMap);
	UNLOAD_TEX(data->metallicRoughnessMap);
	UNLOAD_TEX(data->occlusionMap);
	UNLOAD_TEX(data->transmissionMap);
	UNLOAD_TEX(data->emissionMap);
	UNLOAD_TEX(data->clearCoatRoughnessMap);
	UNLOAD_TEX(data->clearCoatNormalMap);
	UNLOAD_TEX(data->clearCoatMap);
	UNLOAD_TEX(data->alphaMaskMap);
}

static inline struct NePipeline *
CreatePipeline(const struct NeMaterialResource *mr, struct NeShader *shader)
{
	struct NeVertexAttribute attribs[] =
	{
		{ 0, 0, VF_FLOAT3, 0, AS_POSITION },					// float x, y, z;
	//	{ 2, 0, VF_FLOAT3, sizeof(float) * 6 },					// float tx, ty, tz;
		{ 1, 0, VF_FLOAT2, sizeof(float) * 9, AS_TEXCOORD0 },	// float u, v;
		{ 2, 0, VF_FLOAT4, sizeof(float) * 11, AS_COLOR },		// float r, g, b, a;
		{ 3, 0, VF_FLOAT3, sizeof(float) * 3, AS_NORMAL }		// float nx, ny, nz;
	};

	struct NeVertexBinding bindings[] =
	{
		{ 0, sizeof(struct NeVertex), VIR_VERTEX }
	};

	struct NeBlendAttachmentDesc blendAttachments[] =
	{
		{
			.enableBlend = mr->alphaBlend,
			.writeMask = RE_WRITE_MASK_RGBA,
			.srcColor = RE_BF_SRC_ALPHA,
			.dstColor = RE_BF_ONE_MINUS_SRC_ALPHA,
			.colorOp = RE_BOP_ADD,
			.srcAlpha = RE_BF_ONE,
			.dstAlpha = RE_BF_ZERO,
			.alphaOp = RE_BOP_ADD
		}
	};
	struct NeGraphicsPipelineDesc desc =
	{
		.flags = RE_POLYGON_FILL |
					RE_CULL_NONE | RE_FRONT_FACE_CCW |
					RE_DEPTH_TEST /*| RE_DEPTH_WRITE*/ |
					(mr->alphaBlend ? RE_DEPTH_OP_GREATER_EQUAL : RE_DEPTH_OP_EQUAL),
		.stageInfo = &shader->opaqueStages,
		.renderPassDesc = Re_MaterialRenderPassDesc,
		.pushConstantSize = sizeof(struct NeMaterialRenderConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments,

		.vertexDesc.attributes = attribs,
		.vertexDesc.attributeCount = sizeof(attribs) / sizeof(attribs[0]) - (mr->alphaBlend ? 0 : 1),
		.vertexDesc.bindings = bindings,
		.vertexDesc.bindingCount = sizeof(bindings) / sizeof(bindings[0])
	};

	switch (mr->primitiveType) {
	case PT_TRIANGLES: desc.flags |= RE_TOPOLOGY_TRIANGLES; break;
	case PT_POINTS: desc.flags |= RE_TOPOLOGY_POINTS; break;
	case PT_LINES: desc.flags |= RE_TOPOLOGY_LINES; break;
	}

	if (mr->alphaBlend && shader->transparentStages.stageCount)
		desc.stageInfo = &shader->transparentStages;

	return Re_GraphicsPipeline(&desc);
}

static inline struct NeMaterialType *
FindMaterialType(const char *name, uint32_t *id)
{
	uint64_t hash = Rt_HashString(name);
	size_t typeId = Rt_ArrayBSearchId(&f_types, &hash, Rt_U64CmpFunc);

	if (typeId == RT_NOT_FOUND)
		return NULL;

	if (id)
		*id = (uint32_t)typeId;

	return Rt_ArrayGet(&f_types, typeId);
}

static inline bool
AllocMaterial(uint32_t size, uint64_t *offset, void **data)
{
	Sys_AtomicLockWrite(&f_lock);
	
	/*
	 * This will search only for a free block with the same size; this
	 * is usually true.
	 */
	if (f_freeList.count) {
		size_t id = Rt_ArrayFindId(&f_freeList, &size, (RtCmpFunc)BlockCmpFunc);
		if (id != RT_NOT_FOUND) {
			struct NeMaterialBlock *b = Rt_ArrayGet(&f_freeList, id);
			*offset = b->offset;
			*data = f_materialData + *offset;
			Rt_ArrayRemove(&f_freeList, id);
			return true;
		}
	}
	
	*offset = f_materialPtr - f_materialData;

	if (*offset + size >= f_bufferSize) {
		Sys_AtomicUnlockWrite(&f_lock);
		return false;
	}

	*data = f_materialPtr;
	f_materialPtr += size;

	Sys_AtomicUnlockWrite(&f_lock);

	return true;
}

static inline void
FreeMaterial(uint64_t offset, uint32_t size)
{
	Sys_AtomicLockWrite(&f_lock);
	
	struct NeMaterialBlock b = { .offset = offset, .size = size };
	Rt_ArrayAdd(&f_freeList, &b);

	Sys_AtomicUnlockWrite(&f_lock);
}

static int32_t
BlockCmpFunc(const struct NeMaterialBlock *b, const uint32_t *size)
{
	return (b->size == *size) ? 0 : -1;
}

/* NekoEngine
 *
 * Material.c
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
