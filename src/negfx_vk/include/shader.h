/* NekoEngine
 *
 * shader.h
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Shader
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

#ifndef _NE_SHADER_H_
#define _NE_SHADER_H_

#include <vulkan/vulkan.h>

#include <engine/status.h>

#define VKGFX_STAGE_COUNT	14
#define VKGFX_LAYOUT_COUNT	10
#define VKGFX_CONSTANT_COUNT	10

#define VKGFX_GRAPHICS		0
#define VKGFX_COMPUTE		1
#define VKGFX_RAYTRACE		2

typedef struct vkgfx_shader_module
{
	VkShaderModule module;
} vkgfx_shader_module;

typedef struct vkgfx_shader
{
	struct {
		VkShaderStageFlagBits stage;
		const vkgfx_shader_module *sh_mod;
		struct {
			int32_t id;
			int32_t value;
		} sp_const[VKGFX_CONSTANT_COUNT];
		uint8_t sc_count;
	} stages[VKGFX_STAGE_COUNT];
	VkDescriptorSetLayout layouts[VKGFX_LAYOUT_COUNT];
	VkPushConstantRange pconst_ranges[VKGFX_LAYOUT_COUNT];
	uint8_t layout_count;
	uint8_t stage_count;
	uint8_t pconst_count;
	uint8_t type;

} vkgfx_shader;

void		*shmod_load(const char *path);
void		 shmod_destroy(void *res);

void		*sh_load(const char *path);
void		 sh_destroy(void *res);

#endif /* _NE_SHADER_ */

