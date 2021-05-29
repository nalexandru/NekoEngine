#include <Render/Render.h>
#include <Engine/Engine.h>
#include <Engine/Resource.h>
#include <Animation/Animation.h>

bool
Anim_InitAnimator(struct Animator *a, const void **args)
{
	a->clip = E_INVALID_HANDLE;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Loop", len)) {
			a->loop = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "OneShot", len)) {
			a->oneShot = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "Play", len)) {
			a->playing = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "Clip", len)) {
			a->clip = E_LoadResource(*(++args), RES_ANIMATION_CLIP);
/*		 } else if (!strncmp(arg, "Aperture", len)) {
			cam->zFar = (float)atof(*(++args));*/
		}
	}

	return true;
}

void
Anim_TermAnimator(struct Animator *a)
{
	E_UnloadResource(a->clip);

	if (!a->skel)
		return;

	Anim_TermSkeleton(a->skel);
	Re_Destroy(a->skelBuffer);
}

void
Anim_BuildSkeleton(void **comp, void *args)
{
	struct Animator *a = comp[0];
	struct ModelRender *mr = comp[1];

	Anim_InitSkeleton(a->skel, NULL, NULL, NULL);

	struct BufferCreateInfo bci = 
	{
		.desc = {
			.size = sizeof(struct mat4) * 0,
			.usage = BU_TRANSFER_DST | BU_STORAGE_BUFFER,
			.memoryType = MT_GPU_LOCAL
		}
	};
	Re_CreateBuffer(&bci, &a->skelBuffer);
}

void
Anim_UpdateAnimator(void **comp, void *args)
{
	struct Animator *a = comp[0];
	struct ModelRender *mr = comp[1];
	const struct AnimationClip *ac = E_ResourcePtr(a->clip);

	if (!a->playing || !ac)
		return;

	a->time += E_deltaTime;
	if (a->time > ac->duration) {
		if (a->loop) {
			a->time = 0.0;
		} else if (a->oneShot) {
			a->clip = a->prevClip;
			a->time = a->prevTime;
			ac = E_ResourcePtr(a->clip);
			if (!ac)
				return;
		} else {
			a->playing = false;
		}
	}

	Anim_UpdateSkeleton(a->skel, a->time, ac);
}
