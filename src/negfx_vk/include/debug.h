/* NekoEngine
 *
 * debug.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Vulkan Graphics Subsystem Debug
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

#ifndef _NE_VK_GFX_DEBUG_
#define _NE_VK_GFX_DEBUG_

#include <vulkan/vulkan.h>

void vk_dbg_init(void);
void vk_dbg_set_obj_name(uint64_t object, VkDebugReportObjectTypeEXT type, const char *name);
void vk_dbg_set_obj_tag(uint64_t object, VkDebugReportObjectTypeEXT type, uint64_t name, uint64_t size, const void *tag);
void vk_dbg_begin_region(VkCommandBuffer cmd_buff, const char *name, float r, float g, float b, float a);
void vk_dbg_insert_marker(VkCommandBuffer cmd_buff, const char *name, float r, float g, float b, float a);
void vk_dbg_end_region(VkCommandBuffer cmd_buff);

VkBool32
vk_debug_callback(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT obj_type,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char *layer_prefix,
	const char *msg,
	void *user);

extern VkDebugReportCallbackEXT vk_debug_callback_handle;

#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
	#define VK_DBG_SET_OBJECT_NAME(object, type, name) vk_dbg_set_obj_name(object, type, name)
	#define VK_DBG_SET_OBJECT_TAG(object, type, name, tagSize, tag) vk_dbg_set_obj_tag(object, type, name, size, tag)
	#define VK_DBG_MARKER_BEGIN(cmdBuffer, name, r, g, b, a) vk_dbg_begin_region(cmd_buff, name, r, g, b, a)
	#define VK_DBG_MARKER_INSERT(cmdBuffer, name, r, g, b, a) vk_dbg_insert_marker(cmd_buff, name, r, g, b, a)
	#define VK_DBG_MARKER_END(cmdBuffer) vk_dbg_end_region(cmd_buff)
#else
	#define VK_DBG_SET_OBJECT_NAME(object, type, name)
	#define VK_DBG_SET_OBJECT_TAG(object, type, name, size, tag)
	#define VK_DBG_MARKER_BEGIN(cmd_buff, name, r, g, b, a)
	#define VK_DBG_MARKER_INSERT(cmd_buff, name, r, g, b, a)
	#define VK_DBG_MARKER_END(cmd_buff)
#endif

#endif /* _NE_VK_GFX_DEBUG_ */

