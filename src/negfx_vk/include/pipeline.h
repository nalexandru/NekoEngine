/* NekoEngine
 *
 * pipeline.h
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Pipeline
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

#ifndef _NE_PIPELINE_H_
#define _NE_PIPELINE_H_

#include <vulkan/vulkan.h>

#include <engine/status.h>

#include <shader.h>

/*
 *
 * Vertex type
 */
typedef enum vkgfx_vertex_type
{
	VKGFX_VTX_NORMAL,
	VKGFX_VTX_GUI,

	VKGFX_VTX_TYPE_COUNT
} vkgfx_vertex_type;

typedef struct vkgfx_pipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
	uint64_t flags;
	const vkgfx_shader *shader;
	VkRenderPass rp;
	vkgfx_vertex_type vtx_type;
} vkgfx_pipeline;

/*
 * Graphics pipeline flags
 */
#define	PIPE_TOPOLOGY_POINT_LIST		((uint64_t)  0 <<  0)
#define	PIPE_TOPOLOGY_LINE_LIST			((uint64_t)  1 <<  0)
#define	PIPE_TOPOLOGY_LINE_STRIP		((uint64_t)  2 <<  0)
#define	PIPE_TOPOLOGY_TRIANGLE_LIST		((uint64_t)  3 <<  0)
#define	PIPE_TOPOLOGY_TRIANGLE_STRIP		((uint64_t)  4 <<  0)
#define	PIPE_TOPOLOGY_TRIANGLE_FAN		((uint64_t)  5 <<  0)
#define	PIPE_TOPOLOGY_LINE_LIST_ADJ		((uint64_t)  6 <<  0)
#define	PIPE_TOPOLOGY_LINE_STRIP_ADJ		((uint64_t)  7 <<  0)
#define	PIPE_TOPOLOGY_TRIANGLE_LIST_ADJ		((uint64_t)  8 <<  0)
#define	PIPE_TOPOLOGY_TRIANGLE_STRIP_ADJ	((uint64_t)  9 <<  0)
#define	PIPE_TOPOLOGY_PATCH			((uint64_t) 10 <<  0)
#define	PIPE_TOPOLOGY_BITS			((uint64_t) 15 <<  0)
#define PIPE_TOPOLOGY_OFFSET			0

#define	PIPE_PRIMITIVE_RESTART			((uint64_t)  1 <<  4)

#define	PIPE_POLYGON_FILL			((uint64_t)  0 <<  5)
#define	PIPE_POLYGON_LINE			((uint64_t)  1 <<  5)
#define	PIPE_POLYGON_POINT			((uint64_t)  2 <<  5)
#define PIPE_POLYGON_FILL_RECTANGLE		((uint64_t)  3 <<  5)
#define PIPE_POLYGON_BITS			((uint64_t)  3 <<  5)
#define PIPE_POLYGON_OFFSET			5

#define	PIPE_DISCARD				((uint64_t)  1 <<  7)

#define	PIPE_CULL_NONE				((uint64_t)  0 <<  8)
#define	PIPE_CULL_FRONT				((uint64_t)  1 <<  8)
#define	PIPE_CULL_BACK				((uint64_t)  2 <<  8)
#define	PIPE_CULL_FRONT_AND_BACK		((uint64_t)  3 <<  8)
#define	PIPE_CULL_BITS				((uint64_t)  3 <<  8)
#define PIPE_CULL_OFFSET			8

#define	PIPE_CCW				((uint64_t)  0 << 10)
#define	PIPE_CW					((uint64_t)  1 << 10)
#define	PIPE_DEPTH_CLAMP			((uint64_t)  1 << 11)
#define	PIPE_DEPTH_BIAS				((uint64_t)  1 << 12)

#define	PIPE_MULTISAMPLE			((uint64_t)  1 << 13)
#define PIPE_SAMPLE_SHADING			((uint64_t)  1 << 14)
#define PIPE_ALPHA_TO_COVERAGE			((uint64_t)  1 << 15)
#define PIPE_ALPHA_TO_ONE			((uint64_t)  1 << 16)

#define	PIPE_DEPTH_TEST				((uint64_t)  1 << 17)
#define	PIPE_DEPTH_WRITE			((uint64_t)  1 << 18)
#define	PIPE_DEPTH_BOUNDS			((uint64_t)  1 << 19)

