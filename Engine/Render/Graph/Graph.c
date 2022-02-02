#include <Engine/Job.h>
#include <Scene/Scene.h>
#include <Runtime/Runtime.h>
#include <Render/Graph/Graph.h>
#include <Render/Graph/Pass.h>
#include <Render/DestroyResource.h>
#include <Render/Driver/TransientResources.h>

#include "../Internal.h"

#define GRAPH_MOD	"RenderGraph"

enum NeGraphResourceType
{
	PRT_TEXTURE,
	PRT_BUFFER,
	PRT_DATA
};

struct NeGraphResource
{
	uint64_t hash;
	struct {
		enum NeGraphResourceType type;
		struct NeTextureDesc texture;
		struct NeBufferDesc buffer;
	} info;
	union {
		struct NeTexture *texture;
		struct {
			uint64_t bufferAddress;
			NeBufferHandle buffer;
			struct NeBuffer *bufferPtr;
		};
		void *hostData;
	} handle;
};

struct NePassData
{
	void *data;
	struct NeRenderPass procs;
};

static struct NeGraphResource *_GetResource(uint64_t hash, const struct NeArray *resources);

bool
Re_AddGraphTexture(const char *name, const struct NeTextureDesc *desc, struct NeArray *resources)
{
	struct NeGraphResource res = { .hash = Rt_HashString(name), .info.type = PRT_TEXTURE, .info.texture = *desc };
	if (_GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

bool
Re_AddGraphBuffer(const char *name, const struct NeBufferDesc *desc, struct NeArray *resources)
{
	struct NeGraphResource res = { .hash = Rt_HashString(name), .info.type = PRT_BUFFER, .info.buffer = *desc };
	if (_GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

bool
Re_AddGraphData(const char *name, void *ptr, struct NeArray *resources)
{
	struct NeGraphResource res = { .hash = Rt_HashString(name), .info.type = PRT_DATA };
	if (_GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

struct NeTexture *
Re_GraphTexture(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *res = _GetResource(hash, resources);
	if (!res)
		return 0;
	return res->info.type == PRT_TEXTURE ? res->handle.texture : NULL;
}

uint64_t
Re_GraphBuffer(uint64_t hash, const struct NeArray *resources, struct NeBuffer **buff)
{
	struct NeGraphResource *res = _GetResource(hash, resources);
	if (!res)
		return 0;

	if (buff)
		*buff = res->handle.bufferPtr;

	return res->info.type == PRT_BUFFER ? res->handle.bufferAddress : 0;
}

void *
Re_GraphData(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *res = _GetResource(hash, resources);
	if (!res)
		return 0;
	return res->info.type == PRT_DATA ? res->handle.hostData : NULL;
}

struct NeRenderGraph *
Re_CreateGraph(void)
{
	struct NeRenderGraph *g = Sys_Alloc(sizeof(*g), 1, MH_Render);
	if (!g)
		return 0;

	Rt_InitArray(&g->allPasses, 10, sizeof(struct NePassData), MH_Render);
	Rt_InitArray(&g->resources, 10, sizeof(struct NeGraphResource), MH_Render);

	g->semaphore = Re_CreateSemaphore();

	return g;
}

bool
Re_AddPass(struct NeRenderGraph *g, struct NeRenderPass *pass)
{
	struct NePassData pd = { .procs = *pass };

	if (!pass->Init(&pd.data))
		return false;

	Rt_ArrayAdd(&g->allPasses, &pd);

	return true;
}

void
Re_BuildGraph(struct NeRenderGraph *g, struct NeTexture *output)
{
	const struct NeScene *s = Scn_activeScene;

	Rt_InitArray(&g->execPasses, g->allPasses.count, g->allPasses.elemSize, MH_Transient);
	Rt_ClearArray(&g->resources, false);

	struct NePassData *pd; 
	Rt_ArrayForEach(pd, &g->allPasses)
		if (pd->procs.Setup(pd->data, &g->resources))
			Rt_ArrayAdd(&g->execPasses, pd);

	uint64_t offset = 0, size = 0;
	struct NeGraphResource *gr;
	Rt_ArrayForEach(gr, &g->resources) {
		if (gr->info.type == PRT_TEXTURE) {
			gr->handle.texture = Re_CreateTransientTexture(&gr->info.texture, offset, &size);
			Re_Destroy(gr->handle.texture);
		} else if (gr->info.type == PRT_BUFFER) {
			if (!gr->handle.buffer)
				Re_ReserveBufferId(&gr->handle.buffer);

			gr->handle.bufferPtr = Re_CreateTransientBuffer(&gr->info.buffer, gr->handle.buffer, offset, &size);
			gr->handle.bufferAddress = Re_BufferAddress(gr->handle.buffer, 0);
			Re_Destroy(gr->handle.buffer);
		} else {
			size = 0;
		}

		offset += size;
	}

	struct NeGraphResource res =
	{
		.hash = Rt_HashString("Re_output"),
		.info.type = PRT_TEXTURE,
		.handle.texture = output
	};
	Rt_ArrayAdd(&g->resources, &res);

	uint64_t sceneAddr, instAddr;
	Scn_DataAddress(s, &sceneAddr, &instAddr);

	res.hash = Rt_HashString("Scn_data");
	res.info.type = PRT_BUFFER,
	res.handle.bufferAddress = sceneAddr;
	res.handle.buffer = s->sceneData;
	Rt_ArrayAdd(&g->resources, &res);

	res.hash = Rt_HashString("Scn_instances");
	res.info.type = PRT_BUFFER,
	res.handle.bufferAddress = instAddr;
	res.handle.buffer = s->sceneData;
	Rt_ArrayAdd(&g->resources, &res);

	res.hash = Rt_HashString("Re_passSemaphore");
	res.info.type = PRT_DATA;
	res.handle.hostData = g->semaphore;
	Rt_ArrayAdd(&g->resources, &res);
}

void
Re_ExecuteGraph(struct NeRenderGraph *g)
{
	struct NePassData *pd; 
	Rt_ArrayForEach(pd, &g->execPasses)
		pd->procs.Execute(pd->data, &g->resources);
}

void
Re_DestroyGraph(struct NeRenderGraph *g)
{
	Rt_TermArray(&g->resources);

	struct NePassData *pd; 
	Rt_ArrayForEach(pd, &g->allPasses)
		pd->procs.Term(pd->data);
	Rt_TermArray(&g->allPasses);

	Re_DestroySemaphore(g->semaphore);

	Sys_Free(g);
}

static struct NeGraphResource *
_GetResource(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *gr;
	Rt_ArrayForEach(gr, resources)
		if (gr->hash == hash)
			return gr;
	return NULL;
}
