#pragma once

#include <Engine/Types.h>

#include "D3D12Render.h"

class RenderPath
{
public:
	virtual bool Init() = 0;
	virtual void RenderScene(const struct Scene *s, ID3D12Resource *output, D3D12_RESOURCE_STATES startState, D3D12_RESOURCE_STATES endState) = 0;
	virtual void Term() = 0;
};
