#ifndef _NE_ANIMATION_CLIP_H_
#define _NE_ANIMATION_CLIP_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Runtime/Runtime.h>

struct NeAnimVectorKey
{
	struct NeVec3 val;
	double time;
};

struct NeAnimQuatKey
{
	struct NeQuaternion val;
	double time;
};

struct NeAnimationChannel
{
	uint64_t hash;
	struct NeArray positionKeys, rotationKeys, scalingKeys;
	char name[256];
};

struct NeAnimationClip
{
	struct NeArray channels;
	double ticks;
	double duration;
	char name[256];
};

struct NeAnimationClipCreateInfo
{
	char name[256];

	double ticks;
	double duration;

	uint32_t channelCount;
	struct {
		char name[256];

		uint32_t positionCount;
		struct NeAnimVectorKey *positionKeys;

		uint32_t rotationCount;
		struct NeAnimQuatKey *rotationKeys;

		uint32_t scalingCount;
		struct NeAnimVectorKey *scalingKeys;
	} *channels;
};

#endif /* _NE_ANIMATION_CLIP_H_ */
