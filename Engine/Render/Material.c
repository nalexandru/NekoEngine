#include <System/Log.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Material.h>
#include <Runtime/Json.h>
#include <Runtime/Runtime.h>
#include <Engine/Resource.h>
#include <System/AtomicLock.h>

#define MAT_MOD				L"Material"
#define MAT_BUFFER_SIZE		2 * 1024 * 1024

struct Block
{
	uint64_t offset;
	uint32_t size;
};

struct RenderPassDesc *Re_MaterialRenderPassDesc;

static struct Array _types, _freeList;
static BufferHandle _materialBuffer;
static uint8_t *_materialData, *_materialPtr, *_gpuBufferPtr;
static uint64_t _bufferSize;
static struct AtomicLock _lock = { 0, 0 };

// default material
static bool _initDefaultMaterial(const char **args, struct DefaultMaterial *data);
static void _termDefaultMaterial(struct DefaultMaterial *data);

static inline struct Pipeline *_createPipeline(const struct MaterialResource *res, struct Shader *shader);

// material type
static inline struct MaterialType *_findMaterialType(const char *name, uint32_t *id);
static inline bool _allocMaterial(uint32_t size, uint64_t *offset, void **data);
static inline void _freeMaterial(uint64_t offset, uint32_t size);

static int32_t _cmpFunc(const struct MaterialType *type, uint64_t *hash);
static int32_t _sortFunc(const struct MaterialType *a, const struct MaterialType *b);
static int32_t _blockCmpFunc(const struct Block *b, const uint32_t *size);

static bool _CreateMaterialResource(const char *name, const struct MaterialResourceCreateInfo *ci, struct MaterialResource *tex, Handle h);
static bool _LoadMaterialResource(struct ResourceLoadInfo *li, const char *args, struct MaterialResource *tex, Handle h);
static void _UnloadMaterialResource(struct MaterialResource *tex, Handle h);

bool
Re_InitMaterial(Handle res, struct Material *mat)
{
	struct MaterialResource *mr = E_ResourcePtr(res);
	if (!mr)
		return false;

	struct MaterialType *mt = Rt_ArrayGet(&_types, mr->typeId);
	if (!mt)
		return false;

	Sys_ZeroMemory(mat, sizeof(*mat));

	if (!_allocMaterial(mt->dataSize, &mat->offset, &mat->data))
		return false;

	mat->pipeline = _createPipeline(mr, mt->shader);
	if (!mat->pipeline)
		goto error;

	if (!mt->init((const char **)mr->args.data, mat->data))
		goto error;

	memcpy(mat->name, mr->name, sizeof(mat->name));
	mat->alphaBlend = mr->alphaBlend;

	return true;

error:
	_freeMaterial(mat->offset, mt->dataSize);

	return false;
}

void
Re_TermMaterial(struct Material *mat)
{
	if (mat->type >= _types.count) {
		Sys_LogEntry(MAT_MOD, LOG_CRITICAL, L"Attempt to free invalid material type for material %s", mat->name);
		return;
	}

	struct MaterialType *mt = Rt_ArrayGet(&_types, mat->type);
	mt->term(mat->data);

	_freeMaterial(mat->offset, mt->dataSize);
}

uint64_t
Re_MaterialAddress(struct Material *mat)
{
	return Re_BufferAddress(_materialBuffer, Re_frameId * _bufferSize + mat->offset);
}

