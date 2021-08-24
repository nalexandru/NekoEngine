#ifndef _NE_ANIMATION_CLIP_H_
#define _NE_ANIMATION_CLIP_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Runtime/Runtime.h>

struct AnimVectorKey
{
	struct vec3 val;
	double time;
};

struct AnimQuatKey
{
	struct quat val;
	double time;
};

struct AnimationChannel
{
	uint64_t hash;
	struct Array positionKeys, rotationKeys, scalingKeys;
	char name[256];
};

struct AnimationClip
{
	struct Array channels;
	double ticks;
	double duration;
	char name[256];
};

struct AnimationClipCreateInfo
{
	char name[256];

	double ticks;
	double duration;

	uint32_t channelCount;
	struct {
		char name[256];

		uint32_t positionCount;
		struct AnimVectorKey *positionKeys;

		uint32_t rotationCount;
		struct AnimQuatKey *rotationKeys;

		uint32_t scalingCount;
		struct AnimVectorKey *scalingKeys;
	} *channels;
};

#endif /* _NE_ANIMATION_CLIP_H_ */
