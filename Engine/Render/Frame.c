#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Context.h>
#include <Render/Pipeline.h>
#include <Render/Swapchain.h>
#include <Render/RenderPass.h>
#include <Render/DescriptorSet.h>

#include <Render/Model.h>
#include <Render/Buffer.h>

uint32_t Re_frameId = 0;

static struct RenderPass *_rp;
static struct Framebuffer *_fb;
static struct Buffer *_vertexBuffer;
static struct Shader *_shader;
static struct Pipeline *_pipeline;
static struct DescriptorSetLayout *_dsLayout;
static struct PipelineLayout *_pLayout;
static struct DescriptorSet *_ds;

static bool _initialized = false;

static struct Vertex _vertices[] =
{
	{  .5f, -.5f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{ -.5f, -.5f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{  .0f,  .5f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
};

void
Re_RenderFrame(void)
{
	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;
	
	if (!_initialized) {
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
		
		struct FramebufferAttachmentDesc fbAtDesc =
		{
			.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
			.format = Re_SwapchainFormat(Re_swapchain)
		};
		
		struct FramebufferDesc fbDesc =
		{
			.attachmentCount = 1,
			.attachments = &fbAtDesc,
			.width = *E_screenWidth,
			.height = *E_screenHeight,
			.layers = 1,
			.renderPass = _rp
		};

		_fb = Re_CreateFramebuffer(&fbDesc);
		
		struct BufferCreateInfo vtxInfo =
		{
			.desc =
			{
				.size = sizeof(_vertices),
				.usage = BU_VERTEX_BUFFER | BU_STORAGE_BUFFER | BU_TRANSFER_DST,
				.memoryType = MT_CPU_COHERENT
			},
			.data = _vertices,
			.dataSize = sizeof(_vertices),
			.keepData = true
		};
		_vertexBuffer = Re_CreateBuffer(&vtxInfo);
		
		_shader = Re_GetShader("DefaultPBR_MR");
		
		struct DescriptorBinding bindings[] =
		{
			{ DT_STORAGE_BUFFER, 1, SS_VERTEX }
		};
		struct DescriptorSetLayoutDesc dslDesc =
		{
			.bindingCount = sizeof(bindings) / sizeof(bindings[0]),
			.bindings = bindings
		};
		_dsLayout = Re_CreateDescriptorSetLayout(&dslDesc);
		
		struct PipelineLayoutDesc plDesc =
		{
			.setLayoutCount = 1,
			.setLayouts = (const struct DescriptorSetLayout **)&_dsLayout,
			.pushConstantSize = 0
		};
		_pLayout = Re_CreatePipelineLayout(&plDesc);
		
		struct BlendAttachmentDesc blendAttachments[] =
		{
			{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGB }
		};
		struct GraphicsPipelineDesc pipeDesc =
		{
			.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
			.shader = _shader,
			.renderPass = _rp,
			.layout = _pLayout,
			.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
			.attachments = blendAttachments
		};
		_pipeline = Re_GraphicsPipeline(&pipeDesc);
		
		_ds = Re_CreateDescriptorSet(_dsLayout);
		
		struct BufferBindInfo bindInfo =
		{
			.buff = _vertexBuffer,
			.offset = 0,
			.size = sizeof(_vertices)
		};
		struct DescriptorWrite dw =
		{
			.type = DT_STORAGE_BUFFER,
			.binding = 0,
			.count = 1,
			.bufferInfo = &bindInfo
		};
		Re_WriteDescriptorSet(_ds, &dw, 1);
		
		_initialized = true;
	}
	
	Re_SetAttachment(_fb, 0, Re_SwapchainTexture(Re_swapchain, image));
	
	Re_BeginDrawCommandBuffer();
	Re_BeginRenderPass(_rp, _fb);
	
	Re_BindPipeline(_pipeline);
	Re_BindDescriptorSets(_pLayout, 0, 1, _ds);

	Re_SetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_SetScissor(0, 0, *E_screenWidth, *E_screenHeight);
	
	Re_Draw(3, 1, 0, 0);
	
	Re_EndRenderPass();
	Re_EndCommandBuffer();
	
	Re_Present(Re_swapchain, image);
	
	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}
