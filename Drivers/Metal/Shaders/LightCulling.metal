#include <metal_stdlib>
#include <metal_atomic>

using namespace metal;

#include "ShaderTypes.h"

kernel void
LightCulling(texture2d<float, access::read> depthBuffer [[texture(0)]],
			 constant struct Light *lights [[buffer(0)]],
			 device uint *visibleIndices [[buffer(1)]],
			 uint2 location [[thread_position_in_grid]],
			 uint2 item [[thread_position_in_threadgroup]],
			 uint2 tile [[threadgroup_position_in_grid]],
			 uint2 tileCount [[threadgroups_per_grid]],
			 uint id [[thread_index_in_threadgroup]])
{
	/*threadgroup uint minDepthI = 0xFFFFFFFF, maxDepthI = 0, visibleLights = 0;
	
	matrix<float, 4> viewProjection;
	matrix<float, 4> inverseProjection;
	
	threadgroup float4 frustum[6];
	threadgroup uint indices[1024];
	
	if (id == 0) {
		//viewProjection =
		//inverseProjection =
	}
	
	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	float maxDepth, minDepth;
	float depth = 1.0; //depthBuffer.read(ushort2(ushort(location), 0)).r;
	depth = 1.0 / (depth * inverseProjection[2][3] + inverseProjection[3][3]);
	
	uint depthI = (uint)depth;
	//atomic_fetch_min_explicit(&minDepthI, depthI, memory_order_relaxed);
	//atomic_fetch_max_explicit(&maxDepthI, depthI, memory_order_relaxed);
	
	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	if (id == 0) {
		
	}
	
	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	threadgroup_barrier(mem_flags::mem_threadgroup);
	
	if (id == 0) {
		uint offset = (tile.y * tileCount.x + tile.x) * 1024;
		for (uint i = 0; i < visibleLights; ++i)
			visibleIndices[offset + i] = indices[i];
		
		if (visibleLights != 1024)
			visibleIndices[offset + visibleLights] = -1;
	}*/
}
