#include <Engine/Engine.h>
#include <Animation/Animation.h>

static inline void _LerpVecKey(struct vec3 *v, double time, const struct NeArray *keys);
static inline void _LerpQuatKey(struct quat *q, double time, const struct NeArray *keys);
static void _XformHierarchy(double time, const struct NeSkeletonNode *n, const struct mat4 *parentXform, const struct NeAnimationClip *ac);

bool
Anim_InitSkeleton(struct NeSkeleton *s, const struct NeArray *bones, const struct NeArray *nodes, const struct mat4 *git)
{
	Rt_CloneArray(&s->bones, bones, MH_Scene);
	Rt_CloneArray(&s->nodes, nodes, MH_Scene);

	m4_copy(&s->globalInverseTransform, git);

	return false;
}

void
Anim_UpdateSkeleton(struct NeSkeleton *s, double time, const struct NeAnimationClip *ac)
{
	const double tTime = time * (ac->ticks != 0 ? ac->ticks : 25.0);
	const double aTime = mod(tTime, ac->duration);

	_XformHierarchy(aTime, s->root, NULL, ac);
}

void
Anim_TermSkeleton(struct NeSkeleton *s)
{

}

static inline void
_LerpVecKey(struct vec3 *v, double time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		v3_copy(v, &((struct NeAnimVectorKey *)Rt_ArrayGet(keys, 0))->val);
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

	struct vec3 tmp;
	v3_sub(&tmp, &nextKey->val, &key->val);

	v3_add(v, &key->val, v3_muls(&tmp, &tmp, (float)f));
}

static inline void
_LerpQuatKey(struct quat *q, double time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		quat_copy(q, &((struct NeAnimQuatKey *)Rt_ArrayGet(keys, 0))->val);
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
	quat_norm(q, quat_slerp(q, &key->val, &nextKey->val, (float)f));
}

static void
_XformHierarchy(double time, const struct NeSkeletonNode *n, const struct mat4 *parentXform, const struct NeAnimationClip *ac)
{
	const struct NeAnimationChannel *ch = NULL;
	Rt_ArrayForEach(ch, &ac->channels) {
		if (ch->hash == n->hash)
			break;
		ch = NULL;
	}

	struct mat4 xform;

	if (ch) {
		struct vec3 scale;
		_LerpVecKey(&scale, time, &ch->scalingKeys);
		struct mat4 scaleMat;
		m4_scale(&scaleMat, scale.x, scale.y, scale.z);

		struct quat rot;
		_LerpQuatKey(&rot, time, &ch->rotationKeys);
		struct mat4 rotMat;
		m4_rot_quat(&rotMat, &rot);

		struct vec3 pos;
		_LerpVecKey(&pos, time, &ch->positionKeys);
		struct mat4 xlateMat;
		m4_translate(&xlateMat, pos.x, pos.y, pos.z);

		m4_mul(&xform, &xlateMat, &rotMat);
		m4_mul(&xform, &xform, &scaleMat);
	} else {
		m4_copy(&xform, &n->xform);
	}

	if (parentXform)
		m4_mul(&xform, parentXform, &xform);

	// bone map
	
	struct NeSkeletonNode *child;
	Rt_ArrayForEach(child, &n->children)
		_XformHierarchy(time, child, &xform, ac);
}
