#ifndef _NE_RENDER_SYSTEMS_H_
#define _NE_RENDER_SYSTEMS_H_

#include <stdatomic.h>

#include <Math/Math.h>
#include <Render/Types.h>

#define RE_COLLECT_DRAWABLES	"Re_CollectDrawables"

struct NeDrawable
{
	NeBufferHandle indexBuffer, vertexBuffer;
	uint64_t vertexOffset;
	uint32_t indexType, firstIndex, indexCount;
	uint32_t vertexCount;
	struct NeMatrix mvp;
	const struct NeMaterial *material;
	float distance;
	uint32_t instanceId;
	const struct NeModelInstance *mi;
	uint64_t vertexAddress, materialAddress;
	const struct NeBounds *bounds;
	const struct NeMatrix *modelMatrix;
};

struct NeCollectDrawablesArgs
{
	struct NeMatrix vp;
	struct NeArray *opaqueDrawableArrays, *blendedDrawableArrays, *instanceArrays, blendedDrawables;
	uint32_t *instanceOffset;
	uint32_t maxDrawables, requiredDrawables, drawableCount;
	NE_ALIGN(16) _Atomic uint32_t nextArray, totalDrawables, visibleDrawables;
	const struct NeScene *s;
	struct NeVec3 camPos;
	struct NeFrustum camFrustum;
};

#endif /* _NE_RENDER_SYSTEMS_H_ */
