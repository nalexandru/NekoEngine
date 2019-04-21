/* NekoEngine
 *
 * renderpass.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Vulkan Graphics Module
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <vkgfx.h>
#include <debug.h>
#include <vkutil.h>
#include <swapchain.h>
#include <renderpass.h>

#define RP_MODULE	"Vulkan_RenderPass"

struct vkgfx_rp_state
{
	VkRenderPass rp;
	int32_t state_count;
	VkPipelineColorBlendAttachmentState states[MAX_RP_CBA_STATES];
};

static struct vkgfx_rp_state _rp[RP_ENUM_COUNT];

static inline ne_status
_create_depth_rp(void)
{
	VkResult res;

	VkAttachmentDescription at[2];
	memset(at, 0x0, sizeof(at));

	at[0].format = vkgfx_render_target.normal.format;
	at[0].samples = vkgfx_render_target.samples;
	at[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	at[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	at[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	at[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	at[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	at[1].format = vkgfx_render_target.depth.format;
	at[1].samples = vkgfx_render_target.samples;
	at[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	at[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	at[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	at[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	at[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	at[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference normal_ref;
	memset(&normal_ref, 0x0, sizeof(normal_ref));
	normal_ref.attachment = 0;
	normal_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_ref;
	memset(&depth_ref, 0x0, sizeof(depth_ref));
	depth_ref.attachment = 1;
	depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription sp;
	memset(&sp, 0x0, sizeof(sp));
	sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sp.colorAttachmentCount = 1;
	sp.pColorAttachments = &normal_ref;
	sp.pDepthStencilAttachment = &depth_ref;

	VkSubpassDependency sp_dep[2];
	memset(sp_dep, 0x0, sizeof(sp_dep));
	sp_dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	sp_dep[0].dstSubpass = 0;
	sp_dep[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sp_dep[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_dep[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sp_dep[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sp_dep[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	sp_dep[1].srcSubpass = 0;
	sp_dep[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	sp_dep[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_dep[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sp_dep[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sp_dep[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sp_dep[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	ci.attachmentCount = 2;
	ci.pAttachments = at;
	ci.subpassCount = 1;
	ci.pSubpasses = &sp;
	ci.dependencyCount = 2;
	ci.pDependencies = sp_dep;

	if ((res = vkCreateRenderPass(vkgfx_device, &ci, vkgfx_allocator,
		&_rp[RP_DEPTH].rp)) != VK_SUCCESS) {
		log_entry(RP_MODULE, LOG_CRITICAL,
			"Failed to create GUI render pass: %s",
			vku_result_string(res));
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME(_rp[RP_DEPTH],
		VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, "Depth");

	return NE_OK;
}

static inline ne_status
_create_lighting_rp(void)
{
	VkResult res;

	VkAttachmentDescription at[3];
	memset(at, 0x0, sizeof(at));

	at[0].format = vkgfx_render_target.color.format;
	at[0].samples = vkgfx_render_target.samples;
	at[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	at[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	at[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	at[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	at[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	at[1].format = vkgfx_render_target.depth.format;
	at[1].samples = vkgfx_render_target.samples;
	at[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	at[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	at[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	at[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	at[2].format = vkgfx_render_target.resolve.format;
	at[2].samples = VK_SAMPLE_COUNT_1_BIT;
	at[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	at[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	at[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	at[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at[2].initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	at[2].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	VkAttachmentReference color_ref;
	memset(&color_ref, 0x0, sizeof(color_ref));
	color_ref.attachment = 0;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_ref;
	memset(&depth_ref, 0x0, sizeof(depth_ref));
	depth_ref.attachment = 1;
	depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference resolve_ref;
	memset(&resolve_ref, 0x0, sizeof(resolve_ref));
	resolve_ref.attachment = 2;
	resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription sp;
	memset(&sp, 0x0, sizeof(sp));
	sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sp.colorAttachmentCount = 1;
	sp.pColorAttachments = &color_ref;
	sp.pDepthStencilAttachment = &depth_ref;
	sp.pResolveAttachments = &resolve_ref;

	VkSubpassDependency sp_dep[2];
	memset(sp_dep, 0x0, sizeof(sp_dep));
	sp_dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	sp_dep[0].dstSubpass = 0;
	sp_dep[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sp_dep[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_dep[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sp_dep[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sp_dep[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	sp_dep[1].srcSubpass = 0;
	sp_dep[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	sp_dep[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_dep[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sp_dep[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sp_dep[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sp_dep[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	ci.attachmentCount = 3;
	ci.pAttachments = at;
	ci.subpassCount = 1;
	ci.pSubpasses = &sp;
	ci.dependencyCount = 2;
	ci.pDependencies = sp_dep;

	if ((res = vkCreateRenderPass(vkgfx_device, &ci, vkgfx_allocator,
		&_rp[RP_LIGHTING].rp)) != VK_SUCCESS) {
		log_entry(RP_MODULE, LOG_CRITICAL,
			"Failed to create scene render pass: %s",
			vku_result_string(res));
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME(_rp[RP_LIGHTING],
		VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, "Lighting");

	_rp[RP_LIGHTING].state_count = 0;

	return NE_OK;
}

static inline ne_status
_create_pp_rp(void)
{
	return NE_OK;
}

static inline ne_status
_create_gui_rp(void)
{
	VkResult res;

	VkAttachmentDescription color;
	memset(&color, 0x0, sizeof(color));
	color.format = sw_format();
	color.samples = VK_SAMPLE_COUNT_1_BIT;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_ref;
	memset(&color_ref, 0x0, sizeof(color_ref));
	color_ref.attachment = 0;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription sp;
	memset(&sp, 0x0, sizeof(sp));
	sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sp.colorAttachmentCount = 1;
	sp.pColorAttachments = &color_ref;
	sp.pDepthStencilAttachment = NULL;

	VkSubpassDependency sp_dep[2];
	memset(sp_dep, 0x0, sizeof(sp_dep));
	sp_dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	sp_dep[0].dstSubpass = 0;
	sp_dep[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sp_dep[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_dep[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sp_dep[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sp_dep[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

/*	sp_dep[1].srcSubpass = 0;
	sp_dep[1].dstSubpass = 0;
	sp_dep[1].srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	sp_dep[1].dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	sp_dep[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	sp_dep[1].dstAccessMask = VK_ACCESS_INDEX_READ_BIT |
		VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	sp_dep[1].dependencyFlags = 0;*/

	sp_dep[1].srcSubpass = 0;
	sp_dep[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	sp_dep[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_dep[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sp_dep[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sp_dep[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sp_dep[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	ci.attachmentCount = 1;
	ci.pAttachments = &color;
	ci.subpassCount = 1;
	ci.pSubpasses = &sp;
	ci.dependencyCount = 2;
	ci.pDependencies = sp_dep;

	if ((res = vkCreateRenderPass(vkgfx_device, &ci, vkgfx_allocator,
		&_rp[RP_GUI].rp)) != VK_SUCCESS) {
		log_entry(RP_MODULE, LOG_CRITICAL,
			"Failed to create GUI render pass: %s",
			vku_result_string(res));
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME(_rp[RP_GUI],
		VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, "GUI");

	_rp[RP_GUI].state_count = 0;

	return NE_OK;
}

ne_status
vkgfx_init_renderpass(void)
{
	ne_status ret;

	memset(_rp, 0x0, sizeof(_rp));

	if ((ret = _create_depth_rp()) != NE_OK)
		return ret;

	if ((ret = _create_lighting_rp()) != NE_OK)
		return ret;

	if ((ret = _create_pp_rp()) != NE_OK)
		return ret;

	if ((ret = _create_gui_rp()) != NE_OK)
		return ret;

	return ret;
}

VkRenderPass
rp_get(vkgfx_rp id)
{
	return _rp[id].rp;
}

const VkPipelineColorBlendAttachmentState *
rp_get_extra_attachment_states(
	VkRenderPass p,
	int32_t *count)
{
	for (uint8_t i = 0; i < RP_ENUM_COUNT; ++i) {
		if (_rp[i].rp != p)
			continue;

		*count = _rp[i].state_count;
		return _rp[i].states;;
	}

	return NULL;
}

void
vkgfx_release_renderpass(void)
{
	for (uint32_t i = 0; i < RP_ENUM_COUNT; ++i) {
		if (_rp[i].rp == VK_NULL_HANDLE)
			continue;

		vkDestroyRenderPass(vkgfx_device, _rp[i].rp, vkgfx_allocator);
		_rp[i].rp = VK_NULL_HANDLE;
	}
}

