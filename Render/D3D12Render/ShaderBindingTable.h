#pragma once

#include <Runtime/Runtime.h>

#include "D3D12Render.h"

enum ShaderEntryType
{
	SET_RayGen = 0,
	SET_Miss,
	SET_HitGroup,
	SET_Callable,

	SET_Count
};

struct ShaderBindingTable
{
	ID3D12Resource *res;
	uint64_t rayGenSectionSize, missSectionSize, hitGroupSectionSize;
	uint64_t rayGenSize, missSize, hitGroupSize;

	Array entries[SET_Count];
};

bool D3D12_InitSBT(struct ShaderBindingTable *sbt);
void D3D12_AddShader(struct ShaderBindingTable *sbt, enum ShaderEntryType type, const wchar_t *entryPoint, const void *input);
bool D3D12_BuildSBT(struct ShaderBindingTable *sbt, ID3D12StateObjectProperties *props);
void D3D12_TermSBT(struct ShaderBindingTable *sbt);
