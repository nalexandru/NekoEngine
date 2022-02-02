#ifndef _NE_RENDER_SYSTEMS_H_
#define _NE_RENDER_SYSTEMS_H_

#include <stdatomic.h>

#include <Math/Math.h>
#include <Render/Types.h>

#define RE_COLLECT_DRAWABLES	"Re_CollectDrawables"

struct NeDrawable
{
	NeBufferHandle indexBuffer;
	uint32_t indexType, firstIndex, indexCount;
	uint32_t vertexCount;
	struct mat4 mvp;
	const struct NeMaterial *material;
	float distance;
	uint32_t instanceId;
	const struct NeModelInstance *mi;
	uint64_t vertexAddress, materialAddress;
};

struct NeCollectDrawablesArgs
{
	struct mat4 vp;
	struct NeArray *opaqueDrawableArrays, *blendedDrawableArrays, *instanceArrays, blendedDrawables;
	uint32_t *instanceOffset;
	uint32_t maxDrawables, requiredDrawables, drawableCount;
	NE_ALIGN(16) _Atomic uint32_t nextArray;
	const struct NeScene *s;
	struct vec3 camPos;
};

#endif /* _NE_RENDER_SYSTEMS_H_ */