#define	PIPE_DEPTH_OP_NEVER			((uint64_t)  0 << 20)
#define	PIPE_DEPTH_OP_LESS			((uint64_t)  1 << 20)
#define	PIPE_DEPTH_OP_EQUAL			((uint64_t)  2 << 20)
#define	PIPE_DEPTH_OP_LESS_EQUAL		((uint64_t)  3 << 20)
#define	PIPE_DEPTH_OP_GREATER			((uint64_t)  4 << 20)
#define	PIPE_DEPTH_OP_NOT_EQUAL			((uint64_t)  5 << 20)
#define	PIPE_DEPTH_OP_GREATER_EQUAL		((uint64_t)  6 << 20)
#define	PIPE_DEPTH_OP_ALWAYS			((uint64_t)  7 << 20)
#define	PIPE_DEPTH_OP_BITS			((uint64_t)  7 << 20)
#define PIPE_DEPTH_OP_OFFSET			20

#define	PIPE_STENCIL_TEST			((uint64_t)  1 << 23)

#define	PIPE_STENCIL_FRONT_FAIL_KEEP		((uint64_t)  0 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_ZERO		((uint64_t)  1 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_REPLACE		((uint64_t)  2 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_INC_CLAMP	((uint64_t)  3 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_DEC_CLAMP	((uint64_t)  4 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_INVERT		((uint64_t)  5 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_INC_WRAP	((uint64_t)  6 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_DEC_WRAP	((uint64_t)  7 << 24)
#define	PIPE_STENCIL_FRONT_FAIL_BITS		((uint64_t)  7 << 24)
#define PIPE_STENCIL_FRONT_FAIL_OFFSET		24

#define	PIPE_STENCIL_FRONT_PASS_KEEP		((uint64_t)  0 << 27)
#define	PIPE_STENCIL_FRONT_PASS_ZERO		((uint64_t)  1 << 27)
#define	PIPE_STENCIL_FRONT_PASS_REPLACE		((uint64_t)  2 << 27)
#define	PIPE_STENCIL_FRONT_PASS_INC_CLAMP	((uint64_t)  3 << 27)
#define	PIPE_STENCIL_FRONT_PASS_DEC_CLAMP	((uint64_t)  4 << 27)
#define	PIPE_STENCIL_FRONT_PASS_INVERT		((uint64_t)  5 << 27)
#define	PIPE_STENCIL_FRONT_PASS_INC_WRAP	((uint64_t)  6 << 27)
#define	PIPE_STENCIL_FRONT_PASS_DEC_WRAP	((uint64_t)  7 << 27)
#define	PIPE_STENCIL_FRONT_PASS_BITS		((uint64_t)  7 << 27)
#define PIPE_STENCIL_FRONT_PASS_OFFSET		27

#define	PIPE_STENCIL_FRONT_DFAIL_KEEP		((uint64_t)  0 << 30)
#define	PIPE_STENCIL_FRONT_DFAIL_ZERO		((uint64_t)  1 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_REPLACE	((uint64_t)  2 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_INC_CLAMP	((uint64_t)  3 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_DEC_CLAMP	((uint64_t)  4 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_INVERT		((uint64_t)  5 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_INC_WRAP	((uint64_t)  6 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_DEC_WRAP	((uint64_t)  7 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_BITS		((uint64_t)  7 << 30)
#define PIPE_STENCIL_FRONT_DFAIL_OFFSET		30

#define PIPE_STENCIL_FRONT_OP_NEVER		((uint64_t)  0 << 33)
#define PIPE_STENCIL_FRONT_OP_LESS		((uint64_t)  1 << 33)
#define PIPE_STENCIL_FRONT_OP_EQUAL		((uint64_t)  2 << 33)
#define PIPE_STENCIL_FRONT_OP_LESS_EQUAL	((uint64_t)  3 << 33)
#define PIPE_STENCIL_FRONT_OP_GREATER		((uint64_t)  4 << 33)
#define PIPE_STENCIL_FRONT_OP_NOT_EQUAL		((uint64_t)  5 << 33)
#define PIPE_STENCIL_FRONT_OP_GREATER_EQUAL	((uint64_t)  6 << 33)
#define PIPE_STENCIL_FRONT_OP_ALWAYS		((uint64_t)  7 << 33)
#define PIPE_STENCIL_FRONT_OP_BITS		((uint64_t)  7 << 33)
#define PIPE_STENCIL_FRONT_OP_OFFSET		33

