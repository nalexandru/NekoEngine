#ifndef _NE_ANIMATION_SKELETON_H_
#define _NE_ANIMATION_SKELETON_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Runtime/Runtime.h>

struct NeBone
{
	uint64_t hash;
	struct NeMatrix offset;
	char name[256];
};

struct NeSkeletonNode
{
	uint64_t hash;
	struct NeMatrix xform;
	struct NeSkeletonNode *parent;
	struct NeArray children;
	char name[256];
};

struct NeSkeleton
{
	struct NeSkeletonNode *root;
	struct NeArray bones, nodes;
	struct NeMatrix globalInverseTransform;
	struct NeArray transforms, prevTransforms;
};

bool Anim_InitSkeleton(struct NeSkeleton *s, const struct NeArray *bones, const struct NeArray *nodes, const struct NeMatrix *git);
void Anim_UpdateSkeleton(struct NeSkeleton *s, double time, const struct NeAnimationClip *ac);
void Anim_TermSkeleton(struct NeSkeleton *s);

#endif /* _NE_ANIMATION_SKELETON_H_ */
