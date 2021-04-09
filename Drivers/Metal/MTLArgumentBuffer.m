#include "MTLDriver.h"

static id<MTLBuffer> _argumentBuffer;
static id<MTLArgumentEncoder> _encoder;

static uint16_t _usedTextures, _usedBuffers;
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
	
	_argumentBuffer = [dev newBufferWithLength: _encoder.encodedLength
									   options: MTLResourceStorageModeManaged];
	[_encoder setArgumentBuffer: _argumentBuffer offset: 0];
	
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
	
	for (uint16_t i = 0; i < _usedTextures; ++i)
		if (_textures[i])
			[encoder useResource: _textures[i] usage: MTLResourceUsageSample];
	
	for (uint16_t i = 0; i < _usedBuffers; ++i)
		if (_buffers[i])
			[encoder useResource: _buffers[i] usage: MTLResourceUsageRead];
}

void MTL_SetComputeArguments(id<MTLComputeCommandEncoder> encoder)
{
	[encoder setBuffer: _argumentBuffer offset: 0 atIndex: 0];
	
	// the usage should be smarter
	for (uint16_t i = 0; i < _usedTextures; ++i)
		if (_textures[i])
			[encoder useResource: _textures[i] usage: MTLResourceUsageWrite];
	
	for (uint16_t i = 0; i < _usedBuffers; ++i)
		if (_buffers[i])
			[encoder useResource: _buffers[i] usage: MTLResourceUsageWrite];
}

void
MTL_TermArgumentBuffer(id<MTLDevice> dev)
{
	[_encoder release];
	[_argumentBuffer release];
}
