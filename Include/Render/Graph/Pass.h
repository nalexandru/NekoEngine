#ifndef _RE_GRAPH_PASS_H_
#define _RE_GRAPH_PASS_H_

#include <Render/Types.h>
#include <Render/RenderPass.h>

struct RenderGraphPass
{
	void (*Setup)(void *graph);
	void (*Execute)(void *graph, void *graphResources);
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

#endif /* _RE_GRAPH_PASS_H_ */

