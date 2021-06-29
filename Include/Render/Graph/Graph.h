#ifndef _NE_RENDER_GRAPH_GRAPH_H_
#define _NE_RENDER_GRAPH_GRAPH_H_

#include <Render/Types.h>
#include <Render/Driver/Core.h>

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
		BufferHandle buffer;
	} handle;
};

bool Re_AddGraphTexture(const char *name, const struct TextureDesc *desc, struct Array *resources);
bool Re_AddGraphBuffer(const char *name, const struct BufferDesc *desc, struct Array *resources);

struct Texture *Re_GraphTexture(uint64_t hash, const struct Array *resources);
BufferHandle Re_GraphBuffer(uint64_t hash, const struct Array *resources);

void Re_BuildGraph(struct RenderGraph *graph);
void Re_ExecuteGraph(struct RenderGraph *graph);

#endif /* _NE_RENDER_GRAPH_GRAPH_H_ */
