#include <Engine/Job.h>
#include <Runtime/Runtime.h>
#include <Render/Graph/Graph.h>
#include <Render/Graph/Pass.h>

struct RenderGraph
{
	uint32_t passCount;
	struct RenderPass *passes;
};

struct BuildJobArgs
{
	uint32_t count;
	struct RenderPass *passes;
//	struct Array passes;
};

typedef void (*JobProc)(int, void *);
static void _BuildPassJob(int worker, struct BuildJobArgs *args);
static struct GraphResource *_GetResource(uint64_t hash, const struct Array *resources);

static struct RenderPass *_renderPasses;

bool
Re_AddGraphTexture(const char *name, const struct TextureDesc *desc, struct Array *resources)
{
	struct GraphResource res = { .hash = Rt_HashString(name) };
	if (_GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

bool
Re_AddGraphBuffer(const char *name, const struct BufferDesc *desc, struct Array *resources)
{
	struct GraphResource res = { .hash = Rt_HashString(name) };
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
	return res->handle.texture;
}

BufferHandle
Re_GraphBuffer(uint64_t hash, const struct Array *resources)
{
	struct GraphResource *res = _GetResource(hash, resources);
	if (!res)
		return 0;
	return res->handle.buffer;
}

void
Re_BuildGraph(struct RenderGraph *graph)
{
	for (uint32_t i = 0; i < graph->passCount; ++i) {
		// run setup
	}

	// create transient resources
}

void
Re_ExecuteGraph(struct RenderGraph *graph)
{
	for (uint32_t i = 0; i < graph->passCount; ++i) {
//		graph->passes[i].Execute(graph, NULL);
	}
}

static void
_BuildPassJob(int worker, struct BuildJobArgs *args)
{
	for (uint32_t i = 0; i < args->count; ++i)
		args->passes[i].Setup(NULL, NULL);
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
