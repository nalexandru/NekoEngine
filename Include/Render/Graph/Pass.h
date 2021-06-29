#ifndef _NE_RENDER_GRAPH_PASS_H_
#define _NE_RENDER_GRAPH_PASS_H_

#include <Render/Types.h>
#include <Render/Driver/Core.h>
#include <Render/Driver/RenderPassDesc.h>
#include <Runtime/Runtime.h>

typedef bool (*PassInitProc)(void **);
typedef void (*PassTermProc)(void *);
typedef bool (*PassSetupProc)(void *, struct Array *);
typedef void (*PassExecuteProc)(void *, const struct Array *);

struct RenderPass
{
	PassInitProc Init;
	PassTermProc Term;

	PassSetupProc Setup;
	PassExecuteProc Execute;
};

/*enum PassAttachmentUsage
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

};*/

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
