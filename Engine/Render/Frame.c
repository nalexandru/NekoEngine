#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Context.h>
#include <Render/Swapchain.h>
#include <Render/RenderPass.h>

uint32_t Re_frameId = 0;

static struct RenderPass *_rp;
static struct Framebuffer *_fb;

void
Re_RenderFrame(void)
{
	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;
	
	if (!_rp) {
		struct AttachmentDesc atDesc =
		{
			.mayAlias = false,
			.format = Re_SwapchainFormat(Re_swapchain),
			.loadOp = ATL_CLEAR,
			.storeOp = ATS_STORE,
			.samples = ASC_1_SAMPLE
		};
		
		struct RenderPassDesc desc =
		{
			.attachmentCount = 1,
			.attachments = &atDesc,
		};
		
		_rp = Re_CreateRenderPass(&desc);
	}
	
	if (!_fb) {
		struct FramebufferAttachmentDesc atDesc =
		{
			.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
			.format = Re_SwapchainFormat(Re_swapchain)
		};
		
		struct FramebufferDesc desc =
		{
			.attachmentCount = 1,
			.attachments = &atDesc,
			.width = *E_screenWidth,
			.height = *E_screenHeight,
			.layers = 1,
			.renderPass = _rp
		};

		_fb = Re_CreateFramebuffer(&desc);
	}
	
	Re_SetAttachment(_fb, 0, Re_SwapchainTexture(Re_swapchain, image));
	
	Re_BeginDrawCommandBuffer();
	Re_BeginRenderPass(_rp, _fb);
	
	Re_EndRenderPass();
	Re_EndCommandBuffer();
	
	Re_Present(Re_swapchain, image);
	
	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}
