#ifndef _NE_ANIMATION_ANIMATOR_H_
#define _NE_ANIMATION_ANIMATOR_H_

#include <Render/Types.h>
#include <Engine/Component.h>

struct NeAnimator
{
	COMPONENT_BASE;

	struct NeSkeleton *skel;
	bool playing, oneShot, loop, dirty;
	double time, prevTime;
	NeHandle clip, prevClip;
	NeBufferHandle skelBuffer;
};

#endif /* _NE_ANIMATION_ANIMATOR_H_ */
