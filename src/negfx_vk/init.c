/* NekoEngine
 *
 * init.c
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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <system/log.h>
#include <system/mutex.h>
#include <system/config.h>
#include <system/platform.h>
#include <engine/resource.h>
#include <engine/task.h>

#include <vkgfx.h>
#include <debug.h>
#include <vkutil.h>
#include <shader.h>
#include <texture.h>
#include <swapchain.h>
#include <renderpass.h>

#define MAX_EXT			20
#define VKGFX_MODULE		"Vulkan_Graphics"

extern VkCommandPool vkgfx_graphics_pool;
extern VkCommandPool vkgfx_compute_pool;
extern VkCommandPool vkgfx_transfer_pool;

const char *vk_valid_layers[2] =
{
	"VK_LAYER_LUNARG_standard_validation",
	NULL
};

const char *vk_dev_ext[2] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	NULL
};

static bool _have_validation = false;
struct vkgfx_device_info vkgfx_device_info;
extern rt_array _cmd_pools;
extern bool _cb_submitted[VKGFX_MAX_SWAPCHAIN_IMAGES];
extern sys_mutex *_update_cb_mutex;
extern rt_array _update_cmd_buffers;
extern VkFence vkgfx_cb_fences[VKGFX_MAX_SWAPCHAIN_IMAGES];

static inline bool
_setup_debug_callback(void)
{
	VkDebugReportCallbackCreateInfoEXT ci;
	memset(&ci, 0x0, sizeof(ci));

	ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	ci.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	ci.pfnCallback = (PFN_vkDebugReportCallbackEXT)vk_debug_callback;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		(PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(vkgfx_instance,
			"vkCreateDebugReportCallbackEXT");

	if (vkCreateDebugReportCallbackEXT(vkgfx_instance, &ci, NULL,
		&vk_debug_callback_handle) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create debug callback");
		return false;
	}

	return true;
}

static inline bool
_check_validation_layer_support(void)
{
	bool ret = false;
	const char **ptr = NULL;
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, NULL);

	VkLayerProperties *layers = calloc(count, sizeof(VkLayerProperties));
	assert(layers);
	vkEnumerateInstanceLayerProperties(&count, layers);

	ptr = vk_valid_layers;
	while (*ptr) {
		bool found = false;
		size_t len = strlen(*ptr);

		for (uint32_t i = 0; i < count; ++i) {
			if (!strncmp(*ptr, layers[i].layerName, len)) {
				found = true;
				break;
			}
		}

		++ptr;

		if (!found)
			goto exit;
	}

	ret = true;

exit:
	free(layers);
	return ret;
}

static inline bool
_check_ext(
	const char *name,
	VkExtensionProperties *ext,
	uint32_t count)
{
	size_t len = strlen(name);

	for (uint32_t i = 0; i < count; ++i)
		if (!strncmp(ext[i].extensionName, name, len))
			return true;

	return false;
}

static inline bool
_check_device_extension(
	VkPhysicalDevice dev,
	const char *name)
{
	bool ret = false;
	uint32_t count = 0;

	vkEnumerateDeviceExtensionProperties(dev, NULL, &count, NULL);

	VkExtensionProperties *extensions =
		calloc(count, sizeof(VkExtensionProperties));
	assert(extensions);
	vkEnumerateDeviceExtensionProperties(vkgfx_device_info.phys_dev,
		NULL, &count, extensions);

	ret = _check_ext(name, extensions, count);

	free(extensions);

	return ret;
}

static inline bool
_check_instance_extension(const char *name)
{
	bool ret = false;
	uint32_t count = 0;

	vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);

	VkExtensionProperties *extensions =
	calloc(count, sizeof(VkExtensionProperties));
	assert(extensions);
	vkEnumerateInstanceExtensionProperties(NULL, &count, extensions);

	ret = _check_ext(name, extensions, count);

	free(extensions);

	return ret;
}

bool
_validate_device(
	VkPhysicalDevice device,
	struct vkgfx_device_info *out)
{
	uint32_t max_mem = 0, family_count = 0, ext_count = 0, fmt_count = 0;
	uint32_t present_mode_count = 0;
	VkQueueFamilyProperties *family_props = NULL;
	VkExtensionProperties *ext_props = NULL;

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);

	// we're only interested in GPU devices
	if (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
		props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		props.deviceType != VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		return false;

	VkPhysicalDeviceFeatures feat;
	vkGetPhysicalDeviceFeatures(device, &feat);

	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, NULL);
	family_props = calloc(family_count, sizeof(*family_props));
	assert(family_props);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count,
		family_props);

	for (uint32_t i = 0; i < family_count; ++i) {
		VkBool32 present_supported = false;
		VkQueueFamilyProperties fam_props = family_props[i];

		if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i,
			vkgfx_surface, &present_supported) != VK_SUCCESS)
			continue;

		if (fam_props.queueCount == 0)
			continue;

		if (out->compute_fam == -1 &&
			fam_props.queueFlags & VK_QUEUE_COMPUTE_BIT)
			out->compute_fam = i;

		if (out->gfx_fam == -1 &&
			fam_props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			out->gfx_fam = i;

		if (out->present_fam == -1 && present_supported)
			out->present_fam = i;

		if (out->gfx_fam > 0 && out->present_fam > 0 &&
			out->compute_fam > 0)
			break;
	}

	free(family_props);

	if (out->gfx_fam < 0 || out->present_fam < 0 || out->compute_fam < 0)
		return false;

	if (vkEnumerateDeviceExtensionProperties(device, NULL,
		&ext_count, NULL) != VK_SUCCESS)
		return false;

	ext_props = calloc(ext_count, sizeof(*ext_props));
	assert(ext_props);
	if (vkEnumerateDeviceExtensionProperties(device, NULL,
		&ext_count, ext_props) != VK_SUCCESS) {
		free(ext_props);
		return false;
	}

	const char **ptr = vk_dev_ext;
	while (*ptr) {
		if (!_check_ext(*ptr++, ext_props, ext_count)) {
			free(ext_props);
			return false;
		}
	}

	free(ext_props);

	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkgfx_surface,
		&out->caps) != VK_SUCCESS)
		return false;

	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkgfx_surface,
		&fmt_count, NULL) != VK_SUCCESS)
		return false;

	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkgfx_surface,
		&present_mode_count, NULL) != VK_SUCCESS)
		return false;

	if (!fmt_count || !present_mode_count)
		return false;

	VkSurfaceFormatKHR * formats =
		calloc(fmt_count, sizeof(*formats));
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkgfx_surface,
		&fmt_count, formats) != VK_SUCCESS) {
		free(formats);
		return false;
	}

	// check format

	free(formats);

	VkPresentModeKHR *present_modes =
		calloc(present_mode_count, sizeof(*present_modes));
	assert(present_modes);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkgfx_surface,
		&present_mode_count, present_modes) != VK_SUCCESS) {
		free(present_modes);
		return false;
	}

	out->pm = VK_PRESENT_MODE_FIFO_KHR;
	VkPresentModeKHR desired_pm =
		sys_config_get_bool("vertical_sync", false) ?
		VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

	for (uint32_t i = 0; i < present_mode_count; ++i) {
		if (present_modes[i] != desired_pm)
			continue;

		out->pm = desired_pm;
		break;
	}

	free(present_modes);

	out->phys_dev = device;
	out->phys_dev_type = props.deviceType;
	out->limits = props.limits;

	snprintf(out->name, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, "%s",
		props.deviceName);
	snprintf(out->version, VK_VERSION_SIZE, "%d.%d.%d",
		(props.apiVersion >> 22), ((props.apiVersion >> 12) & 0x3FF),
		(props.apiVersion & 0xFFF));

	return true;
}

// Early init

ne_status
vkgfx_init_types(void)
{
	if (res_register_type(RES_SHADER, sh_load,
		sh_destroy) != NE_OK) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to register shader resource handler");
		return NE_FAIL;
	}

	if (res_register_type(RES_SHADER_MODULE, shmod_load,
		shmod_destroy) != NE_OK) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to register shader module resource handler");
		return NE_FAIL;
	}

	return NE_OK;
}

ne_status
vkgfx_init_render_target_info(void)
{
	memset(&vkgfx_render_target, 0x0, sizeof(vkgfx_render_target));
	vkgfx_render_target.color.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vkgfx_render_target.normal.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vkgfx_render_target.depth.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	vkgfx_render_target.resolve.format = VK_FORMAT_R32G32B32A32_SFLOAT;

	if (sys_config_get_bool("multisampling", true)) {
		vkgfx_render_target.samples = sys_config_get_int("samples", 8);

		if (!vkgfx_render_target.samples)
			vkgfx_render_target.samples = 8;
	} else {
		vkgfx_render_target.samples = VK_SAMPLE_COUNT_1_BIT;
	}

	vkgfx_render_target.width = ne_gfx_screen_width;
	vkgfx_render_target.height = ne_gfx_screen_height;

	return NE_OK;
}

// Instance

ne_status
vkgfx_init_instance(void)
{
#ifdef _DEBUG
	bool debug = true;
	bool validation = true;
#else
	bool debug = false;
	bool validation = false;
#endif
	VkResult ret;
	VkApplicationInfo app;
	VkInstanceCreateInfo inst;
	const char **ptr = NULL;
	const char *extensions[MAX_EXT];
	uint8_t ext_count = 0, valid_layer_count = 0;

	debug = sys_config_get_bool("vkgfx_debug", debug);
	validation = sys_config_get_bool("vkgfx_validation", validation);

	_have_validation = validation && _check_validation_layer_support();

	if (debug)
		log_entry(VKGFX_MODULE, LOG_DEBUG, "Debugging enabled");

	if (_have_validation)
		log_entry(VKGFX_MODULE, LOG_DEBUG, "Validation enabled");
	else if (validation)
		log_entry(VKGFX_MODULE, LOG_DEBUG,
			"Validation requested, but no layers present");

	memset(&app, 0x0, sizeof(app));
	memset(&inst, 0x0, sizeof(inst));

	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "NekoEngine";
	app.applicationVersion = VK_MAKE_VERSION(0, 6, 0);
	app.pEngineName = "NekoEngine";
	app.engineVersion = VK_MAKE_VERSION(0, 6, 0);

#ifdef SYS_PLATFORM_APPLE
	app.apiVersion = VK_API_VERSION_1_0;
#else
	app.apiVersion = VK_API_VERSION_1_1;
#endif

	inst.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst.pApplicationInfo = &app;

	ptr = vk_gfx_platform_ext;
	while (*ptr && ext_count < MAX_EXT - 1)
		extensions[ext_count++] = *ptr++;

	if (debug && _check_instance_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
		extensions[ext_count++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;

	inst.enabledExtensionCount = ext_count;
	inst.ppEnabledExtensionNames = extensions;

	if (_have_validation) {
		ptr = vk_valid_layers;
		while (*ptr++)
			++valid_layer_count;

		inst.enabledLayerCount = valid_layer_count;
		inst.ppEnabledLayerNames = vk_valid_layers;
	}

	ret = vkCreateInstance(&inst, vkgfx_allocator, &vkgfx_instance);
	if (ret != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create instance %d", ret);
		return NE_FAIL;
	}

	return NE_OK;
}

void
vkgfx_release_instance()
{
	vkDestroyInstance(vkgfx_instance, vkgfx_allocator);
	vkgfx_instance = VK_NULL_HANDLE;
}

// Device

ne_status
vkgfx_init_device(void)
{
#ifdef _DEBUG
	bool debug = true;
#else
	bool debug = false;
#endif
	int64_t dev_id = 0;
	uint32_t dev_count = 0, ext_count = 0, valid_layer_count = 0;
	const char **ptr = NULL;
	const char *extensions[MAX_EXT];

	debug = sys_config_get_bool("vkgfx_debug", debug);

	memset(&vkgfx_device_info, 0x0, sizeof(vkgfx_device_info));

	if (vkEnumeratePhysicalDevices(vkgfx_instance, &dev_count, NULL)
		!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to enumerate devices");
		return NE_FAIL;
	}

	if (!dev_count) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL, "No devices found");
		return NE_FAIL;
	}

	VkPhysicalDevice *devices = calloc(dev_count, sizeof(*devices));
	assert(devices);

	if (vkEnumeratePhysicalDevices(vkgfx_instance, &dev_count, devices)
		!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to enumerate devices");
		free(devices);
		return NE_FAIL;
	}

	dev_id = sys_config_get_int("vkgfx_device_id", -1);

	if (dev_id != -1) {
		vkgfx_device_info.id = (uint32_t)dev_id;
		if (!_validate_device(devices[dev_id], &vkgfx_device_info)) {
			log_entry(VKGFX_MODULE, LOG_CRITICAL,
				"The specified device cannot be used, aborting");
			free(devices);
			return NE_GFX_DEV_CREATE_FAIL;
		}
	} else {
		uint32_t next_dev = 0, best_dev = 0;
		struct vkgfx_device_info *dev_info = NULL;

		dev_info = calloc(dev_count, sizeof(*dev_info));
		assert(dev_info);

		for (uint32_t i = 0; i < dev_count; ++i) {
			dev_info[next_dev].id = i;
			if (!_validate_device(devices[i], &dev_info[next_dev]))
				continue;
			++next_dev;
		}

		if (!next_dev) {
			log_entry(VKGFX_MODULE, LOG_CRITICAL,
				"No usable devices found");
			free(dev_info);
			free(devices);
			return NE_GFX_DEV_CREATE_FAIL;
		}

		if (next_dev > 1) {
			for (uint32_t i = 0; i < next_dev; ++i) {
				// compare devices
			}
		}

		memcpy(&vkgfx_device_info, &dev_info[best_dev],
			sizeof(vkgfx_device_info));
		free(dev_info);
	}

	free(devices);

	float queue_pri = 1.f;
	int32_t queue_fam[3] = { vkgfx_device_info.gfx_fam,
		vkgfx_device_info.present_fam,
		vkgfx_device_info.compute_fam };
	uint32_t queue_count = 0;
	VkDeviceQueueCreateInfo qci[3];
	memset(qci, 0x0, sizeof(qci));

	for (uint8_t i = 0; i < 3; ++i) {
		bool add = true;

		for (uint8_t j = 0; j < queue_count; ++j) {
			if (qci[j].queueFamilyIndex == queue_fam[i]) {
				add = false;
				break;
			}
		}

		if (!add)
			continue;

		qci[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qci[queue_count].queueFamilyIndex = queue_fam[i];
		qci[queue_count].queueCount = 1;
		qci[queue_count].pQueuePriorities = &queue_pri;

		++queue_count;
	}

	VkPhysicalDeviceFeatures dev_features;
	memset(&dev_features, 0x0, sizeof(dev_features));
	dev_features.independentBlend = VK_TRUE;
	dev_features.depthBounds = VK_TRUE;
	dev_features.fillModeNonSolid = VK_TRUE;
	dev_features.geometryShader = VK_TRUE;
	dev_features.samplerAnisotropy = VK_TRUE;
	dev_features.textureCompressionBC = VK_TRUE;
	dev_features.fullDrawIndexUint32 = VK_TRUE;
	dev_features.alphaToOne = VK_TRUE;

	VkDeviceCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.pQueueCreateInfos = qci;
	ci.queueCreateInfoCount = queue_count;
	ci.pEnabledFeatures = &dev_features;

	ptr = vk_dev_ext;
	while (*ptr && ext_count < MAX_EXT - 1)
		extensions[ext_count++] = *ptr++;

	if (debug && vkgfx_device_info.have_debug_ext)
		extensions[ext_count++] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;

	ci.enabledExtensionCount = ext_count;
	ci.ppEnabledExtensionNames = extensions;

	if (_have_validation) {
		ptr = vk_valid_layers;
		while (*ptr++)
			++valid_layer_count;

		ci.enabledLayerCount = valid_layer_count;
		ci.ppEnabledLayerNames = vk_valid_layers;
	}

	if (vkCreateDevice(vkgfx_device_info.phys_dev, &ci, vkgfx_allocator,
		&vkgfx_device) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create device");
		return NE_GFX_DEV_CREATE_FAIL;
	}

	vkGetDeviceQueue(vkgfx_device, vkgfx_device_info.gfx_fam, 0,
		&vkgfx_graphics_queue);
	vkGetDeviceQueue(vkgfx_device, vkgfx_device_info.present_fam, 0,
		&vkgfx_present_queue);
	vkGetDeviceQueue(vkgfx_device, vkgfx_device_info.compute_fam, 0,
		&vkgfx_compute_queue);
	vkGetDeviceQueue(vkgfx_device, vkgfx_device_info.transfer_fam, 0,
		&vkgfx_transfer_queue);

	log_entry(VKGFX_MODULE, LOG_INFORMATION, "Vulkan: %s", vkgfx_device_info.version);
	log_entry(VKGFX_MODULE, LOG_INFORMATION, "Device: %s", vkgfx_device_info.name);

	return NE_OK;
}

void
vkgfx_release_device(void)
{
	if (vkgfx_device != VK_NULL_HANDLE)
		vkDestroyDevice(vkgfx_device, vkgfx_allocator);
	vkgfx_device = VK_NULL_HANDLE;
}

// Swapchain

ne_status
vkgfx_init_swapchain(void)
{
	return sw_init(vkgfx_device_info.caps, VK_FORMAT_B8G8R8A8_UNORM,
		VK_COLORSPACE_SRGB_NONLINEAR_KHR, vkgfx_device_info.pm,
		vkgfx_device_info.gfx_fam, vkgfx_device_info.present_fam);
}

void
vkgfx_release_swapchain(void)
{
	sw_destroy();
}

// Debugging

ne_status
vkgfx_init_debug(void)
{
	vk_dbg_init();

	if (_have_validation && !_setup_debug_callback())
		return NE_FAIL;

	return NE_OK;
}

void
vkgfx_release_debug(void)
{
	if (vk_debug_callback_handle != VK_NULL_HANDLE) {
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		(PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
			vkgfx_instance, "vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT(vkgfx_instance,
			vk_debug_callback_handle, vkgfx_allocator);
	}
	vk_debug_callback_handle = VK_NULL_HANDLE;
}

// Command Pools

ne_status
vkgfx_init_cmd_pools(void)
{
	VkResult res;
	VkCommandPoolCreateInfo gfx_ci, xfer_ci, compute_ci;

	memset(&_cmd_pools, 0x0, sizeof(_cmd_pools));

	memset(&gfx_ci, 0x0, sizeof(gfx_ci));
	gfx_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	gfx_ci.queueFamilyIndex = vkgfx_device_info.gfx_fam;
	gfx_ci.flags = 0;

	memset(&xfer_ci, 0x0, sizeof(xfer_ci));
	xfer_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	xfer_ci.queueFamilyIndex = vkgfx_device_info.transfer_fam;
	xfer_ci.flags = 0;

	memset(&compute_ci, 0x0, sizeof(compute_ci));
	compute_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	compute_ci.queueFamilyIndex = vkgfx_device_info.compute_fam;
	compute_ci.flags = 0;

	if ((res = vkCreateCommandPool(vkgfx_device, &gfx_ci,
		vkgfx_allocator, &vkgfx_graphics_pool)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create graphics command pool: %d", res);
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)vkgfx_graphics_pool,
		VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
		"Graphics Command Pool");

	if ((res = vkCreateCommandPool(vkgfx_device, &compute_ci,
		vkgfx_allocator, &vkgfx_compute_pool)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create compute command pool: %d", res);
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)vkgfx_compute_pool,
		VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
		"Compute Command Pool");

	if ((res = vkCreateCommandPool(vkgfx_device, &xfer_ci,
		vkgfx_allocator, &vkgfx_transfer_pool)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create transfer command pool: %d", res);
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)vkgfx_transfer_queue,
		VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
		"Transfer Command Pool");

	if (rt_array_init(&_cmd_pools, task_num_workers() + 1,
			sizeof(struct vkgfx_thread_cp)) != SYS_OK)
		goto error;

	rt_array_fill(&_cmd_pools);

	for (uint32_t i = 0; i < _cmd_pools.count; ++i) {
		struct vkgfx_thread_cp *cp = rt_array_get(&_cmd_pools, i);

		if ((res = vkCreateCommandPool(vkgfx_device, &gfx_ci,
			vkgfx_allocator, &cp->static_gfx_pool))
				!= VK_SUCCESS)
			goto error;
		VK_DBG_SET_OBJECT_NAME((uint64_t)cp->_static_gfx_pool,
					VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
					"Graphics Command Pool");

		if ((res = vkCreateCommandPool(vkgfx_device, &xfer_ci,
			vkgfx_allocator, &cp->static_xfer_pool))
				!= VK_SUCCESS)
			goto error;
		VK_DBG_SET_OBJECT_NAME((uint64_t)_cmd_pools[i]._static_xfer_pool,
					VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
					"Transfer Command Pool");

		if ((res = vkCreateCommandPool(vkgfx_device, &compute_ci,
			vkgfx_allocator, &cp->static_compute_pool))
				!= VK_SUCCESS)
			goto error;
		VK_DBG_SET_OBJECT_NAME((uint64_t)_cmd_pools[i]._static_compute_pool,
					VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
					"Compute Command Pool");

		gfx_ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		xfer_ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		compute_ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		for (uint8_t j = 0; j < VKGFX_MAX_SWAPCHAIN_IMAGES; ++j) {
			if (rt_array_init_ptr(&cp->gfx_free_list[j],
					50) != SYS_OK)
				goto error;

			if (rt_array_init_ptr(&cp->xfer_free_list[j],
					50) != SYS_OK)
				goto error;

			if (rt_array_init_ptr(&cp->compute_free_list[j],
					50) != SYS_OK)
				goto error;

			if ((res = vkCreateCommandPool(vkgfx_device, &gfx_ci,
				vkgfx_allocator, &cp->dynamic_gfx_pools[j]))
					!= VK_SUCCESS)
				goto error;
			VK_DBG_SET_OBJECT_NAME((uint64_t)_cmd_pools[i]._dynamic_gfx_pools[j],
						VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
						"Graphics Command Pool");

			if ((res = vkCreateCommandPool(vkgfx_device, &xfer_ci,
				vkgfx_allocator, &cp->dynamic_xfer_pools[j]))
					!= VK_SUCCESS)
			VK_DBG_SET_OBJECT_NAME((uint64_t)_cmd_pools[i]._dynamic_xfer_pools[j],
						VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
						"Transfer Command Pool");

			if ((res = vkCreateCommandPool(vkgfx_device, &compute_ci,
				vkgfx_allocator, &cp->dynamic_compute_pools[j]))
					!= VK_SUCCESS)
				goto error;
			VK_DBG_SET_OBJECT_NAME((uint64_t)_cmd_pools[i]._dynamic_compute_pools[j],
						VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
						"Compute Command Pool");
		}
	}

	return NE_OK;

error:
	log_entry(VKGFX_MODULE, LOG_CRITICAL,
		"Failed to create command pool: %s", vku_result_string(res));
	return NE_FAIL;
}

void
vkgfx_release_cmd_pools(void)
{
	if (vkgfx_graphics_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(vkgfx_device, vkgfx_graphics_pool, vkgfx_allocator);
	vkgfx_graphics_pool = VK_NULL_HANDLE;

	if (vkgfx_compute_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(vkgfx_device, vkgfx_compute_pool, vkgfx_allocator);
	vkgfx_compute_pool = VK_NULL_HANDLE;

	if (vkgfx_transfer_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(vkgfx_device, vkgfx_transfer_pool, vkgfx_allocator);
	vkgfx_transfer_pool = VK_NULL_HANDLE;

	for (uint32_t i = 0; i < _cmd_pools.count; ++i) {
		struct vkgfx_thread_cp *cp = rt_array_get(&_cmd_pools, i);

		if (cp->static_gfx_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(vkgfx_device,
					cp->static_gfx_pool,
					vkgfx_allocator);
		cp->static_gfx_pool = VK_NULL_HANDLE;

		if (cp->static_xfer_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(vkgfx_device,
					cp->static_xfer_pool,
					vkgfx_allocator);
		cp->static_xfer_pool = VK_NULL_HANDLE;

		if (cp->static_compute_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(vkgfx_device,
					cp->static_compute_pool,
					vkgfx_allocator);
		cp->static_compute_pool = VK_NULL_HANDLE;

		for (uint8_t j = 0; j < VKGFX_MAX_SWAPCHAIN_IMAGES; ++j) {
			rt_array_release(&cp->gfx_free_list[j]);
			rt_array_release(&cp->xfer_free_list[j]);
			rt_array_release(&cp->compute_free_list[j]);

			if (cp->dynamic_gfx_pools[j] != VK_NULL_HANDLE)
				vkDestroyCommandPool(vkgfx_device,
						cp->dynamic_gfx_pools[j],
						vkgfx_allocator);
			cp->dynamic_gfx_pools[j] = VK_NULL_HANDLE;

			if (cp->dynamic_xfer_pools[j] != VK_NULL_HANDLE)
				vkDestroyCommandPool(vkgfx_device,
						cp->dynamic_xfer_pools[j],
						vkgfx_allocator);
			cp->dynamic_xfer_pools[j] = VK_NULL_HANDLE;

			if (cp->dynamic_compute_pools[j] != VK_NULL_HANDLE)
				vkDestroyCommandPool(vkgfx_device,
						cp->dynamic_compute_pools[j],
						vkgfx_allocator);
			cp->dynamic_compute_pools[j] = VK_NULL_HANDLE;
		}
	}

	rt_array_release(&_cmd_pools);
	memset(&_cmd_pools, 0x0, sizeof(_cmd_pools));
}

// Semaphores

ne_status
vkgfx_init_sem(void)
{
	VkResult res;
	VkSemaphoreCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));

	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if ((res = vkCreateSemaphore(vkgfx_device, &ci,
		vkgfx_allocator, &vkgfx_image_available_sem)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create image available semaphore");
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)vkgfx_image_available_sem,
		VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, "Image Available Semaphore");

	if ((res = vkCreateSemaphore(vkgfx_device, &ci,
		vkgfx_allocator, &vkgfx_transfer_complete_sem)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
				  "Failed to create transfer complete semaphore");
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)vkgfx_transfer_complete_sem,
		VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, "Transfer Complete Semaphore");

	if ((res = vkCreateSemaphore(vkgfx_device, &ci,
		vkgfx_allocator, &vkgfx_scene_complete_sem)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
				  "Failed to create scene complete semaphore");
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)vk_sceme_complete_sem,
		VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, "Scene Complete Semaphore");

/*	if (VkResult res = vkCreateSemaphore(NeGraphicsDevice, &ci,
		NeAllocationCallbacks, &NePostProcessFinishedSemaphore);
		res != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL, "Failed to create post process finisshed semaphore");
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)NePostProcessFinishedSemaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, "Post Process Finished Semaphore");*/

	if ((res = vkCreateSemaphore(vkgfx_device, &ci,
		vkgfx_allocator, &vkgfx_render_complete_sem)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create render complete semaphore");
		return NE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)vkgfx_render_complete_sem,
		VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, "Render Complete Semaphore");

	return NE_OK;
}

