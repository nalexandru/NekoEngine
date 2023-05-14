#ifndef NE_BULLET_PLUGIN_H
#define NE_BULLET_PLUGIN_H

#include <bullet/btBulletCollisionCommon.h>

#include <Engine/Component.h>
#include <Interfaces/Physics.h>

#define BT_PLUGIN	"BulletPlugin"

#define BT_UPDATE_WORLD					"BT_UpdateWorld"
#define BT_UPDATE_BOX_COLLIDER			"BT_UpdateBoxCollider"
#define BT_UPDATE_SPHERE_COLLIDER		"BT_UpdateSphereCollider"
#define BT_UPDATE_CAPSULE_COLLIDER		"BT_UpdateCapsuleCollider"
#define BT_UPDATE_MESH_COLLIDER			"BT_UpdateMeshCollider"

struct BtPhysicsWorld
{
	NE_COMPONENT_BASE;

	btCollisionWorld *world;
	btCollisionDispatcher *dispatcher;
	btBroadphaseInterface *broadphase;
};

struct BtCollider
{
	NE_COMPONENT_BASE;

	btCollisionObject *object;
	btCollisionShape *shape;
};

struct BtMeshCollider : public BtCollider
{
	btTriangleIndexVertexArray *tivArray;
};

class NeDebugDraw : public btIDebugDraw
{
public:
	void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;
	void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color) override;
	void reportErrorWarning(const char *warningString) override;
	void draw3dText(const btVector3 &location, const char* textString) override;
	void setDebugMode(int debugMode) override;
	int getDebugMode() const override;

private:
	int _debugMode;
};

extern btCollisionConfiguration *BT_configuration;

void BT_GetBoxHalfExtents(BtCollider *collider, struct NeVec3 *halfExtents);
void BT_SetBoxHalfExtents(BtCollider *collider, const struct NeVec3 *halfExtents);
float BT_GetSphereRadius(BtCollider *collider);
void BT_SetSphereRadius(BtCollider *collider, float radius);
void BT_GetCapsuleDimensions(BtCollider *collider, float *radius, float *height);
void BT_SetCapsuleDimensions(BtCollider *collider, float radius, float height);
void BT_UpdateMesh(BtMeshCollider *collider);

#endif /* NE_BULLET_PLUGIN_H */
