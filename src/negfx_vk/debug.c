/* NekoEngine
 *
 * debug.c
 * Author: Alexandru Naiman
 *
 * Vulkan DebugMarker extension wrapper
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

#include <string.h>

#include <system/log.h>

#include <debug.h>
#include <vkgfx.h>

#define DBGMKR_MODULE	"DebugMarker"
#define GFXDBG_MODULE	"GraphicsDebug"

VkDebugReportCallbackEXT vk_debug_callback_handle = VK_NULL_HANDLE;

static PFN_vkDebugMarkerSetObjectNameEXT _DebugMakerSetObjectNameEXT = NULL;
static PFN_vkDebugMarkerSetObjectTagEXT _DebugMarkerSetObjectTagEXT = NULL;
static PFN_vkCmdDebugMarkerBeginEXT _DebugMarkerBeginEXT = NULL;
static PFN_vkCmdDebugMarkerInsertEXT _DebugMarkerInsertEXT = NULL;
static PFN_vkCmdDebugMarkerEndEXT _DebugMarkerEndEXT = NULL;

void
vk_dbg_init(void)
{
	log_entry(DBGMKR_MODULE, LOG_INFORMATION, "Initializing...");

	_DebugMakerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)
		vkGetDeviceProcAddr(vkgfx_device, "vkDebugMarkerSetObjectNameEXT");
	_DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)
		vkGetDeviceProcAddr(vkgfx_device, "vkDebugMarkerSetObjectTagEXT");
	_DebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)
		vkGetDeviceProcAddr(vkgfx_device, "vkCmdDebugMarkerBeginEXT");
	_DebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)
		vkGetDeviceProcAddr(vkgfx_device, "vkCmdDebugMarkerInsertEXT");
	_DebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)
		vkGetDeviceProcAddr(vkgfx_device, "vkCmdDebugMarkerEndEXT");

	if(_DebugMakerSetObjectNameEXT)
		log_entry(DBGMKR_MODULE, LOG_INFORMATION, "Initialized");
	else
		log_entry(DBGMKR_MODULE, LOG_INFORMATION, "VK_EXT_debug_marker not present");
}

void
vk_dbg_set_obj_name(
	uint64_t object,
	VkDebugReportObjectTypeEXT type,
	const char *name)
{
	if (!_DebugMakerSetObjectNameEXT)
		return;

	VkDebugMarkerObjectNameInfoEXT info;
	memset(&info, 0x0, sizeof(info));
	info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
	info.objectType = type;
	info.object = object;
	info.pObjectName = name;

	_DebugMakerSetObjectNameEXT(vkgfx_device, &info);
}

void
vk_dbg_set_obj_tag(
	uint64_t object,
	VkDebugReportObjectTypeEXT type,
	uint64_t name,
	uint64_t size,
	const void *tag)
{
	if (!_DebugMarkerSetObjectTagEXT)
		return;

	VkDebugMarkerObjectTagInfoEXT info;
	memset(&info, 0x0, sizeof(info));
	info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
	info.objectType = type;
	info.object = object;
	info.tagName = name;
	info.tagSize = size;
	info.pTag = tag;

	_DebugMarkerSetObjectTagEXT(vkgfx_device, &info);
}

void
vk_dbg_begin_region(
	VkCommandBuffer cmd_buff,
	const char *name,
	float r,
	float g,
	float b,
	float a)
{
	if (!_DebugMarkerBeginEXT)
		return;

	VkDebugMarkerMarkerInfoEXT info;
	memset(&info, 0x0, sizeof(info));
	info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
	info.pMarkerName = name;
	info.color[0] = r;
	info.color[1] = g;
	info.color[2] = b;
	info.color[3] = a;

	_DebugMarkerBeginEXT(cmd_buff, &info);
}

void
vk_dbg_insert_marker(
	VkCommandBuffer cmd_buff,
	const char *name,
	float r,
	float g,
	float b,
	float a)
{
	if (!_DebugMarkerInsertEXT)
		return;

	VkDebugMarkerMarkerInfoEXT info;
	memset(&info, 0x0, sizeof(info));
	info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
	info.pMarkerName = name;
	info.color[0] = r;
	info.color[1] = g;
	info.color[2] = b;
	info.color[3] = a;

	_DebugMarkerInsertEXT(cmd_buff, &info);
}

void
vk_dbg_end_region(VkCommandBuffer cmd_buff)
{
	if (!_DebugMarkerEndEXT)
		return;

	_DebugMarkerEndEXT(cmd_buff);
}

VkBool32
vk_debug_callback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT obj_type,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char *layer_prefix,
	const char *msg,
	void *user)
{
	log_entry(GFXDBG_MODULE, LOG_DEBUG, "Vaidation [%s]: %s",
		layer_prefix, msg);
	return VK_FALSE;
}

