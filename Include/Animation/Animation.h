#ifndef _NE_ANIMATION_ANIMATION_H_
#define _NE_ANIMATION_ANIMATION_H_

#include <Engine/Types.h>
#include <Animation/Clip.h>
#include <Animation/Skeleton.h>
#include <Animation/Animator.h>

#define ANIM_BUILD_SKELETON		L"Anim_BuildSkeleton"
#define ANIM_UPDATE_ANIMATOR	L"Anim_UpdateAnimator"

bool Anim_InitAnimationSystem(void);
void Anim_TermAnimationSystem(void);

#endif /* _NE_ANIMATION_ANIMATION_H_ */
