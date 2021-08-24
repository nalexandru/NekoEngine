#ifndef _NE_RENDER_SYSTEMS_H_
#define _NE_RENDER_SYSTEMS_H_

#include <stdatomic.h>

#include <Math/Math.h>
#include <Render/Types.h>

#define RE_COLLECT_DRAWABLES	L"Re_CollectDrawables"

struct Drawable
{
	BufferHandle indexBuffer;
	uint32_t indexType, firstIndex, indexCount;
	uint32_t vertexCount;
	struct mat4 mvp;
	const struct Material *material;
	float distance;
	uint32_t instanceId;
	const struct ModelInstance *mi;
	uint64_t vertexAddress, materialAddress;
};

struct CollectDrawablesArgs
{
	struct mat4 vp;
	struct Array *opaqueDrawableArrays, *blendedDrawableArrays, *instanceArrays, blendedDrawables;
	uint32_t *instanceOffset;
	uint32_t maxDrawables, requiredDrawables, drawableCount;
	ALIGN(16) _Atomic uint32_t nextArray;
	const struct Scene *s;
	struct vec3 camPos;
};

void Re_CollectDrawables(void **comp, struct CollectDrawablesArgs *args);

#endif /* _NE_RENDER_SYSTEMS_H_ */
