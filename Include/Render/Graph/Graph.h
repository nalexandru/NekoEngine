#ifndef _NE_RENDER_GRAPH_GRAPH_H_
#define _NE_RENDER_GRAPH_GRAPH_H_

#include <Render/Types.h>
#include <Render/Driver/Core.h>

extern struct NeRenderGraph *Re_activeGraph;

bool Re_AddGraphTexture(const char *name, const struct NeTextureDesc *desc, struct NeArray *resources);
bool Re_AddGraphBuffer(const char *name, const struct NeBufferDesc *desc, struct NeArray *resources);
bool Re_AddGraphData(const char *name, void *ptr, struct NeArray *resources);

struct NeTexture *Re_GraphTexture(uint64_t hash, const struct NeArray *resources);
uint64_t Re_GraphBuffer(uint64_t hash, const struct NeArray *resources, struct NeBuffer **buff);
void *Re_GraphData(uint64_t hash, const struct NeArray *resources);

struct NeRenderGraph *Re_CreateGraph(void);
bool Re_AddPass(struct NeRenderGraph *g, struct NeRenderPass *pass);
void Re_BuildGraph(struct NeRenderGraph *g, struct NeTexture *output);
void Re_ExecuteGraph(struct NeRenderGraph *g);
void Re_DestroyGraph(struct NeRenderGraph *g);

#endif /* _NE_RENDER_GRAPH_GRAPH_H_ */
