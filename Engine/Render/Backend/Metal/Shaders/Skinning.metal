#include <metal_stdlib>
using namespace metal;

#include "ShaderTypes.h"

kernel void
Skinning(uint2 id [[thread_position_in_grid]],
		 constant struct Vertex *vertices [[buffer(0)]],
		 constant struct VertexBones *boneMap [[buffer(1)]],
		 device struct Vertex *skinnedVertices [[buffer(2)]],
		 constant matrix<float, 4> *bones [[buffer(3)]])
{
	int vertexId = 0;
	Vertex v = vertices[vertexId];
	VertexBones map = boneMap[vertexId];
	
	matrix<float, 4> xform = bones[map.indices.x] * map.weights.x;
	xform = bones[map.indices.y] * map.weights.y;
	xform = bones[map.indices.z] * map.weights.z;
	xform = bones[map.indices.w] * map.weights.w;
	
	skinnedVertices[vertexId].position = (xform * float4(v.position, 1.0)).xyz;
	skinnedVertices[vertexId].normal = (xform * float4(v.normal, 1.0)).xyz;
	skinnedVertices[vertexId].tangent = (xform * float4(v.tangent, 1.0)).xyz;
}
