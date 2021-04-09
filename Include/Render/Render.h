#ifndef _RE_RENDER_H_
#define _RE_RENDER_H_

#include <Render/Types.h>
#include <Render/Model.h>
#include <Render/Buffer.h>
#include <Render/Device.h>
#include <Render/Shader.h>
#include <Render/Texture.h>
#include <Render/Context.h>
#include <Render/Sampler.h>
#include <Render/Material.h>
#include <Render/Pipeline.h>
#include <Render/Swapchain.h>
#include <Render/RenderPass.h>
#include <Render/Framebuffer.h>
#include <Render/DestroyResource.h>
#include <Render/TransientResources.h>
#include <Render/ShaderBindingTable.h>
#include <Render/AccelerationStructure.h>

#define RE_NUM_FRAMES	3

ENGINE_API extern uint32_t Re_frameId;
ENGINE_API extern struct Surface *Re_surface;

bool Re_InitRender(void);
void Re_RenderFrame(void);
void Re_TermRender(void);

#endif /* _RE_RENDER_H_ */

