#ifndef _NE_ANIMATION_ANIMATION_H_
#define _NE_ANIMATION_ANIMATION_H_

#include <Render/Types.h>
#include <Animation/Clip.h>
#include <Animation/Skeleton.h>
#include <Animation/Animator.h>

#define ANIM_BUILD_SKELETON		"Anim_BuildSkeleton"
#define ANIM_UPDATE_ANIMATOR	"Anim_UpdateAnimator"
#define ANIM_COMPUTE_SKINNING	"Anim_ComputeSkinning"

extern struct NePipeline *Anim_pipeline;

bool Anim_InitAnimationSystem(void);
void Anim_TermAnimationSystem(void);

#endif /* _NE_ANIMATION_ANIMATION_H_ */
