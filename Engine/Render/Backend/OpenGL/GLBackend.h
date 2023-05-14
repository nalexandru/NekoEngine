#ifndef GL_BACKEND_H
#define GL_BACKEND_H

#include <assert.h>

// Build configuration

#include <Engine/BuildConfig.h>

// End build configuration

#include "glad.h"

#if ENABLE_OPENXR
#	define XR_USE_GRAPHICS_API_OPENGL
#	include <openxr/openxr.h>
#	include <openxr/openxr_platform.h>
#endif

#define RE_NATIVE_OPENGL
#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Render/RayTracing.h>
#include <Runtime/Array.h>
#include <System/Thread.h>
#include <System/Log.h>

#include "../Compat/CommandBuffer.h"
#include "../Compat/TransientResource.h"

#ifdef __cplusplus

extern "C" {

struct GLCommandBuffer : public NeCommandBuffer
{
	uint8_t pushConstants[128];
};

#else

struct GLCommandBuffer;

#endif

#define GLBK_MOD	"OpenGLBackend"

#define NE_ENABLE_GL_LOGGING

#if defined(NE_STOP_ON_GL_ERROR)
#	define GL_TRACE(x) x; { GLenum err = glGetError(); if (err != GL_NO_ERROR) Sys_LogEntry(GLBK_MOD, LOG_WARNING, "%s call from %s:%d returned 0x%x", #x, __FILE__, __LINE__, err); assert(err == GL_NO_ERROR); }
#elif defined(NE_ENABLE_GL_LOGGING)
#	define GL_TRACE(x) x; { GLenum err = glGetError(); if (err != GL_NO_ERROR) Sys_LogEntry(GLBK_MOD, LOG_WARNING, "%s call from %s:%d returned 0x%x", #x, __FILE__, __LINE__, err); }
#else
#	define GL_TRACE(x) x
#endif

#define GL_ASSERT(x) x; assert(glGetError() == GL_NO_ERROR)

extern struct GLBkVersion
{
	GLuint major, minor;
} GLBk_contextVersion;

struct GLBkPipelineState
{
	GLenum polygonMode;
	GLenum frontFace;
	GLenum fillMode;
	GLenum cullMode;
	bool rasterizerDiscard;

	bool depthTest, depthWrite, depthBounds, depthClamp, depthBias;
	GLenum depthFunc;
	GLclampd minDepthBounds, maxDepthBounds;

	bool multisampling, sampleShading, alphaToCoverage, alphaToOne;
	uint32_t samples;
};

enum GLBkPipelineType
{
	PT_GRAPHICS = 0,
	PT_COMPUTE = 1
};

enum GLBkUniformType
{
	UT_NONE,
	UT_INT,
	UT_UINT,
	UT_FLOAT,
	UT_MAT3,
	UT_MAT4,
	UT_TEX2D,
	UT_TEX3D,
	UT_VEC3,
	UT_VEC4
};

struct GLBkShaderUniform
{
	enum GLBkUniformType type;
	char name[256];
	size_t size;
};

struct GLBkPipelineUniform
{
	enum GLBkUniformType type;
	GLuint location;
	size_t size;
};

struct NeRenderDevice
{
	struct GLBkPipelineState state;

	float clearColor[4];
	struct {
		float clearDepth;
		uint8_t clearStencil;
	};
};

struct NeRenderContext
{
	struct GLCommandBuffer *cmdBuffer;
	struct NePipeline *boundPipeline;
	struct NeRenderPassDesc *boundRenderPass;

	struct NeArray queued;
	struct NeQueue free;

	void *glContext;
};

struct NeSwapchain
{
	GLint swapInterval;
};

#if ENABLE_OPENXR
struct XrExtProcs
{
	PFN_xrGetVulkanGraphicsRequirements2KHR GetVulkanGraphicsRequirements2;
};

extern struct XrExtProcs Vkd_XRExt;

struct XRSwapchain
{
	struct NeSwapchain vkSwapchain;
	XrSwapchain colorSwapchain, depthSwapchain;
	XrActionSet actionSet;
	XrPath paths[2];
	struct
	{
		XrAction place;
		XrAction pose;
		XrAction exit;
		XrAction vibrateLeft;
		XrAction vibrateRight;
	} actions;
	XrSystemId sysId;
	XrSpace sceneSpace, leftHandSpace, rightHandSpace;
	uint32_t imageCount;
	XrSwapchainImageBaseHeader *colorImages, *depthImages;
};

extern XrInstance Vkd_xrInst;
extern XrSession Vkd_xrSession;
extern XrSessionState Vkd_xrState;
#endif

struct NeBuffer
{
	GLuint id;
	GLenum bindPoint;
	GLuint64 handle;
	GLenum usage;
	GLbitfield flags;

	void *persistent, *staging;
	GLsync fences[RE_NUM_FRAMES];

	uint64_t hash;
	bool transient;
};

struct NeSampler
{
	GLint magFilter, minFilter;
	GLint addressModeU, addressModeV, addressModeW;

	bool anisotropicFiltering;
	float maxAnisotropy;

	bool compare;
	GLint compareOp;

	float lodBias, minLod, maxLod;
	float borderColor[4];
};

struct NeTexture
{
	GLuint id;
	GLenum bindPoint;
	GLuint64 handle;
	GLenum format;
	union {
		GLenum type;
		GLsizei formatSize;
	};
	GLint internalFormat;

	uint64_t hash;
	bool transient;

	struct NeSampler sampler;
};

struct NeFramebuffer
{
	GLuint id;
	uint32_t width, height, layers, attachmentCount;
	uint64_t hash;
};

struct GLBkRenderPassAttachment
{
	bool clear, store;
	union {
		float clearColor[4];
		struct {
			float clearDepth;
			uint8_t clearStencil;
		};
	};
};

struct NeRenderPassDesc
{
	uint32_t count;
	struct GLBkRenderPassAttachment color[8];
	GLenum drawBuffers[8];

	bool hasDepth;
	struct GLBkRenderPassAttachment depth;

	uint32_t inputCount;
	struct GLBkRenderPassAttachment input[8];
	GLenum readBuffers[8];
};

struct GLBkShaderModule
{
	GLuint id;
	struct NeArray uniforms;
};

struct NePipeline
{
	enum GLBkPipelineType type;
	GLuint program, vao, vbo, ibo;
	uint64_t pushConstantSize;

	struct GLBkPipelineState ps;
	struct NeBlendAttachmentDesc attachments[8];

	struct NeArray uniforms;
};

struct NeSemaphore
{
	uint64_t value;
};

struct NeAccelerationStructure
{
	uint8_t empty;
};

struct NeFence
{
	GLsync sync;
};

struct GLTransientResource
{
	uint64_t hash;
	GLuint id;
};

extern GLuint GLBk_textures[];

// Platform
bool GLBk_InitContext(void);
void *GLBk_CreateShareContext(void);
void GLBk_DestroyShareContext(void *ctx);
void GLBk_MakeCurrent(void *ctx);
void GLBk_EnableVerticalSync(bool enable);
void GLBk_SwapBuffers(void);
void GLBk_HardwareInfo(struct NeRenderDeviceInfo *info);
void GLBk_TermContext(void);

// Debug
bool GLBk_InitDebug(void);
//bool GLBk_SetObjectName(VkDevice dev, void *handle, VkObjectType type, const char *name);
void GLBk_TermDebug(void);

// Context
void GLBk_ExecuteCommands();

// Pipeline
void GLBk_ApplyPipelineState(const struct GLBkPipelineState *ps);

// Buffer
bool GLBk_CreateBuffer(const struct NeBufferDesc *desc, struct NeBuffer *buff);

// Texture
bool GLBk_CreateTexture(const struct NeTextureDesc *desc, struct NeTexture *tex);
void GLBk_UploadImage(struct NeTexture *tex, const struct NeBuffer *buff, struct NeBufferImageCopy *bic);

// Sampler
void GLBk_SetSampler(struct NeTexture *tex, const struct NeSampler *s);

// Descriptor Set
/*bool Vk_CreateDescriptorSet(struct NeRenderDevice *dev);
VkDescriptorSet Vk_AllocateIADescriptorSet(struct NeRenderDevice *dev);
void Vk_SetSampler(struct NeRenderDevice *dev, uint16_t location, VkSampler sampler);
void Vk_SetTexture(uint16_t location, VkImageView imageView);
void Vk_SetInputAttachment(struct NeRenderDevice *dev, VkDescriptorSet set, uint16_t location, VkImageView imageView);
void Vk_TermDescriptorSet(struct NeRenderDevice *dev);*/

// Shader
bool GLBk_LoadShaders(void);
void GLBk_UnloadShaders(void);

// Transient resources
void GLBk_ReleaseTransientTexture(GLuint id, uint64_t hash);
void GLBk_ReleaseTransientBuffer(GLuint id, uint64_t hash);

// Staging; support for systems without CPU visible device local memory (eg. Windows 7)
/*bool Vkd_InitStagingArea(struct NeRenderDevice *dev);
void *Vkd_AllocateStagingMemory(VkDevice dev, VkBuffer buff, VkMemoryRequirements *mr);
void Vkd_CommitStagingArea(struct NeRenderDevice *dev, VkSemaphore wait);
void Vkd_TermStagingArea(struct NeRenderDevice *dev);
void Vkd_StagingBarrier(VkCommandBuffer cmdBuffer);*/

static inline bool
GLBkCompressedFormat(GLenum fmt)
{
	switch (fmt) {
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
		case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
		case GL_COMPRESSED_RGB8_ETC2:
		case GL_COMPRESSED_SRGB8_ETC2:
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_R11_EAC:
		case GL_COMPRESSED_SIGNED_R11_EAC:
		case GL_COMPRESSED_RG11_EAC:
		case GL_COMPRESSED_SIGNED_RG11_EAC:
			return true;
		default:
			return false;
	}
}

static inline GLenum
NeToGLSizedTextureFormat(enum NeTextureFormat fmt)
{
	switch (fmt) {
		case TF_R8G8B8A8_UNORM: return GL_RGBA8;
		case TF_R8G8B8A8_SRGB: return GL_SRGB8_ALPHA8;
		case TF_B8G8R8A8_UNORM: return GL_BGRA8_EXT;
		case TF_B8G8R8A8_SRGB: return GL_SRGB8_ALPHA8;
		case TF_R16G16B16A16_SFLOAT: return GL_RGBA16F_ARB;
		case TF_R32G32B32A32_SFLOAT: return GL_RGBA32F_ARB;
		case TF_A2R10G10B10_UNORM: return GL_RGB10_A2;
		case TF_D32_SFLOAT: return GL_DEPTH_COMPONENT32;
		case TF_D24_STENCIL8: return GL_DEPTH24_STENCIL8;
		case TF_R8G8_UNORM: return GL_RG8;
		case TF_R8_UNORM: return GL_R8;
		case TF_BC5_UNORM: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TF_BC5_SNORM: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
		case TF_BC6H_UF16: return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
		case TF_BC6H_SF16: return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
		case TF_BC7_UNORM: return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
		case TF_BC7_SRGB: return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
		case TF_ETC2_R8G8B8_UNORM: return GL_COMPRESSED_RGB8_ETC2;
		case TF_ETC2_R8G8B8_SRGB: return GL_COMPRESSED_SRGB8_ETC2;
		case TF_ETC2_R8G8B8A1_UNORM: return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case TF_ETC2_R8G8B8A1_SRGB: return GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case TF_EAC_R11_UNORM: return GL_COMPRESSED_R11_EAC;
		case TF_EAC_R11_SNORM: return GL_COMPRESSED_SIGNED_R11_EAC;
		case TF_EAC_R11G11_UNORM: return GL_COMPRESSED_RG11_EAC;
		case TF_EAC_R11G11_SNORM: return GL_COMPRESSED_SIGNED_RG11_EAC;
		default: return GL_INVALID_ENUM;
	}
}

static inline GLenum
NeToGLTexFilter(enum NeImageFilter f, enum NeSamplerMipmapMode mode)
{
	if (mode == SMM_LINEAR) {
		switch (f) {
		case IF_NEAREST: return GL_NEAREST_MIPMAP_LINEAR;
		case IF_LINEAR:
		case IF_CUBIC: return GL_LINEAR_MIPMAP_LINEAR;
		}
	} else {
		switch (f) {
			case IF_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
			case IF_LINEAR:
			case IF_CUBIC: return GL_LINEAR_MIPMAP_NEAREST;
		}
	}
	return GL_NEAREST;
}

static inline GLenum
NeToGLAddressMode(enum NeSamplerAddressMode mode)
{
	switch (mode) {
		case SAM_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
		case SAM_CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
		case SAM_CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
		case SAM_MIRROR_CLAMP_TO_EDGE: return GL_MIRROR_CLAMP_TO_EDGE;
		case SAM_REPEAT:
		default: return GL_REPEAT;
	}
}

static inline GLenum
NeToGLCompareOp(enum NeCompareOperation op)
{
	switch(op) {
		case CO_NEVER: return GL_NEVER;
		case CO_LESS: return GL_LESS;
		case CO_EQUAL: return GL_EQUAL;
		case CO_LESS_EQUAL: return GL_LEQUAL;
		case CO_GREATER: return GL_GREATER;
		case CO_NOT_EQUAL: return GL_NOTEQUAL;
		case CO_GREATER_EQUAL: return GL_GEQUAL;
		case CO_ALWAYS:
		default: return GL_ALWAYS;
	}
}

#ifdef __cplusplus
}
#endif

#endif /* GL_BACKEND_H */

/* NekoEngine
 *
 * GLBackend.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
