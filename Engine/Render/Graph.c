#include <Engine/Job.h>
#include <Runtime/Runtime.h>
#include <Render/Graph/Graph.h>
#include <Render/Graph/Pass.h>

struct RenderGraph
{
	uint32_t passCount;
	struct RenderGraphPass *passes;
};

struct BuildJobArgs
{
	uint32_t count;
	struct RenderGraphPass *passes;
//	struct Array passes;
};

typedef void (*JobProc)(int, void *);
static void _BuildPassJob(int worker, struct BuildJobArgs *args);

static struct RenderGraphPass *_renderPasses;


void
Re_BuildGraph(struct RenderGraph *graph)
{
	for (uint32_t i = 0; i < graph->passCount; ++i) {
		
	}
}

void
Re_ExecuteGraph(struct RenderGraph *graph)
{
	for (uint32_t i = 0; i < graph->passCount; ++i) {
		graph->passes[i].Execute(graph, NULL);
	}
}

static void
_BuildPassJob(int worker, struct BuildJobArgs *args)
{
	for (uint32_t i = 0; i < args->count; ++i)
		args->passes[i].Setup(NULL);
}

