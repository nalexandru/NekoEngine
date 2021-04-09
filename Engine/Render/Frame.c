#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Context.h>
#include <Render/Pipeline.h>
#include <Render/Swapchain.h>
#include <Render/RenderPass.h>
#include <Render/Framebuffer.h>

#include <Engine/Resource.h>

#include <Render/Model.h>
#include <Render/Buffer.h>

uint32_t Re_frameId = 0;

static struct RenderPass *_rp;
static struct Framebuffer *_fb;
static BufferHandle _vertexBuffer, _indexBuffer;
static struct Shader *_shader;
static struct Pipeline *_pipeline;
static Handle _texHandle;

static bool _initialized = false;

static struct Vertex _vertices[] =
{
	{ -.5f, -.5f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
	{ -.5f,  .5f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f },
	{  .5f,  .5f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f },
	{  .5f, -.5f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f }
};
static uint16_t _indices[] =
{
	0, 1, 2, 0, 2, 3
};

void
Re_RenderFrame(void)
{
	if (!_initialized) {
		struct AttachmentDesc atDesc =
		{
			.mayAlias = false,
			.format = Re_SwapchainFormat(Re_swapchain),
			.loadOp = ATL_CLEAR,
			.storeOp = ATS_STORE,
			.samples = ASC_1_SAMPLE,
			.clearColor = { .3f, .0f, .4f, 1.f }
		};
		struct RenderPassDesc desc =
		{
			.attachmentCount = 1,
			.attachments = &atDesc,
		};
		_rp = Re_CreateRenderPass(&desc);
		
		struct BufferCreateInfo vtxInfo =
		{
			.desc =
			{
				.size = sizeof(_vertices),
				.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
				.memoryType = MT_GPU_LOCAL
			},
			.data = _vertices,
			.dataSize = sizeof(_vertices),
			.keepData = true
		};
		Re_CreateBuffer(&vtxInfo, &_vertexBuffer);
		
		struct BufferCreateInfo idxInfo =
		{
			.desc =
			{
				.size = sizeof(_indices),
				.usage = BU_INDEX_BUFFER | BU_TRANSFER_DST,
				.memoryType = MT_GPU_LOCAL
			},
			.data = _indices,
			.dataSize = sizeof(_indices),
			.keepData = true
		};
		Re_CreateBuffer(&idxInfo, &_indexBuffer);
		
		_shader = Re_GetShader("DefaultPBR_MR");
		
		struct BlendAttachmentDesc blendAttachments[] =
		{
			{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGB }
		};
		struct GraphicsPipelineDesc pipeDesc =
		{
			.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
			.shader = _shader,
			.renderPass = _rp,
			.pushConstantSize = 0,
			.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
			.attachments = blendAttachments
		};
		_pipeline = Re_GraphicsPipeline(&pipeDesc);
		
		_texHandle = E_LoadResource("/Textures/Anna.tga", RES_TEXTURE);
		
		_initialized = true;
	}
	
	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;

	for (int32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_ResetContext(Re_contexts[i]);
	Re_DestroyResources();
	
	Re_BeginDrawCommandBuffer();

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
	Re_SetAttachment(_fb, 0, Re_SwapchainTexture(Re_swapchain, image));

	Re_BeginRenderPass(_rp, _fb);
	
	Re_Destroy(_fb);
	
	Re_BindPipeline(_pipeline);
	Re_BindIndexBuffer(_indexBuffer, 0, IT_UINT_16);

	Re_SetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_SetScissor(0, 0, *E_screenWidth, *E_screenHeight);
	
	Re_DrawIndexed(6, 1, 0, 0, 0);
	
	Re_EndRenderPass();
	Re_EndCommandBuffer();
	
	Re_Present(Re_swapchain, image);
	
	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}

