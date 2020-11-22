#include <Render/Device.h>

#include "ShaderBindingTable.h"

struct SBTEntry
{
	const wchar_t *entryPoint;
	const void *input;
	enum ShaderEntryType type;
	UINT64 size;
};

static inline void _CalculateEntrySize(ShaderBindingTable *sbt, ShaderEntryType type, uint64_t *entrySizePtr, uint64_t *sectionSizePtr);
static inline void _CopyShaderInfo(ShaderBindingTable *sbt, ID3D12StateObjectProperties *props, ShaderEntryType type, uint8_t *ptr);

bool
D3D12_InitSBT(struct ShaderBindingTable *sbt)
{
	ZeroMemory(sbt, sizeof(*sbt));

	for (int i = 0 ; i < SET_Count; ++i)
		if (!Rt_InitArray(&sbt->entries[i], 10, sizeof(struct SBTEntry)))
			return false;

	return true;
}

void
D3D12_AddShader(struct ShaderBindingTable *sbt, enum ShaderEntryType type, const wchar_t *entryPoint, const void *input)
{
	struct SBTEntry ent = { entryPoint, input, type, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + (input ? sizeof(input) : 0) };
	ent.size = ROUND_UP(ent.size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
	Rt_ArrayAdd(&sbt->entries[type], &ent);
}

bool
D3D12_BuildSBT(struct ShaderBindingTable *sbt, ID3D12StateObjectProperties *props)
{
	uint8_t *data;
	HRESULT hr;

	uint64_t rayGenEnt = 0, missEnt = 0, hitGroupEnt = 0;

	_CalculateEntrySize(sbt, SET_Miss, &sbt->missSize, &sbt->missSectionSize);
	_CalculateEntrySize(sbt, SET_RayGen, &sbt->rayGenSize, &sbt->rayGenSectionSize);
	_CalculateEntrySize(sbt, SET_HitGroup, &sbt->hitGroupSize, &sbt->hitGroupSectionSize);

	const UINT64 size = sbt->rayGenSectionSize + sbt->missSectionSize + sbt->hitGroupSectionSize;
	D3D12_HEAP_PROPERTIES hp{ D3D12_HEAP_TYPE_UPLOAD };
	CD3DX12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(size);
	hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&sbt->res));
	if (FAILED(hr))
		return false;
	sbt->res->SetName(L"Shader Binding Table Storage Buffer");

	hr = sbt->res->Map(0, NULL, (void **)&data);
	if (FAILED(hr))
		return false;

	_CopyShaderInfo(sbt, props, SET_RayGen, data);
	data += sbt->rayGenSectionSize;

	_CopyShaderInfo(sbt, props, SET_Miss, data);
	data += sbt->missSectionSize;

	_CopyShaderInfo(sbt, props, SET_HitGroup, data);
	data += sbt->hitGroupSectionSize;

	sbt->res->Unmap(0, NULL);

	return true;
}

void
D3D12_TermSBT(struct ShaderBindingTable *sbt)
{
	D3D12_DestroyResource(sbt->res);

	for (int i = 0 ; i < SET_Count; ++i)
		Rt_TermArray(&sbt->entries[i]);
}

static inline void
_CalculateEntrySize(ShaderBindingTable *sbt, ShaderEntryType type, uint64_t *entrySizePtr, uint64_t *sectionSizePtr)
{
	const Array *a = &sbt->entries[type];
	uint64_t entrySize = 0, sectionSize = 0;

	for (size_t i = 0; i < a->count; ++i) {
		struct SBTEntry *ent = (struct SBTEntry *)Rt_ArrayGet(a, i);
		entrySize = max(entrySize, ent->size);
	}

	*entrySizePtr = entrySize;
	*sectionSizePtr = ROUND_UP(entrySize * sbt->entries[type].count, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
}

static inline void
_CopyShaderInfo(ShaderBindingTable *sbt, ID3D12StateObjectProperties *props, ShaderEntryType type, uint8_t *data)
{
	const Array *a = &sbt->entries[type];

	for (size_t i = 0; i < a->count; ++i) {
		const struct SBTEntry *ent = (struct SBTEntry *)Rt_ArrayGet(a, i);

		void *id = props->GetShaderIdentifier(ent->entryPoint);
		memcpy(data, id, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

		if (ent->input)
			memcpy(data + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, &ent->input, sizeof(ent->input));

		data += ent->size;
	}
}

