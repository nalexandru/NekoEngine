#include <Engine/Engine.h>
#include <Animation/Animation.h>

static inline void _LerpVecKey(struct NeVec3 *v, double time, const struct NeArray *keys);
static inline void _LerpQuatKey(struct NeQuaternion *q, double time, const struct NeArray *keys);
static void _XformHierarchy(double time, const struct NeSkeletonNode *n, const struct NeMatrix *parentXform, const struct NeAnimationClip *ac);

bool
Anim_InitSkeleton(struct NeSkeleton *s, const struct NeArray *bones, const struct NeArray *nodes, const struct NeMatrix *git)
{
	Rt_CloneArray(&s->bones, bones, MH_Scene);
	Rt_CloneArray(&s->nodes, nodes, MH_Scene);

	M_CopyMatrix(&s->globalInverseTransform, git);

	return false;
}

void
Anim_UpdateSkeleton(struct NeSkeleton *s, double time, const struct NeAnimationClip *ac)
{
	const double tTime = time * (ac->ticks != 0 ? ac->ticks : 25.0);
	const double aTime = M_Mod(tTime, ac->duration);

	_XformHierarchy(aTime, s->root, NULL, ac);
}

void
Anim_TermSkeleton(struct NeSkeleton *s)
{

}

static inline void
_LerpVecKey(struct NeVec3 *v, double time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		M_CopyVec3(v, &((struct NeAnimVectorKey *)Rt_ArrayGet(keys, 0))->val);
		return;
	}

	size_t idx = 0;
	for (size_t i = 0; i < keys->count - 1; ++i) {
		if (time > ((const struct NeAnimVectorKey *)Rt_ArrayGet(keys, i + 1))->time)
			continue;

		idx = i;
		break;
	}

	const struct NeAnimVectorKey *key = Rt_ArrayGet(keys, idx);
	const struct NeAnimVectorKey *nextKey = Rt_ArrayGet(keys, idx + 1);

	const double f = (time - key->time) / (nextKey->time - key->time);

	struct NeVec3 tmp;
	M_SubVec3(&tmp, &nextKey->val, &key->val);

	M_AddVec3(v, &key->val, M_MulVec3S(&tmp, &tmp, (float)f));
}

static inline void
_LerpQuatKey(struct NeQuaternion *q, double time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		M_CopyQuat(q, &((struct NeAnimQuatKey *)Rt_ArrayGet(keys, 0))->val);
		return;
	}

	size_t idx = 0;
	for (size_t i = 0; i < keys->count - 1; ++i) {
		if (time > ((const struct NeAnimQuatKey *)Rt_ArrayGet(keys, i + 1))->time)
			continue;

		idx = i;
		break;
	}

	const struct NeAnimQuatKey *key = Rt_ArrayGet(keys, idx);
	const struct NeAnimQuatKey *nextKey = Rt_ArrayGet(keys, idx + 1);

	const double f = (time - key->time) / (nextKey->time - key->time);
	M_NormalizeQuat(q, M_SlerpQuat(q, &key->val, &nextKey->val, (float)f));
}

static void
_XformHierarchy(double time, const struct NeSkeletonNode *n, const struct NeMatrix *parentXform, const struct NeAnimationClip *ac)
{
	const struct NeAnimationChannel *ch = NULL;
	Rt_ArrayForEach(ch, &ac->channels) {
		if (ch->hash == n->hash)
			break;
		ch = NULL;
	}

	struct NeMatrix xform;

	if (ch) {
		struct NeVec3 scale;
		_LerpVecKey(&scale, time, &ch->scalingKeys);
		struct NeMatrix scaleMat;
		M_ScaleMatrixV(&scaleMat, &scale);

		struct NeQuaternion rot;
		_LerpQuatKey(&rot, time, &ch->rotationKeys);
		struct NeMatrix rotMat;
		M_RotationMatrixFromQuat(&rotMat, &rot);

		struct NeVec3 pos;
		_LerpVecKey(&pos, time, &ch->positionKeys);
		struct NeMatrix xlateMat;
		M_TranslationMatrixV(&xlateMat, &pos);

		M_MulMatrix(&xform, &xlateMat, &rotMat);
		M_MulMatrix(&xform, &xform, &scaleMat);
	} else {
		M_CopyMatrix(&xform, &n->xform);
	}

	if (parentXform)
		M_MulMatrix(&xform, parentXform, &xform);

	// bone map
	
	struct NeSkeletonNode *child;
	Rt_ArrayForEach(child, &n->children)
		_XformHierarchy(time, child, &xform, ac);
}
