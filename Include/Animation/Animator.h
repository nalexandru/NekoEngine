#ifndef _NE_ANIMATION_ANIMATOR_H_
#define _NE_ANIMATION_ANIMATOR_H_

#include <Render/Types.h>
#include <Engine/Component.h>

struct Animator
{
	COMPONENT_BASE;

	struct Skeleton *skel;
	bool playing, oneShot, loop;
	double time, prevTime;
	Handle clip, prevClip;
	BufferHandle skelBuffer;
};

bool Anim_InitAnimator(struct Animator *a, const void **args);
void Anim_TermAnimator(struct Animator *a);

void Anim_BuildSkeleton(void **comp, void *args);
void Anim_UpdateAnimator(void **comp, void *args);

#endif /* _NE_ANIMATION_ANIMATOR_H_ */
