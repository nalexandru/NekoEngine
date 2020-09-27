#pragma once

#include "../RenderPath.h"
#include "../ShaderBindingTable.h"

class RtRenderPath : public RenderPath
{
public:
	virtual bool Init() override;
	virtual void RenderScene(const struct Scene *s, ID3D12Resource *output, D3D12_RESOURCE_STATES startState, D3D12_RESOURCE_STATES endState) override;
	virtual void Term() override;

private:
	ID3D12StateObject *_pso;
	ShaderBindingTable _sbt[RE_NUM_BUFFERS];
	ID3D12StateObjectProperties *_psoProps;
	ID3D12RootSignature *_globalRootSignature, *_rayGenSignature, *_hitSignature, *_missSignature;
	ID3D12DescriptorHeap *_descHeap[RE_NUM_BUFFERS];
	UINT _descIncrement;
};
