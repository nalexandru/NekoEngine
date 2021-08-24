#include <Engine/Job.h>
#include <Scene/Scene.h>
#include <Runtime/Runtime.h>
#include <Render/Graph/Graph.h>
#include <Render/Graph/Pass.h>
#include <Render/DestroyResource.h>
#include <Render/Driver/TransientResources.h>

enum GraphResourceType
{
	PRT_TEXTURE,
	PRT_BUFFER
};

struct GraphResource
{
	uint64_t hash;
	struct {
		enum GraphResourceType type;
		struct TextureDesc texture;
		struct BufferDesc buffer;
	} info;
	union {
		struct Texture *texture;
		struct {
			uint64_t bufferAddress;
			BufferHandle buffer;
			struct Buffer *ptr;
		};
	} handle;
};

struct PassData
{
	void *data;
	struct RenderPass procs;
};

struct RenderGraph
{
	struct Array execPasses;
	struct Array resources;
	struct Array allPasses;
};

static struct GraphResource *_GetResource(uint64_t hash, const struct Array *resources);

bool
Re_AddGraphTexture(const char *name, const struct TextureDesc *desc, struct Array *resources)
{
	struct GraphResource res = { .hash = Rt_HashString(name), .info.type = PRT_TEXTURE, .info.texture = *desc };
	if (_GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

bool
Re_AddGraphBuffer(const char *name, const struct BufferDesc *desc, struct Array *resources)
{
	struct GraphResource res = { .hash = Rt_HashString(name), .info.type = PRT_BUFFER, .info.buffer = *desc };
	if (_GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

struct Texture *
Re_GraphTexture(uint64_t hash, const struct Array *resources)
{
	struct GraphResource *res = _GetResource(hash, resources);
	if (!res)
		return 0;
	return res->info.type == PRT_TEXTURE ? res->handle.texture : NULL;
}

uint64_t
Re_GraphBuffer(uint64_t hash, const struct Array *resources, struct Buffer **buff)
{
	struct GraphResource *res = _GetResource(hash, resources);
	if (!res)
		return 0;

	if (buff)
		*buff = res->handle.ptr;

	return res->info.type == PRT_BUFFER ? res->handle.bufferAddress : 0;
}

struct RenderGraph *
Re_CreateGraph(void)
{
	struct RenderGraph *g = Sys_Alloc(sizeof(*g), 1, MH_Render);
	if (!g)
		return 0;

	Rt_InitArray(&g->allPasses, 10, sizeof(struct PassData), MH_Render);
	Rt_InitArray(&g->resources, 10, sizeof(struct GraphResource), MH_Render);

	return g;
}

bool
Re_AddPass(struct RenderGraph *g, struct RenderPass *pass)
{
	struct PassData pd = { .procs = *pass };

	if (!pass->Init(&pd.data))
		return false;

	Rt_ArrayAdd(&g->allPasses, &pd);

	return true;
}

void
Re_BuildGraph(struct RenderGraph *g, struct Texture *output)
{
	const struct Scene *s = Scn_activeScene;

	Rt_InitArray(&g->execPasses, g->allPasses.count, g->allPasses.elemSize, MH_Transient);
	Rt_ClearArray(&g->resources, false);

	struct PassData *pd; 
	Rt_ArrayForEach(pd, &g->allPasses)
		if (pd->procs.Setup(pd->data, &g->resources))
			Rt_ArrayAdd(&g->execPasses, pd);

	uint64_t offset = 0, size = 0;
	struct GraphResource *gr;
	Rt_ArrayForEach(gr, &g->resources) {
		if (gr->info.type == PRT_TEXTURE) {
			gr->handle.texture = Re_CreateTransientTexture(&gr->info.texture, offset, &size);
			Re_Destroy(gr->handle.texture);
		} else if (gr->info.type == PRT_BUFFER) {
			if (!gr->handle.buffer)
				Re_ReserveBufferId(&gr->handle.buffer);

			gr->handle.ptr = Re_CreateTransientBuffer(&gr->info.buffer, gr->handle.buffer, offset, &size);
			Re_Destroy(gr->handle.buffer);
		} else {
			size = 0;
		}

		offset += size;
	}

	struct GraphResource res =
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
}

void
Re_ExecuteGraph(struct RenderGraph *g)
{
	struct PassData *pd; 
	Rt_ArrayForEach(pd, &g->execPasses)
		pd->procs.Execute(pd->data, &g->resources);
}

void
Re_DestroyGraph(struct RenderGraph *g)
{
	Rt_TermArray(&g->resources);

	struct PassData *pd; 
	Rt_ArrayForEach(pd, &g->allPasses)
		pd->procs.Term(pd->data);
	Rt_TermArray(&g->allPasses);

	Sys_Free(g);
}

static struct GraphResource *
_GetResource(uint64_t hash, const struct Array *resources)
{
	struct GraphResource *gr;
	Rt_ArrayForEach(gr, resources)
		if (gr->hash == hash)
			return gr;
	return NULL;
}
