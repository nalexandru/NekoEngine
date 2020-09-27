#include <UI/UI.h>
#include <Render/Shader.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>

#include "D3D9Render.h"

struct UIContextLoadArgs
{
	uint8_t *vtxPtr;
	uint8_t *idxPtr;
};

static IDirect3DVertexDeclaration9 *_vtxDecl;
static IDirect3DVertexBuffer9 *_vtxBuffer;
static IDirect3DIndexBuffer9 *_idxBuffer;
static struct Shader *_uiShader;

bool
D3D9_InitUI(void)
{
	D3DVERTEXELEMENT9 vtxDesc[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	if (FAILED(Re_Device.dev->CreateVertexDeclaration(vtxDesc, &_vtxDecl)))
		return false;

	Re_Device.dev->CreateVertexBuffer(sizeof(UIVertex) * 4000, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_vtxBuffer, NULL);
	Re_Device.dev->CreateIndexBuffer(sizeof(uint16_t) * 6000, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &_idxBuffer, NULL);

	_uiShader = (struct Shader *)Re_GetShader(Rt_HashStringW(L"UI"));

	return true;
}

void
D3D9_TermUI(void)
{
	if (_vtxBuffer)
		_vtxBuffer->Release();

	if (_idxBuffer)
		_idxBuffer->Release();
}

void
D3D9_RenderUI(struct Scene *s)
{
	struct UIContextLoadArgs loadArgs;

	_vtxBuffer->Lock(0, 0, (void **)&loadArgs.vtxPtr, D3DLOCK_DISCARD);
	_idxBuffer->Lock(0, 0, (void **)&loadArgs.idxPtr, D3DLOCK_DISCARD);

	E_ExecuteSystemS(s, LOAD_UI_CONTEXT, &loadArgs);

	_vtxBuffer->Unlock();
	_idxBuffer->Unlock();

	Re_Device.dev->SetStreamSource(0, _vtxBuffer, 0, sizeof(struct UIVertex));
	Re_Device.dev->SetVertexDeclaration(_vtxDecl);
	Re_Device.dev->SetIndices(_idxBuffer);

	Re_Device.dev->SetVertexShader(_uiShader->vs);
	Re_Device.dev->SetPixelShader(_uiShader->ps);

	Re_Device.dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	Re_Device.dev->SetRenderState(D3DRS_ZENABLE, FALSE);

	Re_Device.dev->SetVertexShaderConstantF(0, (const float *)&UI_Projection, 4);

	Re_Device.dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Re_Device.dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	Re_Device.dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Re_Device.dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	E_ExecuteSystemS(s, DRAW_UI_CONTEXT, NULL);

	Re_Device.dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	Re_Device.dev->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void
D3D9_LoadUIContext(void **comp, struct UIContextLoadArgs *args)
{
	struct UIContext *ctx = (struct UIContext *)comp[0];

	const size_t vtxSize = sizeof(UIVertex) * ctx->vertices.count;
	const size_t idxSize = sizeof(uint16_t) * ctx->indices.count;

	memcpy(args->vtxPtr, ctx->vertices.data, vtxSize);
	args->vtxPtr += vtxSize;

	memcpy(args->idxPtr, ctx->indices.data, idxSize);
	args->idxPtr += idxSize;
}

void
D3D9_DrawUIContext(void **comp, void *args)
{
	struct UIContext *ctx = (struct UIContext *)comp[0];

	for (size_t i = 0; i < ctx->draws.count; ++i) {
		const struct UIDrawCall *drawCall = (struct UIDrawCall *)Rt_ArrayGet(&ctx->draws, i);

		struct TextureRenderData *trd = (struct TextureRenderData *)&((struct Texture *)E_ResourcePtr(drawCall->texture))->renderDataStart;
		Re_Device.dev->SetTexture(0, trd->tex);

		Re_Device.dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, drawCall->vtxCount, drawCall->idxOffset, drawCall->idxCount / 3);
	}
}
