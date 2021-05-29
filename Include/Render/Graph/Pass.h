#ifndef _NE_RENDER_GRAPH_PASS_H_
#define _NE_RENDER_GRAPH_PASS_H_

#include <Render/Types.h>
#include <Render/Driver/RenderPassDesc.h>
#include <Runtime/Runtime.h>

struct PassResource
{
	uint64_t hash;
	union {
		TextureHandle texHandle;
		BufferHandle buffer;
	};
};

enum PassAttachmentUsage
{
	ATU_INPUT,
	ATU_COLOR,
	ATU_DEPTH,
	ATU_STENCIL
};

struct PassAttachment
{
	uint64_t hash;
	enum PassAttachmentUsage usage;

};

struct RenderGraphPass
{
	void (*Setup)(struct Array *resources);
	void (*Execute)(const struct Array *resources);
};

/*struct MaterialPassDesc
{
	//struct RenderPassTarget colorAttachments[8];
//	struct RenderPassTarget depthAttachment;
};

struct FullscreenPassDesc
{
//	struct RenderPassInput inputAttachments[8];
	struct RenderPassTarget colorAttachments[8];
	struct RenderPassTarget depthAttachment;
};*/

#endif /* _NE_RENDER_GRAPH_PASS_H_ */
