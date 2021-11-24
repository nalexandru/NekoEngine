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

extern struct RenderPass RP_depthPrePass;
extern struct RenderPass RP_lightCulling;
extern struct RenderPass RP_ui;
extern struct RenderPass RP_forward;
extern struct RenderPass RP_opaque;
extern struct RenderPass RP_transparent;
extern struct RenderPass RP_sky;

#endif /* _NE_RENDER_GRAPH_PASS_H_ */
