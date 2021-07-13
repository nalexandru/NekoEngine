#ifndef _NE_RENDER_GRAPH_GRAPH_H_
#define _NE_RENDER_GRAPH_GRAPH_H_

#include <Render/Types.h>
#include <Render/Driver/Core.h>

extern struct RenderGraph *Re_activeGraph;

bool Re_AddGraphTexture(const char *name, const struct TextureDesc *desc, struct Array *resources);
bool Re_AddGraphBuffer(const char *name, const struct BufferDesc *desc, struct Array *resources);

struct Texture *Re_GraphTexture(uint64_t hash, const struct Array *resources);
BufferHandle Re_GraphBuffer(uint64_t hash, const struct Array *resources);

struct RenderGraph *Re_CreateGraph(void);
bool Re_AddPass(struct RenderGraph *g, struct RenderPass *pass);
void Re_BuildGraph(struct RenderGraph *g, struct Texture *output);
void Re_ExecuteGraph(struct RenderGraph *g);
void Re_DestroyGraph(struct RenderGraph *g);

#endif /* _NE_RENDER_GRAPH_GRAPH_H_ */
