#include <Render/Render.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Engine/Resource.h>
#include <Animation/Animation.h>
#include <Scene/Components.h>

static bool _InitAnimator(struct NeAnimator *a, const void **args);
static void _TermAnimator(struct NeAnimator *a);

E_REGISTER_COMPONENT(ANIMATOR_COMP, struct NeAnimator, 16, _InitAnimator, _TermAnimator)

static bool
_InitAnimator(struct NeAnimator *a, const void **args)
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

E_SYSTEM(ANIM_BUILD_SKELETON, ECSYS_GROUP_MANUAL, 0, false, void, 2, ANIMATOR_COMP, MODEL_RENDER_COMP)
{
	struct NeAnimator *a = comp[0];
	struct NeModelRender *mr = comp[1];

	const struct NeModel *m = E_ResourcePtr(mr->model);
	if (!m)
		return;

	Anim_InitSkeleton(a->skel, &m->skeleton.bones, &m->skeleton.nodes, &m->skeleton.globalInverseTransform);

	struct NeBufferCreateInfo bci = 
	{
		.desc = {
			.size = sizeof(struct NeMatrix) * m->skeleton.bones.count,
			.usage = BU_TRANSFER_DST | BU_STORAGE_BUFFER,
			.memoryType = MT_GPU_LOCAL
		}
	};
	Re_CreateBuffer(&bci, &a->skelBuffer);
}

E_SYSTEM(ANIM_UPDATE_ANIMATOR, ECSYS_GROUP_PRE_RENDER, 0, false, void, 1, ANIMATOR_COMP)
{
	struct NeAnimator *a = comp[0];
	const struct NeAnimationClip *ac = E_ResourcePtr(a->clip);

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

	a->dirty = true;
}

E_SYSTEM(ANIM_COMPUTE_SKINNING, ECSYS_GROUP_RENDER, ECSYS_PRI_SKINNING, false, void, 2, ANIMATOR_COMP, MODEL_RENDER_COMP)
{
	struct NeAnimator *a = comp[0];
	struct NeModelRender *mr = comp[1];

	struct {
		uint64_t bones;
		uint64_t source;
		uint64_t destination;
	} constants;

	if (!a->dirty)
		return;

	Re_BeginComputeCommandBuffer();

	Re_CmdBindPipeline(Anim_pipeline);

	struct NeModel *mdl = E_ResourcePtr(mr->model);
	constants.bones = Re_BufferAddress(a->skelBuffer, 0);
	constants.source = Re_BufferAddress(mdl->gpu.vertexBuffer, 0);
	constants.destination = Re_BufferAddress(mr->vertexBuffer, 0);
	Re_CmdPushConstants(SS_COMPUTE, sizeof(constants), &constants);

	Re_CmdDispatch((uint32_t)a->skel->bones.count, 1, 1);

	Re_QueueCompute(Re_EndCommandBuffer(), NULL, NULL);

	a->dirty = false;
}

static void
_TermAnimator(struct NeAnimator *a)
{
	E_UnloadResource(a->clip);

	if (!a->skel)
		return;

	Anim_TermSkeleton(a->skel);
	Re_Destroy(a->skelBuffer);
}
