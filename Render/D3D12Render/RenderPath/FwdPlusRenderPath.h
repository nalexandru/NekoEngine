#pragma once

#include "../RenderPath.h"
#include "../ShaderBindingTable.h"

class FwdPlusRenderPath : public RenderPath
{
public:
	virtual bool Init() override;
	virtual void RenderScene(const struct Scene *s, ID3D12Resource *output, D3D12_RESOURCE_STATES outputState) override;
	virtual void Term() override;

private:
	ID3D12StateObject *_pso;
	ID3D12StateObjectProperties *_psoProps;
	ID3D12RootSignature *_globalRootSignature;
	ID3D12DescriptorHeap *_rtvHeap, *_dsvHeap, *_descHeap[RE_NUM_BUFFERS];
	UINT _srvIncrement, _rtvIncrement, _dsvIncrement;
	struct ShaderLibrary *_library;
};
