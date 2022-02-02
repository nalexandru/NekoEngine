#ifndef _NE_RENDER_GRAPH_PASS_H_
#define _NE_RENDER_GRAPH_PASS_H_

#include <Render/Types.h>
#include <Render/Driver/Core.h>
#include <Render/Driver/RenderPassDesc.h>
#include <Runtime/Runtime.h>

typedef bool (*NePassInitProc)(void **);
typedef void (*NePassTermProc)(void *);
typedef bool (*NePassSetupProc)(void *, struct NeArray *);
typedef void (*NePassExecuteProc)(void *, const struct NeArray *);

struct NeRenderPass
{
	NePassInitProc Init;
	NePassTermProc Term;

	NePassSetupProc Setup;
	NePassExecuteProc Execute;
};

extern struct NeRenderPass RP_depthPrePass;
extern struct NeRenderPass RP_lightCulling;
extern struct NeRenderPass RP_ui;
extern struct NeRenderPass RP_opaque;
extern struct NeRenderPass RP_transparent;
extern struct NeRenderPass RP_sky;

#endif /* _NE_RENDER_GRAPH_PASS_H_ */