#define PIPE_STENCIL_BACK_FAIL_KEEP		((uint64_t)  0 << 36)
#define PIPE_STENCIL_BACK_FAIL_ZERO		((uint64_t)  1 << 36)
#define PIPE_STENCIL_BACK_FAIL_REPLACE		((uint64_t)  2 << 36)
#define PIPE_STENCIL_BACK_FAIL_INC_CLAMP	((uint64_t)  3 << 36)
#define PIPE_STENCIL_BACK_FAIL_DEC_CLAMP	((uint64_t)  4 << 36)
#define PIPE_STENCIL_BACK_FAIL_INVERT		((uint64_t)  5 << 36)
#define PIPE_STENCIL_BACK_FAIL_INC_WRAP		((uint64_t)  6 << 36)
#define PIPE_STENCIL_BACK_FAIL_DEC_WRAP		((uint64_t)  7 << 36)
#define PIPE_STENCIL_BACK_FAIL_BITS		((uint64_t)  7 << 36)
#define PIPE_STENCIL_BACK_FAIL_OFFSET		36

#define PIPE_STENCIL_BACK_PASS_KEEP		((uint64_t)  0 << 39)
#define PIPE_STENCIL_BACK_PASS_ZERO		((uint64_t)  1 << 39)
#define PIPE_STENCIL_BACK_PASS_REPLACE		((uint64_t)  2 << 39)
#define PIPE_STENCIL_BACK_PASS_INC_CLAMP	((uint64_t)  3 << 39)
#define PIPE_STENCIL_BACK_PASS_DEC_CLAMP	((uint64_t)  4 << 39)
#define PIPE_STENCIL_BACK_PASS_INVERT		((uint64_t)  5 << 39)
#define PIPE_STENCIL_BACK_PASS_INC_WRAP		((uint64_t)  6 << 39)
#define PIPE_STENCIL_BACK_PASS_DEC_WRAP		((uint64_t)  7 << 39)
#define PIPE_STENCIL_BACK_PASS_BITS		((uint64_t)  7 << 39)
#define PIPE_STENCIL_BACK_PASS_OFFSET		39

#define PIPE_STENCIL_BACK_DFAIL_KEEP		((uint64_t)  0 << 42)
#define PIPE_STENCIL_BACK_DFAIL_ZERO		((uint64_t)  1 << 42)
#define PIPE_STENCIL_BACK_DFAIL_REPLACE		((uint64_t)  2 << 42)
#define PIPE_STENCIL_BACK_DFAIL_INC_CLAMP	((uint64_t)  3 << 42)
#define PIPE_STENCIL_BACK_DFAIL_DEC_CLAMP	((uint64_t)  4 << 42)
#define PIPE_STENCIL_BACK_DFAIL_INVERT		((uint64_t)  5 << 42)
#define PIPE_STENCIL_BACK_DFAIL_INC_WRAP	((uint64_t)  6 << 42)
#define PIPE_STENCIL_BACK_DFAIL_DEC_WRAP	((uint64_t)  7 << 42)
#define PIPE_STENCIL_BACK_DFAIL_BITS		((uint64_t)  7 << 42)
#define PIPE_STENCIL_BACK_DFAIL_OFFSET		42

#define PIPE_STENCIL_BACK_OP_NEVER		((uint64_t)  0 << 45)
#define PIPE_STENCIL_BACK_OP_LESS		((uint64_t)  1 << 45)
#define PIPE_STENCIL_BACK_OP_EQUAL		((uint64_t)  2 << 45)
#define PIPE_STENCIL_BACK_OP_LESS_EQUAL		((uint64_t)  3 << 45)
#define PIPE_STENCIL_BACK_OP_GREATER		((uint64_t)  4 << 45)
#define PIPE_STENCIL_BACK_OP_NOT_EQUAL		((uint64_t)  5 << 45)
#define PIPE_STENCIL_BACK_OP_GREATER_EQUAL	((uint64_t)  6 << 45)
#define PIPE_STENCIL_BACK_OP_ALWAYS		((uint64_t)  7 << 45)
#define PIPE_STENCIL_BACK_OP_BITS		((uint64_t)  7 << 45)
#define PIPE_STENCIL_BACK_OP_OFFSET		45

#define PIPE_BLEND_ENABLE			((uint64_t)  1 << 48)

