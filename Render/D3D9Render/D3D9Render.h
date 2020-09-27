#pragma once

#ifdef _XBOX
#	include <xtl.h>
#endif

#include <d3d9.h>

#include <Runtime/Array.h>
#include <Engine/Component.h>

#define GET_DRAWABLES_SYS	L"D3D9GetDrawables"
#define LOAD_UI_CONTEXT		L"D3D9_LoadUIContext"
#define DRAW_UI_CONTEXT		L"D3D9_DrawUIContext"

struct RenderDevice
{
	IDirect3DDevice9 *dev;
	IDirect3D9 *d3d;
	D3DCAPS9 caps;
};

struct Shader
{
	IDirect3DVertexDeclaration9 *vd;
	IDirect3DVertexShader9 *vs;
	IDirect3DPixelShader9 *ps;
	uint64_t hash;
};

struct MaterialRenderData
{
	struct Shader *shader;
};

struct ModelRenderData
{
	IDirect3DVertexBuffer9 *vtxBuffer;
	IDirect3DIndexBuffer9 *idxBuffer;
	struct MaterialRenderData *materialData;
};

struct ModelRenderer
{
	COMPONENT_BASE;

	struct Model *mesh;
};

struct SceneRenderData
{
	Array drawables;
	/*ID3D12Resource *topLevelAS;
	ID3D12Resource *scratchBuffer;
	ID3D12Resource *instances;
	UINT64 asBufferSize, scratchBufferSize, instanceBufferSize;*/
};

struct TextureRenderData
{
	union {
		IDirect3DTexture9 *tex;
		IDirect3DBaseTexture9 *baseTex;
		IDirect3DCubeTexture9 *cubeTex;
		IDirect3DVolumeTexture9 *volTex;
	};
};

struct GetDrawablesArgs;
struct UIContextLoadArgs;

extern IDirect3DVertexDeclaration9 *D3D9_VertexDeclaration;

void D3D9_GetDrawables(void **comp, struct GetDrawablesArgs *args);

bool D3D9_LoadShaders(void);
void D3D9_UnloadShaders(void);

void D3D9_RenderScene(struct Scene *scene);

bool D3D9_InitUI(void);
void D3D9_RenderUI(struct Scene *scene);
void D3D9_TermUI(void);
void D3D9_LoadUIContext(void **comp, struct UIContextLoadArgs *args);
void D3D9_DrawUIContext(void **comp, void *args);

