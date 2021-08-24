#ifndef _NE_ANIMATION_SKELETON_H_
#define _NE_ANIMATION_SKELETON_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Runtime/Runtime.h>

struct Bone
{
	uint64_t hash;
	struct mat4 offset;
	char name[256];
};

struct SkeletonNode
{
	uint64_t hash;
	struct mat4 xform;
	struct SkeletonNode *parent;
	struct Array children;
	char name[256];
};

struct Skeleton
{
	struct SkeletonNode *root;
	struct Array bones, nodes;
	struct mat4 globalInverseTransform;
	struct Array transforms, prevTransforms;
};

bool Anim_InitSkeleton(struct Skeleton *s, const struct Array *bones, const struct Array *nodes, const struct mat4 *git);
void Anim_UpdateSkeleton(struct Skeleton *s, double time, const struct AnimationClip *ac);
void Anim_TermSkeleton(struct Skeleton *s);

#endif /* _NE_ANIMATION_SKELETON_H_ */
