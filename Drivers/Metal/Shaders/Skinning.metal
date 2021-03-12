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
	
	float4 pos = xform * float4(v.x, v.y, v.z, 1.0);
	float4 nrm = xform * float4(v.nx, v.ny, v.nz, 1.0);
	float4 tgt = xform * float4(v.tx, v.ty, v.tz, 1.0);
	
	skinnedVertices[vertexId].x = pos.x;
	skinnedVertices[vertexId].y = pos.y;
	skinnedVertices[vertexId].z = pos.z;
	
	skinnedVertices[vertexId].nx = nrm.x;
	skinnedVertices[vertexId].ny = nrm.y;
	skinnedVertices[vertexId].nz = nrm.z;
	
	skinnedVertices[vertexId].tx = tgt.x;
	skinnedVertices[vertexId].ty = tgt.y;
	skinnedVertices[vertexId].tz = tgt.z;
}