void
vkgfx_release_sem(void)
{
	if (vkgfx_image_available_sem != VK_NULL_HANDLE)
		vkDestroySemaphore(vkgfx_device, vkgfx_image_available_sem, vkgfx_allocator);
	vkgfx_image_available_sem = VK_NULL_HANDLE;

	if (vkgfx_transfer_complete_sem != VK_NULL_HANDLE)
		vkDestroySemaphore(vkgfx_device, vkgfx_transfer_complete_sem, vkgfx_allocator);
	vkgfx_transfer_complete_sem = VK_NULL_HANDLE;

	if (vkgfx_scene_complete_sem != VK_NULL_HANDLE)
		vkDestroySemaphore(vkgfx_device, vkgfx_scene_complete_sem, vkgfx_allocator);
	vkgfx_scene_complete_sem = VK_NULL_HANDLE;

	if (vkgfx_render_complete_sem != VK_NULL_HANDLE)
		vkDestroySemaphore(vkgfx_device, vkgfx_render_complete_sem, vkgfx_allocator);
	vkgfx_render_complete_sem = VK_NULL_HANDLE;
}

// Framebuffers

ne_status
vkgfx_init_framebuffers(void)
{
	VkResult res;

	// Images
	VkImageCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.imageType = VK_IMAGE_TYPE_2D;
	ci.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	ci.mipLevels = 1;
	ci.arrayLayers = 1;
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.tiling = VK_IMAGE_TILING_OPTIMAL;
	ci.extent.depth = 1;

	ci.extent.width = vkgfx_render_target.width;
	ci.extent.height = vkgfx_render_target.height;

	ci.format = vkgfx_render_target.color.format;
	ci.samples = vkgfx_render_target.samples;
	ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	res = vkCreateImage(vkgfx_device, &ci, vkgfx_allocator,
		&vkgfx_render_target.color.image);
	if (res != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create color image: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	ci.format = vkgfx_render_target.normal.format;
	ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT;
	res = vkCreateImage(vkgfx_device, &ci, vkgfx_allocator,
		&vkgfx_render_target.normal.image);
	if (res != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create normal image: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	if (vkgfx_render_target.samples != VK_SAMPLE_COUNT_1_BIT) {
		ci.samples = VK_SAMPLE_COUNT_1_BIT;;
		ci.format = vkgfx_render_target.resolve.format;
		ci.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		res = vkCreateImage(vkgfx_device, &ci, vkgfx_allocator,
			&vkgfx_render_target.resolve.image);
		if (res != VK_SUCCESS) {
			log_entry(VKGFX_MODULE, LOG_CRITICAL,
				"Failed to create resolve image: %s",
				vku_result_string(res));
			return NE_FAIL;
		}
	}

	ci.samples = vkgfx_render_target.samples;
	ci.format = vkgfx_render_target.depth.format;
	ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	res = vkCreateImage(vkgfx_device, &ci, vkgfx_allocator,
			&vkgfx_render_target.depth.image);
	if (res != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create depth image: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	// Allocate & bind memory
	VkMemoryRequirements color_req, normal_req,
			     resolve_req, depth_req;
	vkGetImageMemoryRequirements(vkgfx_device,
			vkgfx_render_target.color.image, &color_req);
	vkGetImageMemoryRequirements(vkgfx_device,
			vkgfx_render_target.normal.image, &normal_req);
	vkGetImageMemoryRequirements(vkgfx_device,
			vkgfx_render_target.depth.image, &depth_req);

	VkDeviceSize color_size = ((color_req.size / 0x400) + 1) * 0x400;
	VkDeviceSize normal_size = ((normal_req.size / 0x400) + 1) * 0x400;
	VkDeviceSize depth_size = ((depth_req.size / 0x400) + 1) * 0x400;

	VkMemoryAllocateInfo ai;
	memset(&ai, 0x0, sizeof(ai));
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	if (vkgfx_render_target.samples != VK_SAMPLE_COUNT_1_BIT) {
		vkGetImageMemoryRequirements(vkgfx_device,
			vkgfx_render_target.resolve.image, &resolve_req);
		VkDeviceSize resolve_size =
			((resolve_req.size / 0x400) + 1) * 0x400;
		ai.allocationSize = color_size + normal_size +
			depth_size + resolve_size;
		ai.memoryTypeIndex =
			vku_get_mem_type(color_req.memoryTypeBits |
				normal_req.memoryTypeBits |
				resolve_req.memoryTypeBits |
				depth_req.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	} else {
		ai.allocationSize = color_size + normal_size +
			depth_size;
		ai.memoryTypeIndex =
			vku_get_mem_type(color_req.memoryTypeBits |
				normal_req.memoryTypeBits |
				depth_req.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	log_entry(VKGFX_MODULE, LOG_DEBUG,
		"Render target memory: %lld MB",
		ai.allocationSize / 1024 / 1024);

	if ((res = vkAllocateMemory(vkgfx_device, &ai,
		vkgfx_allocator, &vkgfx_render_target.memory)) != VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to allocate combined fb memory: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	if ((res = vkBindImageMemory(vkgfx_device,
			vkgfx_render_target.color.image,
			vkgfx_render_target.memory, 0))
		!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to bind color image memory: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	if ((res = vkBindImageMemory(vkgfx_device,
			vkgfx_render_target.depth.image,
			vkgfx_render_target.memory, color_size))
		!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to bind depth image memory: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	if ((res = vkBindImageMemory(vkgfx_device,
			vkgfx_render_target.normal.image,
			vkgfx_render_target.memory, color_size + depth_size))
		!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to bind normal image memory: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	if (vkgfx_render_target.samples != VK_SAMPLE_COUNT_1_BIT) {
		if ((res = vkBindImageMemory(vkgfx_device,
				vkgfx_render_target.resolve.image,
				vkgfx_render_target.memory,
				color_size + depth_size +
				normal_size))
			!= VK_SUCCESS) {
			log_entry(VKGFX_MODULE, LOG_CRITICAL,
				"Failed to bind resolve image memory: %s",
				vku_result_string(res));
			return NE_FAIL;
		}
	}

	// Image Views

	VkImageViewCreateInfo ivci;
	memset(&ivci, 0x0, sizeof(ivci));
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.components.r = VK_COMPONENT_SWIZZLE_R;
	ivci.components.g = VK_COMPONENT_SWIZZLE_G;
	ivci.components.b = VK_COMPONENT_SWIZZLE_B;
	ivci.components.a = VK_COMPONENT_SWIZZLE_A;
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.flags = 0;

	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	ivci.image = vkgfx_render_target.color.image;
	ivci.format = vkgfx_render_target.color.format;
	if ((res = vkCreateImageView(vkgfx_device, &ivci, vkgfx_allocator,
			&vkgfx_render_target.color.image_view))
			!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create color image view: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	ivci.image = vkgfx_render_target.normal.image;
	ivci.format = vkgfx_render_target.normal.format;
	if ((res = vkCreateImageView(vkgfx_device, &ivci, vkgfx_allocator,
			&vkgfx_render_target.normal.image_view))
			!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create normal image view: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	if (vkgfx_render_target.samples != VK_SAMPLE_COUNT_1_BIT) {
		ivci.image = vkgfx_render_target.resolve.image;
		ivci.format = vkgfx_render_target.resolve.format;
		if ((res = vkCreateImageView(vkgfx_device, &ivci, vkgfx_allocator,
				&vkgfx_render_target.resolve.image_view))
				!= VK_SUCCESS) {
			log_entry(VKGFX_MODULE, LOG_CRITICAL,
				"Failed to create resolve image view: %s",
				vku_result_string(res));
			return NE_FAIL;
		}
	}

	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT |
		VK_IMAGE_ASPECT_STENCIL_BIT;
	ivci.image = vkgfx_render_target.depth.image;
	ivci.format = vkgfx_render_target.depth.format;
	if ((res = vkCreateImageView(vkgfx_device, &ivci, vkgfx_allocator,
			&vkgfx_render_target.depth.image_view))
			!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create depth image view: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	// Framebuffers

	VKU_STRUCT(VkFramebufferCreateInfo, fbci,
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
	fbci.width = vkgfx_render_target.width;
	fbci.height = vkgfx_render_target.height;
	fbci.layers = 1;

	VkImageView depth_attachments[2] =
	{
		vkgfx_render_target.normal.image_view,
		vkgfx_render_target.depth.image_view
	};
	fbci.attachmentCount = 2;
	fbci.pAttachments = depth_attachments;
	fbci.renderPass = rp_get(RP_DEPTH);
	if ((res = vkCreateFramebuffer(vkgfx_device, &fbci, vkgfx_allocator,
			&vkgfx_render_target.depth_fb))
			!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create depth framebuffer: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	VkImageView lighting_attachments[3] =
	{
		vkgfx_render_target.color.image_view,
		vkgfx_render_target.depth.image_view,
		vkgfx_render_target.resolve.image_view
	};
	fbci.attachmentCount = 3;
	fbci.pAttachments = lighting_attachments;
	fbci.renderPass = rp_get(RP_LIGHTING);
	if ((res = vkCreateFramebuffer(vkgfx_device, &fbci, vkgfx_allocator,
			&vkgfx_render_target.lighting_fb))
			!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
			"Failed to create lighting framebuffer: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	// Transition Images

	VkCommandBuffer cmd_buff =
		vku_create_one_shot_cmd_buffer(vkgfx_current_graphics_cmd_pool());

	vku_transition_image_layout(
		vkgfx_render_target.color.image,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,
		cmd_buff,
		false,
		vkgfx_current_graphics_cmd_pool(),
		vkgfx_graphics_queue);

	vku_transition_image_layout(
		vkgfx_render_target.normal.image,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,
		cmd_buff,
		false,
		vkgfx_current_graphics_cmd_pool(),
		vkgfx_graphics_queue);

	vku_transition_image_layout(
		vkgfx_render_target.resolve.image,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,
		cmd_buff,
		false,
		vkgfx_current_graphics_cmd_pool(),
		vkgfx_graphics_queue);

	vku_transition_image_layout(
		vkgfx_render_target.depth.image,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		cmd_buff,
		false,
		vkgfx_current_graphics_cmd_pool(),
		vkgfx_graphics_queue);

	vku_execute_one_shot_cmd_buffer(cmd_buff, vkgfx_current_graphics_cmd_pool(),
		vkgfx_graphics_queue, VK_NULL_HANDLE, 0);

	return NE_OK;
}

void
vkgfx_release_framebuffers(void)
{
	// Framebuffers
	if (vkgfx_render_target.depth_fb != VK_NULL_HANDLE)
		vkDestroyFramebuffer(vkgfx_device,
			vkgfx_render_target.depth_fb, vkgfx_allocator);
	vkgfx_render_target.depth_fb = VK_NULL_HANDLE;

	if (vkgfx_render_target.lighting_fb != VK_NULL_HANDLE)
		vkDestroyFramebuffer(vkgfx_device,
			vkgfx_render_target.lighting_fb, vkgfx_allocator);
	vkgfx_render_target.lighting_fb = VK_NULL_HANDLE;

	// Image Views
	if (vkgfx_render_target.color.image_view != VK_NULL_HANDLE)
		vkDestroyImageView(vkgfx_device,
			vkgfx_render_target.color.image_view, vkgfx_allocator);
	vkgfx_render_target.color.image_view = VK_NULL_HANDLE;

	if (vkgfx_render_target.depth.image_view != VK_NULL_HANDLE)
		vkDestroyImageView(vkgfx_device,
			vkgfx_render_target.depth.image_view, vkgfx_allocator);
	vkgfx_render_target.depth.image_view = VK_NULL_HANDLE;

	if (vkgfx_render_target.normal.image_view != VK_NULL_HANDLE)
		vkDestroyImageView(vkgfx_device,
			vkgfx_render_target.normal.image_view, vkgfx_allocator);
	vkgfx_render_target.normal.image_view = VK_NULL_HANDLE;

	if (vkgfx_render_target.resolve.image_view != VK_NULL_HANDLE)
		vkDestroyImageView(vkgfx_device,
			vkgfx_render_target.resolve.image_view, vkgfx_allocator);
	vkgfx_render_target.resolve.image_view = VK_NULL_HANDLE;

	// Images
	if (vkgfx_render_target.color.image != VK_NULL_HANDLE)
		vkDestroyImage(vkgfx_device,
			vkgfx_render_target.color.image, vkgfx_allocator);
	vkgfx_render_target.color.image = VK_NULL_HANDLE;

	if (vkgfx_render_target.depth.image != VK_NULL_HANDLE)
		vkDestroyImage(vkgfx_device,
			vkgfx_render_target.depth.image, vkgfx_allocator);
	vkgfx_render_target.depth.image = VK_NULL_HANDLE;

	if (vkgfx_render_target.normal.image != VK_NULL_HANDLE)
		vkDestroyImage(vkgfx_device,
			vkgfx_render_target.normal.image, vkgfx_allocator);
	vkgfx_render_target.normal.image = VK_NULL_HANDLE;

	if (vkgfx_render_target.resolve.image != VK_NULL_HANDLE)
		vkDestroyImage(vkgfx_device,
			vkgfx_render_target.resolve.image, vkgfx_allocator);
	vkgfx_render_target.resolve.image = VK_NULL_HANDLE;

	if (vkgfx_render_target.memory != VK_NULL_HANDLE)
		vkFreeMemory(vkgfx_device,
			vkgfx_render_target.memory, vkgfx_allocator);
	vkgfx_render_target.memory = VK_NULL_HANDLE;
}

// Fences

ne_status
vkgfx_init_synchronization(void)
{
	memset(vkgfx_cb_fences, 0x0, sizeof(vkgfx_cb_fences));
	memset(_cb_submitted, 0x0, sizeof(_cb_submitted));

	_update_cb_mutex = sys_mutex_create();
	if (!_update_cb_mutex) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL, "Failed to create update mutex");
		return NE_FAIL;
	}

	VkFenceCreateInfo fci;
	memset(&fci, 0x0, sizeof(fci));
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	for (uint32_t i = 0; i < VKGFX_MAX_SWAPCHAIN_IMAGES; ++i) {
		assert(vkCreateFence(vkgfx_device, &fci, vkgfx_allocator,
				&vkgfx_cb_fences[i]) == VK_SUCCESS);
	}


	return NE_OK;
}

void
vkgfx_release_synchronization(void)
{
	sys_mutex_destroy(_update_cb_mutex);
	_update_cb_mutex = NULL;

	for (uint32_t i = 0; i < VKGFX_MAX_SWAPCHAIN_IMAGES; ++i)
		if (vkgfx_cb_fences[i] != VK_NULL_HANDLE)
			vkDestroyFence(vkgfx_device, vkgfx_cb_fences[i], vkgfx_allocator);
}

// Command Buffers

ne_status
vkgfx_init_cmd_buffers(void)
{
	if (rt_array_init(&_update_cmd_buffers, 10, sizeof(VkCommandBuffer))
		!= SYS_OK) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
				  "Failed to initialize update buffer array");
		return NE_FAIL;
	}

	return NE_OK;
}

void
vkgfx_release_cmd_buffers(void)
{
	rt_array_release(&_update_cmd_buffers);
}