#define PIPE_BLEND_LOGIC_OP_CLEAR		((uint64_t)  0 << 49)
#define PIPE_BLEND_LOGIC_OP_AND			((uint64_t)  1 << 49)
#define PIPE_BLEND_LOGIC_OP_AND_REVERSE		((uint64_t)  2 << 49)
#define PIPE_BLEND_LOGIC_OP_COPY		((uint64_t)  3 << 49)
#define PIPE_BLEND_LOGIC_OP_AND_INVERTED	((uint64_t)  4 << 49)
#define PIPE_BLEND_LOGIC_OP_NO_OP		((uint64_t)  5 << 49)
#define PIPE_BLEND_LOGIC_OP_XOR			((uint64_t)  6 << 49)
#define PIPE_BLEND_LOGIC_OP_OR			((uint64_t)  7 << 49)
#define PIPE_BLEND_LOGIC_OP_NOR			((uint64_t)  8 << 49)
#define PIPE_BLEND_LOGIC_OP_EQUIVALENT		((uint64_t)  9 << 49)
#define PIPE_BLEND_LOGIC_OP_INVERT		((uint64_t) 10 << 49)
#define PIPE_BLEND_LOGIC_OP_OR_REVERSE		((uint64_t) 11 << 49)
#define PIPE_BLEND_LOGIC_OP_COPY_INVERTED	((uint64_t) 12 << 49)
#define PIPE_BLEND_LOGIC_OP_OR_INVERTED		((uint64_t) 13 << 49)
#define PIPE_BLEND_LOGIC_OP_NAND		((uint64_t) 14 << 49)
#define PIPE_BLEND_LOGIC_OP_SET			((uint64_t) 15 << 49)
#define PIPE_BLEND_LOGIC_OP_BITS		((uint64_t) 15 << 49)
#define PIPE_BLEND_LOGIC_OP_OFFSET		49

#define PIPE_WRITE_R				((uint64_t)  1 << 53)
#define PIPE_WRITE_G				((uint64_t)  1 << 54)
#define PIPE_WRITE_B				((uint64_t)  1 << 55)
#define PIPE_WRITE_A				((uint64_t)  1 << 56)

#define PIPE_TESSELLATION			((uint64_t)  1 << 57)

// Operations

#define	PIPE_COMPARE_OP_NEVER			0
#define	PIPE_COMPARE_OP_LESS			1
#define	PIPE_COMPARE_OP_EQUAL			2
#define	PIPE_COMPARE_OP_LESS_EQUAL		3
#define	PIPE_COMPARE_OP_GREATER			4
#define	PIPE_COMPARE_OP_NOT_EQUAL		5
#define	PIPE_COMPARE_OP_GREATER_EQUAL		6
#define	PIPE_COMPARE_OP_ALWAYS			7

#define PIPE_STENCIL_OP_KEEP			0
#define PIPE_STENCIL_OP_ZERO			1
#define PIPE_STENCIL_OP_REPLACE			2
#define PIPE_STENCIL_OP_INC_CLAMP		3
#define PIPE_STENCIL_OP_DEC_CLAMP		4
#define PIPE_STENCIL_OP_INVERT			5
#define PIPE_STENCIL_OP_INC_WRAP		6
#define PIPE_STENCIL_OP_DEC_WRAP		7

// Sane defaults

#define PIPE_WRITE_ALL				(PIPE_WRITE_R | PIPE_WRITE_G \
						| PIPE_WRITE_B | PIPE_WRITE_A)

#define PIPE_DEFAULT_TRIANGLES			(PIPE_TOPOLOGY_TRIANGLE_LIST | \
						 PIPE_POLYGON_FILL |	\
						 PIPE_CULL_BACK |	\
						 PIPE_CCW |		\
						 PIPE_WRITE_ALL)
#define PIPE_DEFAULT_TRIANGLE_STRIP		(PIPE_TOPOLOGY_TRIANGLE_STRIP | \
						 PIPE_POLYGON_FILL |	\
						 PIPE_CULL_BACK |	\
						 PIPE_CCW |		\
						 PIPE_WRITE_ALL)
#define PIPE_DEFAULT_DEPTH_WRITE		(PIPE_DEPTH_TEST |	\
						 PIPE_DEPTH_WRITE |	\
						 PIPE_DEPTH_OP_LESS)
#define PIPE_DEFAULT_DEPTH_READ			(PIPE_DEPTH_TEST |	\
						 PIPE_DEPTH_OP_EQUAL)
ne_status	 vkgfx_init_pipeline(void);

vkgfx_pipeline	*pipe_get_graphics(vkgfx_vertex_type, const vkgfx_shader *, VkRenderPass,
					uint32_t, uint64_t, VkPipelineColorBlendAttachmentState *);
vkgfx_pipeline	*pipe_get_compute(const vkgfx_shader *shader);
VkPipeline	 pipe_get_ray_tracing(vkgfx_shader *shader);

void		 vkgfx_release_pipeline(void);

#endif /* _NE_PIPELINE_ */

