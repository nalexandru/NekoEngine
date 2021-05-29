#ifndef _NE_RENDER_RENDER_H_
#define _NE_RENDER_RENDER_H_

#include <Render/Types.h>
#include <Render/Driver/Core.h>
#include <Render/Driver/Device.h>
#include <Render/Driver/Context.h>
#include <Render/Driver/RenderPassDesc.h>
#include <Render/Driver/TransientResources.h>

#include <Render/Model.h>
#include <Render/Material.h>
#include <Render/DestroyResource.h>

#define RE_NUM_FRAMES	3

ENGINE_API extern uint32_t Re_frameId;
ENGINE_API extern struct Surface *Re_surface;

bool Re_InitRender(void);
void Re_RenderFrame(void);
void Re_TermRender(void);

#endif /* _NE_RENDER_RENDER_H_ */
