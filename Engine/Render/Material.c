#include <assert.h>

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

struct RenderPassDesc *Re_MaterialRenderPassDesc;

static struct Array _types;
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

bool
Re_CreateMaterialResource(const char *name, const struct MaterialResourceCreateInfo *ci, struct MaterialResource *mr, Handle h)
{
	mr->alphaBlend = ci->alphaBlend;
	strncpy(mr->name, ci->name, sizeof(mr->name));

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

	if (!Rt_InitPtrArray(&mr->args, argc, MH_Asset))
		return false;

	uint8_t *data = Sys_Alloc(1, dataSize, MH_Render);
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

	return false;
}

bool
Re_LoadMaterialResource(struct ResourceLoadInfo *li, const char *args, struct MaterialResource *mr, Handle h)
{
	struct Metadata meta =
	{
		.version = MATERIAL_META_VER,
		.id = MATERIAL_META_ID
	};

	if (!E_LoadMetadataFromStream(&meta, &li->stm))
		return false;

	uint8_t *data = Sys_Alloc(1, meta.jsonSize, MH_Render);
	if (!data)
		return false;
	mr->data = data;

	if (!Rt_InitPtrArray(&mr->args, 10, MH_Asset))
		return false;

	for (uint32_t i = 0; i < meta.tokenCount; ++i) {
		jsmntok_t key = meta.tokens[i];
		jsmntok_t val = meta.tokens[++i];

		if (JSON_STRING("name", key, meta.json)) {
			strncpy(mr->name, meta.json + val.start, sizeof(mr->name));
		} else if (JSON_STRING("type", key, meta.json)) {
			char *type = meta.json + val.start;
			meta.json[val.end] = 0x0;

			uint32_t id;
			if (!_findMaterialType(type, &id)) {
				Sys_LogEntry(MAT_MOD, LOG_CRITICAL, L"Failed to load material resource %s, material type %s does not exist", li->path, type);
				return false;
			}
			mr->typeId = id;
		} else if (JSON_STRING("alphaBlend", key, meta.json)) {
			char *alphaBlend = meta.json + val.start;
			meta.json[val.end] = 0x0;

			mr->alphaBlend = strstr(alphaBlend, "true") != NULL;
		} else {
			Rt_ArrayAddPtr(&mr->args, data);
			size_t len = key.end - key.start;
			memcpy(data, meta.json + key.start, len);
			data += len + 1;

			Rt_ArrayAddPtr(&mr->args, data);
			len = val.end - val.start;
			memcpy(data, meta.json + val.start, len);
			data += len + 1;
		}
	}

	return true;
}

void
Re_UnloadMaterialResource(struct MaterialResource *mr, Handle h)
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
		Sys_LogEntry(MAT_MOD, LOG_CRITICAL, L"Shader %s not found for material type %s", shader, name);
		return false;
	}

	strncpy(mt.name, name, sizeof(mt.name));

	if (!Rt_ArrayAdd(&_types, &mt))
		return false;

	Rt_ArraySort(&_types, (RtSortFunc)_sortFunc);

	return true;
}

bool
Re_InitMaterialSystem(void)
{
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
	Re_MaterialRenderPassDesc = Re_CreateRenderPassDesc(&atDesc, 1, NULL);

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
	Re_Destroy(_materialBuffer);
}

static bool
_initDefaultMaterial(const char **args, struct DefaultMaterial *data)
{
	Handle diffuseMap = E_INVALID_HANDLE, normalMap = E_INVALID_HANDLE,
			metallicMap = E_INVALID_HANDLE, roughnessMap = E_INVALID_HANDLE;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "DiffuseMap", len)) {
			diffuseMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "NormalMap", len)) {
			normalMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "MetallicMap", len)) {
			metallicMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "RoughnessMap", len)) {
			roughnessMap = E_LoadResource(*(++args), RES_TEXTURE);
		} else if (!strncmp(arg, "DiffuseColor", len)) {
			char *ptr = (char *)*(++args);
			data->diffuseColor[0] = strtof(ptr, &ptr);
			data->diffuseColor[1] = strtof(ptr + 2, &ptr);
			data->diffuseColor[2] = strtof(ptr + 2, &ptr);
			data->diffuseColor[3] = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "EmissionColor", len)) {
			char *ptr = (char *)*(++args);
			data->emissionColor[0] = strtof(ptr, &ptr);
			data->emissionColor[1] = strtof(ptr + 2, &ptr);
			data->emissionColor[2] = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "Metallic", len)) {
			data->metallic = strtof((char *)*(++args), NULL);
		} else if (!strncmp(arg, "Roughness", len)) {
			data->roughness = strtof((char *)*(++args), NULL);
		}
	}

	data->diffuseMap = E_ResHandleToGPU(diffuseMap);
	data->normalMap = E_ResHandleToGPU(normalMap);
	data->metallicMap = E_ResHandleToGPU(metallicMap);
	data->roughnessMap = E_ResHandleToGPU(roughnessMap);

	return true;
}

static void
_termDefaultMaterial(struct DefaultMaterial *data)
{
	E_UnloadResource(E_GPUHandleToRes(data->diffuseMap, RES_TEXTURE));
	E_UnloadResource(E_GPUHandleToRes(data->normalMap, RES_TEXTURE));
	E_UnloadResource(E_GPUHandleToRes(data->metallicMap, RES_TEXTURE));
	E_UnloadResource(E_GPUHandleToRes(data->roughnessMap, RES_TEXTURE));
}

static inline struct Pipeline *
_createPipeline(const struct MaterialResource *mr, struct Shader *shader)
{
	/*enum BlendFactor srcColor;
	enum BlendFactor dstColor;
	enum BlendOperation colorOp;
	enum BlendFactor srcAlpha;
	enum BlendFactor dstAlpha;
	enum BlendOperation alphaOp;*/
	assert(!mr->alphaBlend);

	struct BlendAttachmentDesc blendAttachments[] =
	{
		{
			.enableBlend = mr->alphaBlend,
			.writeMask = RE_WRITE_MASK_RGB
		}
	};
	struct GraphicsPipelineDesc desc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
		.shader = shader,
		.renderPassDesc = Re_MaterialRenderPassDesc,
		.pushConstantSize = 16,
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments

	};
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
	Sys_LogEntry(MAT_MOD, LOG_DEBUG, L"Free Material Not Implemented");
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