static bool
_CreateMaterialResource(const char *name, const struct MaterialResourceCreateInfo *ci, struct MaterialResource *mr, Handle h)
{
	mr->alphaBlend = ci->alphaBlend;
	snprintf(mr->name, sizeof(mr->name), "%s", ci->name);

	uint32_t id;
	if (!_findMaterialType(ci->type, &id)) {
		Sys_LogEntry(MAT_MOD, LOG_CRITICAL, L"Failed to create material resource %s, material type %s does not exist", name, ci->type);
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

	if (!Rt_InitPtrArray(&mr->args, argc + 1, MH_Asset))
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
_LoadMaterialResource(struct ResourceLoadInfo *li, const char *args, struct MaterialResource *mr, Handle h)
{
	struct Metadata meta =
	{
		.version = MATERIAL_META_VER,
		.id = MATERIAL_META_ID
	};

	if (!E_LoadMetadataFromStream(&meta, &li->stm))
		return false;

	uint8_t *data = Sys_Alloc(1, meta.jsonSize, MH_Asset);
	if (!data)
		return false;
	mr->data = data;

	if (!Rt_InitPtrArray(&mr->args, 10, MH_Asset))
		return false;

	for (uint32_t i = 0; i < meta.tokenCount; ++i) {
		jsmntok_t key = meta.tokens[i];
		jsmntok_t val = meta.tokens[++i];

		if (JSON_STRING("name", key, meta.json)) {
			snprintf(mr->name, sizeof(mr->name), "%s", meta.json + val.start);
		} else if (JSON_STRING("type", key, meta.json)) {
			char *type = meta.json + val.start;
			meta.json[val.end] = 0x0;

			uint32_t id;
			if (!_findMaterialType(type, &id)) {
				Sys_LogEntry(MAT_MOD, LOG_CRITICAL, L"Failed to load material resource %hs, material type %hs does not exist", li->path, type);
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
_UnloadMaterialResource(struct MaterialResource *mr, Handle h)
{
	Sys_Free(mr->data);
	Rt_TermArray(&mr->args);
}

bool
Re_RegisterMaterialType(const char *name, const char *shader, uint32_t dataSize, MaterialInitProc init, MaterialTermProc term)
{
	struct MaterialType mt =
	{
		.hash = Rt_HashString(name),
		.dataSize = dataSize,
		.shader = Re_GetShader(shader),
		.init = init,
		.term = term
	};

	if (!mt.shader) {
		Sys_LogEntry(MAT_MOD, LOG_CRITICAL, L"Shader %hs not found for material type %hs", shader, name);
		return false;
	}

	snprintf(mt.name, sizeof(mt.name), "%s", name);

	if (!Rt_ArrayAdd(&_types, &mt))
		return false;

	Rt_ArraySort(&_types, (RtSortFunc)_sortFunc);

	return true;
}

bool
Re_InitMaterialSystem(void)
{
	if (!E_RegisterResourceType(RES_MATERIAL, sizeof(struct MaterialResource), (ResourceCreateProc)_CreateMaterialResource,
						(ResourceLoadProc)_LoadMaterialResource, (ResourceUnloadProc)_UnloadMaterialResource))
		return false;

	_bufferSize = E_GetCVarU64(L"Render_MaterialBufferSize", MAT_BUFFER_SIZE)->u64;
	struct BufferCreateInfo bci =
	{
		.desc =
		{
			.size = _bufferSize * RE_NUM_FRAMES,
			.usage = BU_TRANSFER_DST | BU_STORAGE_BUFFER,
			.memoryType = MT_CPU_COHERENT
		},
	};
	if (!Re_CreateBuffer(&bci, &_materialBuffer))
		return false;

	_gpuBufferPtr = Re_MapBuffer(_materialBuffer);
	if (!_gpuBufferPtr) {
		Re_Destroy(_materialBuffer);
		return false;
	}

	_materialData = Sys_Alloc(1, _bufferSize, MH_Render);
	_materialPtr = _materialData;

	Rt_InitArray(&_types, 10, sizeof(struct MaterialType), MH_Render);

	Re_RegisterMaterialType("Default", "DefaultPBR_MR", sizeof(struct DefaultMaterial),
							(MaterialInitProc)_initDefaultMaterial, (MaterialTermProc)_termDefaultMaterial);

	struct AttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = Re_SwapchainFormat(Re_swapchain),
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_UNKNOWN,
		.layout = TL_COLOR_ATTACHMENT,
		.finalLayout = TL_PRESENT_SRC,
		.clearColor = { .3f, .0f, .4f, 1.f }
	};
	struct AttachmentDesc depthDesc =
	{
		.mayAlias = true,
		.format = TF_D32_SFLOAT,
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_DONT_CARE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_UNKNOWN,
		.layout = TL_DEPTH_ATTACHMENT,
		.finalLayout = TL_DEPTH_ATTACHMENT,
		.clearDepth = 0.f
	};
	struct AttachmentDesc normalDesc =
	{
		.mayAlias = false,
		.format = TF_R16G16B16A16_SFLOAT,
		.loadOp = ATL_LOAD,
		.storeOp = ATS_DONT_CARE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_SHADER_READ_ONLY,
		.layout = TL_SHADER_READ_ONLY,
		.finalLayout = TL_SHADER_READ_ONLY,
		.clearColor = { .3f, .0f, .4f, 1.f }
	};

	Re_MaterialRenderPassDesc = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc, &normalDesc, 1);
	
	Rt_InitArray(&_freeList, 10, sizeof(struct Block), MH_Render);

	return true;
}

void
Re_TransferMaterials(void)
{
	Sys_AtomicLockRead(&_lock);
	memcpy(_gpuBufferPtr + _bufferSize * Re_frameId, _materialData, _bufferSize);
	Sys_AtomicUnlockRead(&_lock);
}

void
Re_TermMaterialSystem(void)
{
	Re_DestroyRenderPassDesc(Re_MaterialRenderPassDesc);

	Sys_Free(_materialData);
	Rt_TermArray(&_types);
	Rt_TermArray(&_freeList);
	Re_Destroy(_materialBuffer);
}

static bool
_initDefaultMaterial(const char **args, struct DefaultMaterial *data)
{
	Handle diffuseMap = E_INVALID_HANDLE, normalMap = E_INVALID_HANDLE,
			metallicRoughnessMap = E_INVALID_HANDLE, occlusionMap = E_INVALID_HANDLE,
			clearCoatMap = E_INVALID_HANDLE, clearCoatRoughnessMap = E_INVALID_HANDLE,
			clearCoatNormalMap = E_INVALID_HANDLE, emissiveMap = E_INVALID_HANDLE,
			transmissionMap = E_INVALID_HANDLE, opacityMap = E_INVALID_HANDLE;

	data->metallic = 1.f;
	data->roughness = 1.f;
	data->specularWeight = 1.f;

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
			Sys_LogEntry(L"DefaultMaterial", LOG_WARNING, L"Unknown property %hs", arg);
			++args;
		}
	}

	data->diffuseMap = diffuseMap != E_INVALID_HANDLE ? E_ResHandleToGPU(diffuseMap) : 0;
	data->normalMap = normalMap != E_INVALID_HANDLE ? E_ResHandleToGPU(normalMap) : 0;
	data->metallicRoughnessMap = metallicRoughnessMap != E_INVALID_HANDLE ? E_ResHandleToGPU(metallicRoughnessMap) : 0;
	data->occlusionMap = occlusionMap != E_INVALID_HANDLE ? E_ResHandleToGPU(occlusionMap) : 0;
	data->transmissionMap = transmissionMap != E_INVALID_HANDLE ? E_ResHandleToGPU(transmissionMap) : 0;
	data->emissionMap = emissiveMap != E_INVALID_HANDLE ? E_ResHandleToGPU(emissiveMap) : 0;
	data->clearCoatRoughnessMap = clearCoatRoughnessMap != E_INVALID_HANDLE ? E_ResHandleToGPU(clearCoatRoughnessMap) : 0;
	data->clearCoatNormalMap = clearCoatNormalMap != E_INVALID_HANDLE ? E_ResHandleToGPU(clearCoatNormalMap) : 0;
	data->clearCoatMap = clearCoatMap != E_INVALID_HANDLE ? E_ResHandleToGPU(clearCoatMap) : 0;
	data->alphaMaskMap = opacityMap != E_INVALID_HANDLE ? E_ResHandleToGPU(opacityMap) : 0;

	return true;
}

static void
_termDefaultMaterial(struct DefaultMaterial *data)
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

static inline struct Pipeline *
_createPipeline(const struct MaterialResource *mr, struct Shader *shader)
{
	struct BlendAttachmentDesc blendAttachments[] =
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
	struct GraphicsPipelineDesc desc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL |
					RE_CULL_NONE | RE_FRONT_FACE_CW |
					RE_DEPTH_TEST | RE_DEPTH_WRITE | RE_DEPTH_OP_GREATER_EQUAL,
		.stageInfo = &shader->opaqueStages,
		.renderPassDesc = Re_MaterialRenderPassDesc,
		.pushConstantSize = sizeof(struct MaterialRenderConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments,
		.depthFormat = TF_D32_SFLOAT
	};

	if (mr->alphaBlend && shader->transparentStages.stageCount)
		desc.stageInfo = &shader->transparentStages;

	return Re_GraphicsPipeline(&desc);
}

static inline struct MaterialType *
_findMaterialType(const char *name, uint32_t *id)
{
	uint64_t hash = Rt_HashString(name);
	size_t typeId = Rt_ArrayBSearchId(&_types, &hash, (RtCmpFunc)_cmpFunc);

	if (typeId == RT_NOT_FOUND)
		return NULL;

	if (id)
		*id = (uint32_t)typeId;

	return Rt_ArrayGet(&_types, typeId);
}

static inline bool
_allocMaterial(uint32_t size, uint64_t *offset, void **data)
{
	Sys_AtomicLockWrite(&_lock);
	
	/*
	 * This will search only for a free block with the same size; this
	 * is usually true.
	 */
	if (_freeList.count) {
		size_t id = Rt_ArrayFindId(&_freeList, &size, (RtCmpFunc)_blockCmpFunc);
		if (id != RT_NOT_FOUND) {
			struct Block *b = Rt_ArrayGet(&_freeList, id);
			*offset = b->offset;
			*data = _materialData + *offset;
			Rt_ArrayRemove(&_freeList, id);
			return true;
		}
	}
	
	*offset = _materialPtr - _materialData;

	if (*offset + size >= _bufferSize) {
		Sys_AtomicUnlockWrite(&_lock);
		return false;
	}

	*data = _materialPtr;
	_materialPtr += size;

	Sys_AtomicUnlockWrite(&_lock);

	return true;
}

static inline void
_freeMaterial(uint64_t offset, uint32_t size)
{
	Sys_AtomicLockWrite(&_lock);
	
	struct Block b = { .offset = offset, .size = size };
	Rt_ArrayAdd(&_freeList, &b);

	Sys_AtomicUnlockWrite(&_lock);
}

static int32_t
_cmpFunc(const struct MaterialType *type, uint64_t *hash)
{
	if (type->hash > *hash)
		return 1;
	else if (type->hash < *hash)
		return -1;
	else
		return 0;
}

static int32_t
_sortFunc(const struct MaterialType *a, const struct MaterialType *b)
{
	if (a->hash > b->hash)
		return 1;
	else if (a->hash < b->hash)
		return -1;
	else
		return 0;
}

static int32_t
_blockCmpFunc(const struct Block *b, const uint32_t *size)
{
	return (b->size == *size) ? 0 : -1;
}
