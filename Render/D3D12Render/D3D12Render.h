#ifndef _RE_RENDER_IMPL_H_
#define _RE_RENDER_IMPL_H_

#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>

#include <System/Log.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Shader.h>
#include <Runtime/Runtime.h>
#include <Engine/Engine.h>
#include <Engine/Component.h>

#include "d3dx12.h"
#include "D3D12Downlevel.h"

#include <Math/Math.h>

#define PREPARE_SCENE_DATA_SYS	L"D3D12_PrepareSceneData"
#define DRAW_UI_CONTEXT			L"D3D12_DrawUIContext"

// From: https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/vkrt_tuto_manyhits.md.htm
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

#define D3DCHK(x) {																	\
if (HRESULT hr = x; FAILED(hr)) {													\
	if (hr == DXGI_ERROR_DEVICE_REMOVED) {											\
		D3D12_HandleDeviceRemoved();												\
	} else {																		\
		Sys_LogEntry(L"D3D12", LOG_CRITICAL, L"Error from HRESULT: 0x%x", hr);		\
		E_Shutdown();																\
	}																				\
}																					\
}

struct GlobalRenderData
{
	ID3D12DescriptorHeap *samplerHeap;
	UINT textureHeapSize, maxTextures;
};

struct RenderDevice
{
	ID3D12Device5 *dev;
	uint32_t frame;
	IDXGISwapChain3 *swapChain;
	ID3D12Resource *targets[RE_NUM_BUFFERS];
	ID3D12CommandQueue *graphicsQueue;
	ID3D12CommandQueue *computeQueue;
	ID3D12CommandQueue *transferQueue;
	ID3D12Debug3 *dbg;
	IDXGIFactory1 *factory;
	IDXGIAdapter1 *adapter;
	ID3D12Fence *renderFence[RE_NUM_BUFFERS];
	HANDLE fenceEvent;
	UINT64 fenceValue[RE_NUM_BUFFERS];
	ID3D12DescriptorHeap *descHeap[RE_NUM_BUFFERS];
	ID3D12CommandQueueDownlevel *graphicsQueueDownlevel;
	ID3D12DeviceDownlevel *downlevel;
};

struct RenderWorker
{
	ID3D12CommandAllocator *allocators[RE_NUM_BUFFERS];
	ID3D12GraphicsCommandList *cmdList;
	ID3D12GraphicsCommandList4 *rtCmdList;

	ID3D12CommandAllocator *copyAllocators[RE_NUM_BUFFERS];
	ID3D12GraphicsCommandList *copyList;

	ID3D12CommandAllocator *computeAllocators[RE_NUM_BUFFERS];
	ID3D12GraphicsCommandList *computeList;
	ID3D12GraphicsCommandList4 *rtComputeList;
};

struct Pipeline
{
	ID3D12PipelineState *ps;
	ID3D12RootSignature *rs;
};

enum ShaderType
{
	ST_Invalid = 0,
	ST_Graphics,
	ST_Mesh,
	ST_Compute,
	ST_Library
};

struct Shader
{
	uint64_t hash;
	enum ShaderType type;
	union {
		struct {
			D3D12_SHADER_BYTECODE VS;
			D3D12_SHADER_BYTECODE GS;
			D3D12_SHADER_BYTECODE PS;
			D3D12_SHADER_BYTECODE DS;
			D3D12_SHADER_BYTECODE HS;
		};
		struct {
			D3D12_SHADER_BYTECODE AS;
			D3D12_SHADER_BYTECODE MS;
		};
		struct {
			D3D12_SHADER_BYTECODE CS;
		};
		struct {
			D3D12_SHADER_BYTECODE lib;
		};
	};
	wchar_t name[64];
};

struct ModelRenderData
{
	struct AccelerationStructure *structures;
	D3D12_VERTEX_BUFFER_VIEW vtxBufferView;
	D3D12_INDEX_BUFFER_VIEW idxBufferView;
	ID3D12Resource *vtxBuffer, *idxBuffer;
};

struct SceneRenderData
{
	ID3D12Resource *asBuffer, *materialBuffer, *dataBuffer;
	ID3D12DescriptorHeap *vtxHeap, *idxHeap;
	Array instanceData, materialData, buffers;
	UINT64 materialBufferSize, heapSize;
	struct {
		mat4 inverseView;
		mat4 inverseProjection;
		mat4 viewProjection;
		mat4 inverseViewProjection;
		float aspect;
		float aperture;
		uint32_t numSamples;
		uint32_t seed;
		uint32_t environmentMap;
	} shaderData;
};

struct TextureRenderData
{
	ID3D12Resource *res;
	DXGI_FORMAT format;
	uint64_t id;
};

struct AccelerationStructure
{
	ID3D12Resource *asBuffer, *scratchBuffer;
};

struct PrepareInstanceBufferArgs;
struct DrawUIContextArgs;

extern struct GlobalRenderData Re_GlobalRenderData;
extern struct RenderWorker Re_MainThreadWorker;
extern struct Fence Re_UploadFence, Re_ASFence;

bool D3D12_InitDevice(void);
bool D3D12_InitSwapchain(void);
void D3D12_TermDevice(void);

struct RenderWorker *D3D12_CurrentThreadWorker();

bool D3D12_InitStaging(void);
void D3D12_StageUpload(ID3D12Resource *dest, UINT64 size, const void *data, UINT64 alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, UINT64 rowPitch = 0, UINT64 slicePitch = 0);
void *D3D12_AllocStagingArea(UINT64 size, UINT64 alignment, D3D12_GPU_VIRTUAL_ADDRESS *gpuHandle);
void D3D12_StageBLASBuild(struct Model *m, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags);
void D3D12_ResetUploadHeap(void);
void D3D12_BuildBLAS(ID3D12GraphicsCommandList4 *cmdList);
void D3D12_TermStaging();

bool D3D12_InitTextureHeap(void);
void D3D12_UpdateTextureHeap(D3D12_CPU_DESCRIPTOR_HANDLE dest);
void D3D12_CopyTextureDescriptor(uint64_t id, D3D12_CPU_DESCRIPTOR_HANDLE dest);
void D3D12_TermTextureHeap(void);

void D3D12_BuildTLAS(ID3D12GraphicsCommandList4 *cmdList, struct Scene *scene);
void D3D12_UpdateSceneData(struct Scene *scene);
void D3D12_PrepareSceneData(void **comp, struct SceneRenderData *args);

void D3D12_DestroyResource(ID3D12Resource *res);

bool D3D12_InitTransientHeap(void);
ID3D12Resource *D3D12_CreateTransientResource(D3D12_RESOURCE_DESC *desc, D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE *clearValue = NULL);
void D3D12_ResetTransientHeap(void);
void D3D12_TermTransientHeap(void);

bool D3D12_LoadShaders();
void D3D12_UnloadShaders();

bool D3D12_InitUI();
void D3D12_RenderUI(struct Scene *s, ID3D12Resource *output, D3D12_RESOURCE_STATES startState = D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_PRESENT);
void D3D12_TermUI();
void D3D12_DrawUIContext(void **comp, struct DrawUIContextArgs *args);

void D3D12_HandleDeviceRemoved(void);

#endif /* _RE_RENDER_IMPL_H_ */

