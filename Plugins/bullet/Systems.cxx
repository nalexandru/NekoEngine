#include "Plugin.h"

#include <System/Memory.h>
#include <Engine/Entity.h>
#include <Engine/ECSystem.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>

NE_SYSTEM(BT_UPDATE_WORLD, ECSYS_GROUP_POST_LOGIC, 0, false, struct NeCollectDrawablesArgs, 1, NE_PHYSICS_WORLD)
{
	struct BtPhysicsWorld *w = (struct BtPhysicsWorld *)comp[0];

	w->world->performDiscreteCollisionDetection();
	for (int i = 0; i < w->dispatcher->getNumManifolds(); ++i) {
		const btPersistentManifold *m = w->dispatcher->getManifoldByIndexInternal(i);

		struct NeHitInfo *info = (struct NeHitInfo *)Sys_Alloc(sizeof(*info), 2 * m->getNumContacts(), MH_Frame);

		for (int j = 0; j < m->getNumContacts(); ++j) {
			const btManifoldPoint &pt = m->getContactPoint(j);

			struct NeHitInfo *hi = &info[j * 2];
			hi->position.x = pt.getPositionWorldOnA().x();
			hi->position.y = pt.getPositionWorldOnA().y();
			hi->position.z = pt.getPositionWorldOnA().z();
			hi->entity = m->getBody1()->getUserPointer();
			E_SendMessage(m->getBody0()->getUserPointer(), NE_MSG_PHYS_COLLIDER_HIT, hi);

			hi = &info[j * 2 + 1];
			hi->position.x = pt.getPositionWorldOnB().x();
			hi->position.y = pt.getPositionWorldOnB().y();
			hi->position.z = pt.getPositionWorldOnB().z();
			hi->entity = m->getBody0()->getUserPointer();
			E_SendMessage(m->getBody1()->getUserPointer(), NE_MSG_PHYS_COLLIDER_HIT, hi);
		}
	}
}

void
BT_UpdateCollider(void **comp, void *args)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct BtCollider *collider = (struct BtCollider *)comp[1];

	struct NeVec3 v;
	struct NeQuaternion q;

	Xform_Position(xform, &v);
	collider->object->getWorldTransform().setOrigin({ v.x, v.y, v.z });

	Xform_Rotation(xform, &q);
	collider->object->getWorldTransform().setRotation({ q.x, q.y, q.z, q.w });

	Xform_Position(xform, &v);
	collider->shape->setLocalScaling({ v.x, v.y, v.z });
}

NE_REGISTER_SYSTEM(BT_UPDATE_BOX_COLLIDER, ECSYS_GROUP_LOGIC, BT_UpdateCollider, 0, false, 2, NE_TRANSFORM, NE_BOX_COLLIDER)
NE_REGISTER_SYSTEM(BT_UPDATE_SPHERE_COLLIDER, ECSYS_GROUP_LOGIC, BT_UpdateCollider, 0, false, 2, NE_TRANSFORM, NE_SPHERE_COLLIDER)
NE_REGISTER_SYSTEM(BT_UPDATE_CAPSULE_COLLIDER, ECSYS_GROUP_LOGIC, BT_UpdateCollider, 0, false, 2, NE_TRANSFORM, NE_CAPSULE_COLLIDER)
NE_REGISTER_SYSTEM(BT_UPDATE_MESH_COLLIDER, ECSYS_GROUP_LOGIC, BT_UpdateCollider, 0, false, 2, NE_TRANSFORM, NE_MESH_COLLIDER)
