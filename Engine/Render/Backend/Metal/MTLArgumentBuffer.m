#include "MTLBackend.h"

static id<MTLBuffer> _argumentBuffer;
static id<MTLArgumentEncoder> _encoder;

static uint16_t _usedTextures, _usedBuffers, _transientTextureStart;
static id<MTLTexture> _textures[UINT16_MAX];
static id<MTLBuffer> _buffers[UINT16_MAX];

bool
MTL_InitArgumentBuffer(id<MTLDevice> dev)
{
	NSMutableArray<MTLArgumentDescriptor *> *args = [[NSMutableArray alloc] init];
	
	MTLArgumentDescriptor *desc = nil;
	
	desc = [MTLArgumentDescriptor argumentDescriptor];
	desc.index = 0;
	desc.dataType = MTLDataTypeSampler;
	desc.arrayLength = 3;
	[args addObject: desc];
	
	desc = [MTLArgumentDescriptor argumentDescriptor];
	desc.index = 3;
	desc.dataType = MTLDataTypeTexture;
	desc.arrayLength = UINT16_MAX;
	[args addObject: desc];
	
	desc = [MTLArgumentDescriptor argumentDescriptor];
	desc.index = UINT16_MAX + 3;
	desc.dataType = MTLDataTypePointer;
	desc.arrayLength = UINT16_MAX;
	[args addObject: desc];
	
	_encoder = [dev newArgumentEncoderWithArguments: args];
	
	[args release];
	
#if TARGET_OS_OSX
	_argumentBuffer = [dev newBufferWithLength: _encoder.encodedLength
									   options: MTLResourceStorageModeManaged];
#else
	_argumentBuffer = [dev newBufferWithLength: _encoder.encodedLength
									   options: MTLResourceStorageModeShared];
#endif
	
	[_encoder setArgumentBuffer: _argumentBuffer offset: 0];
	
	_transientTextureStart = (uint16_t)(UINT16_MAX - E_GetCVarU32("Render_TransientTextureHandles", 35)->u32);
	
	return true;
}

void
MTL_SetSampler(uint16_t location, id<MTLSamplerState> sampler)
{
	[_encoder setSamplerState: sampler atIndex: location];
}

void
MTL_SetTexture(uint16_t location, id<MTLTexture> tex)
{
	_textures[location] = tex;
	[_encoder setTexture: tex atIndex: (uint32_t)location + 3];
	if (location + 1 > _usedTextures)
		_usedTextures = location + 1;
}

void
MTL_RemoveTexture(id<MTLTexture> tex)
{
	for (uint16_t i = 0; i < _usedTextures; ++i) {
		if (_textures[i] != tex)
			continue;
		
		_textures[i] = nil;
		
		if (_usedTextures == i - 1)
			--_usedTextures;
		
		break;
	}
}

void
MTL_SetBuffer(uint16_t location, id<MTLBuffer> buff)
{
	_buffers[location] = buff;
	[_encoder setBuffer: buff offset: 0 atIndex: (uint32_t)location + UINT16_MAX + 3];
	if (location + 1 > _usedBuffers)
		_usedBuffers = location + 1;
}

void
MTL_RemoveBuffer(id<MTLBuffer> buff)
{
	for (uint16_t i = 0; i < _usedBuffers; ++i) {
		if (_buffers[i] != buff)
			continue;
		
		_buffers[i] = nil;
		
		if (_usedBuffers == i - 1)
			--_usedBuffers;
		
		break;
	}
}

void
MTL_SetRenderArguments(id<MTLRenderCommandEncoder> encoder)
{
	[encoder setVertexBuffer: _argumentBuffer offset: 0 atIndex: 0];
	[encoder setFragmentBuffer: _argumentBuffer offset: 0 atIndex: 0];

	for (uint16_t i = 0; i < _usedTextures; ++i) {
		if (!_textures[i])
			continue;
		
		MTLResourceUsage usage = MTLResourceUsageRead;
		if (i >= _transientTextureStart)
			usage |= MTLResourceUsageWrite;
		
		[encoder useResource: _textures[i] usage: usage stages: MTLRenderStageFragment];
	}
	
	if (@available(macOS 13, iOS 16, *)) {
		for (uint16_t i = 0; i < _usedBuffers; ++i)
			if (_buffers[i])
				[encoder useResource: _buffers[i] usage: MTLResourceUsageRead stages: MTLRenderStageVertex | MTLRenderStageFragment |
				 MTLRenderStageMesh | MTLRenderStageTile | MTLRenderStageObject];
	} else {
		for (uint16_t i = 0; i < _usedBuffers; ++i)
			if (_buffers[i])
				[encoder useResource: _buffers[i] usage: MTLResourceUsageRead stages: MTLRenderStageVertex | MTLRenderStageFragment];
	}
}

void MTL_SetComputeArguments(id<MTLComputeCommandEncoder> encoder)
{
	[encoder setBuffer: _argumentBuffer offset: 0 atIndex: 0];

	for (uint16_t i = 0; i < _usedTextures; ++i)
		if (_textures[i])
			[encoder useResource: _textures[i] usage: MTLResourceUsageRead | MTLResourceUsageWrite];

	for (uint16_t i = 0; i < _usedBuffers; ++i)
		if (_buffers[i])
			[encoder useResource: _buffers[i] usage: MTLResourceUsageRead | MTLResourceUsageWrite];
}

void
MTL_TermArgumentBuffer(id<MTLDevice> dev)
{
	[_encoder release];
	[_argumentBuffer release];
}

/* NekoEngine
 *
 * MTLArgumentBuffer.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
