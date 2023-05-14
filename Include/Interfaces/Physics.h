#ifndef NE_INTERFACES_PHYSICS_H
#define NE_INTERFACES_PHYSICS_H

#include <Math/Types.h>

#define NE_PHYSICS_WORLD		"PhysicsWorld"
#define NE_BOX_COLLIDER			"BoxCollider"
#define NE_SPHERE_COLLIDER		"SphereCollider"
#define NE_CAPSULE_COLLIDER		"CapsuleCollider"
#define NE_MESH_COLLIDER		"MeshCollider"

#define NE_MSG_PHYS_DEBUG_DRAW			0x000A0001
#define NE_MSG_PHYS_COLLIDER_HIT		0x000A0002

struct NeHitInfo
{
	NeEntityHandle entity;
	struct NeVec3 position;
};

struct NePhysics
{
	bool (*rayCast)(struct NeScene *scn, struct NeVec3 *start, struct NeVec3 *end, struct NeHitInfo *hi);

	void (*getBoxHalfExtents)(void *collider, struct NeVec3 *halfExtents);
	void (*setBoxHalfExtents)(void *collider, const struct NeVec3 *halfExtents);

	float (*getSphereRadius)(void *collider);
	void (*setSphereRadius)(void *collider, float radius);

	void (*getCapsuleDimensions)(void *collider, float *radius, float *height);
	void (*setCapsuleDimensions)(void *collider, float radius, float height);

	void (*updateMesh)(void *collider);

	/*void (*getRigidBodyGravity)(void *rb, struct NeVec3 *gravity);
	void (*setRigidBodyGravity)(void *rb, const struct NeVec3 *gravity);

	void (*getRigidBodyDamping)(void *rb float *linearDamping, float *angularDamping);
	void (*setRigidBodyDamping)(void *rb float linearDamping, float angularDamping);

	void (*getRigidBodyLinearSleepingThreshold)(void *rb);
	void (*getAngularSleepingThreshold)(void *rb);

	void (*applyRigidBodyDamping)(void *rb, float timeStep);*/
};

#ifdef __cplusplus
extern "C" {
#endif

extern NeCompTypeId NE_PHYSICS_WORLD_ID;
extern NeCompTypeId NE_BOX_COLLIDER_ID;
extern NeCompTypeId NE_SPHERE_COLLIDER_ID;
extern NeCompTypeId NE_CAPSULE_COLLIDER_ID;
extern NeCompTypeId NE_MESH_COLLIDER_ID;

#ifdef __cplusplus
}
#endif

#endif /* NE_INTERFACES_PHYSICS_H */
