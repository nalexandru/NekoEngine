#include "Plugin.h"

#include <Math/Math.h>
#include <Engine/Entity.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Render/Model.h>
#include <Render/Components/ModelRender.h>

static bool
InitWorld(struct BtPhysicsWorld *w, const char **args)
{
	float size = 2000.f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strnlen(arg, UINT16_MAX);

		if (!strncmp(arg, "Radius", len))
			size = (float)atof(*(++args));
	}

	btVector3 min(-size, -size, -size);
	btVector3 max(size, size, size);

	w->broadphase = new bt32BitAxisSweep3(min, max);
	w->dispatcher = new btCollisionDispatcher(BT_configuration);
	w->world = new btCollisionWorld(w->dispatcher, w->broadphase, BT_configuration);

	return true;
}

static void
WorldMessageHandler(BtPhysicsWorld *w, uint32_t msg, const void *data)
{
	switch (msg) {
	case NE_MSG_PHYS_DEBUG_DRAW: {
		NeDebugDraw *dd = (NeDebugDraw *)w->world->getDebugDrawer();

		if (data && !dd) {
			w->world->setDebugDrawer(new NeDebugDraw());
		} else if (!data && dd) {
			w->world->setDebugDrawer(nullptr);
			delete dd;
		}
	} break;
	}
}

static void
TermWorld(struct BtPhysicsWorld *w)
{
	delete w->world;
	delete w->broadphase;
	delete w->dispatcher;
}
NE_REGISTER_COMPONENT(NE_PHYSICS_WORLD, struct BtPhysicsWorld, 16, InitWorld, WorldMessageHandler, TermWorld)

// Collider shared
static bool
InitCollider(struct BtCollider *c, const char **args)
{
	/*for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strnlen(arg, UINT16_MAX);

		if (!strncmp(arg, "Radius", len))
			radius = (float)atof(*(++args));
	}*/

	c->object = new btCollisionObject();
	c->object->setUserPointer(c->_owner);
	c->object->setCollisionShape(c->shape);

	return true;
}

static void
TermCollider(struct BtCollider *c)
{
	delete c->object;
	delete c->shape;
}

// Box collider
static bool
InitBox(struct BtCollider *c, const char **args)
{
	btVector3 hExtents { -1.f, -1.f, -1.f };

	for (; args && *args; ++args) {
		const char *arg = *args;
		const size_t len = strlen(arg);

		if (!strncmp(arg, "HalfExtents", len)) {
			char *ptr = (char *)*(++args);
			hExtents.setX(strtof(ptr, &ptr));
			hExtents.setY(strtof(ptr + 2, &ptr));
			hExtents.setZ(strtof(ptr + 2, &ptr));
		}
	}

	if (hExtents.x() < 0.f || hExtents.y() < 0.f || hExtents.z() < 0.f) {
		const struct NeModelRender *mr = (const struct NeModelRender *)E_GetComponent(c->_owner, NE_MODEL_RENDER_ID);
		if (mr) {
			hExtents.setX((mr->bounds.aabb.max.x - mr->bounds.aabb.min.x) / 2.f);
			hExtents.setY((mr->bounds.aabb.max.y - mr->bounds.aabb.min.y) / 2.f);
			hExtents.setZ((mr->bounds.aabb.max.z - mr->bounds.aabb.min.z) / 2.f);
		} else {
			hExtents = btVector3(1.f, 1.f, 1.f);
		}
	}

	c->shape = new btBoxShape(hExtents);
	if (!c->shape)
		return false;

	return InitCollider(c, args);
}
NE_REGISTER_COMPONENT(NE_BOX_COLLIDER, struct BtCollider, 16, InitBox, nullptr, TermCollider)

void
BT_GetBoxHalfExtents(BtCollider *collider, struct NeVec3 *halfExtents)
{
	const btVector3 &he = ((btBoxShape *)collider->shape)->getHalfExtentsWithoutMargin();
	halfExtents-> x = he.x(); halfExtents->y = he.y(); halfExtents->z = he.z();
}

void
BT_SetBoxHalfExtents(BtCollider *collider, const struct NeVec3 *halfExtents)
{
	delete collider->shape;
	collider->shape = new btBoxShape({ halfExtents->x, halfExtents->y, halfExtents->z });
	collider->object->setCollisionShape(collider->shape);
}

// Sphere collider
static bool
InitSphere(struct BtCollider *c, const char **args)
{
	float radius = -1.f;

	const char **argPtr = args;
	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strnlen(arg, UINT16_MAX);

		if (!strncmp(arg, "Radius", len))
			radius = (float)atof(*(++args));
	}

	if (radius < 0.f) {
		const struct NeModelRender *mr = (const struct NeModelRender *)E_GetComponent(c->_owner, NE_MODEL_RENDER_ID);
		radius = mr ? mr->bounds.sphere.radius : 1.f;
	}

	c->shape = new btSphereShape(radius);
	if (!c->shape)
		return false;

	return InitCollider(c, argPtr);
}
NE_REGISTER_COMPONENT(NE_SPHERE_COLLIDER, struct BtCollider, 16, InitSphere, nullptr, TermCollider)

float
BT_GetSphereRadius(BtCollider *collider)
{
	return ((btSphereShape *)collider->shape)->getRadius();
}

void
BT_SetSphereRadius(BtCollider *collider, float radius)
{
	return ((btSphereShape *)collider->shape)->setUnscaledRadius(radius);
}

// Sphere collider
static bool
InitCapsule(struct BtCollider *c, const char **args)
{
	float radius = -1.f, height = -1.f;

	const char **argPtr = args;
	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strnlen(arg, UINT16_MAX);

		if (!strncmp(arg, "Radius", len))
			radius = (float)atof(*(++args));
		else if (!strncmp(arg, "Height", len))
			height = (float)atof(*(++args));
	}

	if (radius < 0.f || height < 0.f) {
		const struct NeModelRender *mr = (const struct NeModelRender *)E_GetComponent(c->_owner, NE_MODEL_RENDER_ID);
		if (mr) {
			radius = M_Max(mr->bounds.aabb.max.x - mr->bounds.aabb.min.x, mr->bounds.aabb.max.z - mr->bounds.aabb.min.z);
			height = mr->bounds.aabb.max.y - mr->bounds.aabb.min.y;
		} else {
			radius = 1.f;
			height = 1.f;
		}
	}

	c->shape = new btCapsuleShape(radius, height);
	if (!c->shape)
		return false;

	return InitCollider(c, argPtr);
}
NE_REGISTER_COMPONENT(NE_CAPSULE_COLLIDER, struct BtCollider, 16, InitCapsule, nullptr, TermCollider)

void
BT_GetCapsuleDimensions(BtCollider *collider, float *radius, float *height)
{
	if (radius)
		*radius = ((btCapsuleShape *)collider->shape)->getRadius();

	if (height)
		*height = ((btCapsuleShape *)collider->shape)->getHalfHeight() * 2.f;
}

void
BT_SetCapsuleDimensions(BtCollider *collider, float radius, float height)
{
	delete collider->shape;
	collider->shape = new btCapsuleShape(radius, height);
	collider->object->setCollisionShape(collider->shape);
}

// Mesh collider
static bool
InitMesh(struct BtMeshCollider *c, const char **args)
{
	struct NeModelRender *mr = (struct NeModelRender *)E_GetComponent(c->_owner, NE_MODEL_RENDER_ID);
	if (!mr)
		return false;

	struct NeModel *mdl = (struct NeModel *)E_ResourcePtr(mr->model);
	if (!mdl || !mdl->cpu.vertices || !mdl->cpu.indices)
		return false;

	btIndexedMesh indexedMesh{};
	indexedMesh.m_numTriangles = mdl->vertexCount / 3;
	indexedMesh.m_numVertices = mdl->vertexCount;
	indexedMesh.m_triangleIndexStride = sizeof(uint32_t) * 3;
	indexedMesh.m_vertexStride = sizeof(struct NeVertex);
	indexedMesh.m_vertexType = PHY_FLOAT;
	indexedMesh.m_triangleIndexBase = (const unsigned char *)mdl->cpu.indices;
	indexedMesh.m_vertexBase = (const unsigned char *)mdl->cpu.vertices;

	c->tivArray = new btTriangleIndexVertexArray();
	if (!c->tivArray)
		return false;

	c->tivArray->addIndexedMesh(indexedMesh);

	c->shape = new btConvexTriangleMeshShape(c->tivArray, true);
	if (!c->shape) {
		delete c->tivArray;
		return false;
	}

	return InitCollider(c, args);
}

static void
TermMesh(struct BtMeshCollider *c)
{
	TermCollider(c);
	delete c->tivArray;
}
NE_REGISTER_COMPONENT(NE_MESH_COLLIDER, struct BtMeshCollider, 16, InitMesh, nullptr, TermMesh)

void
BT_UpdateMesh(BtMeshCollider *collider)
{
	delete collider->tivArray;
	delete collider->shape;

	struct NeModelRender *mr = (struct NeModelRender *)E_GetComponent(collider->_owner, NE_MODEL_RENDER_ID);
	if (!mr)
		return;

	struct NeModel *mdl = (struct NeModel *)E_ResourcePtr(mr->model);
	if (!mdl || !mdl->cpu.vertices || !mdl->cpu.indices)
		return;

	btIndexedMesh indexedMesh{};
	indexedMesh.m_numTriangles = mdl->vertexCount / 3;
	indexedMesh.m_numVertices = mdl->vertexCount;
	indexedMesh.m_triangleIndexStride = sizeof(uint32_t) * 3;
	indexedMesh.m_vertexStride = sizeof(struct NeVertex);
	indexedMesh.m_vertexType = PHY_FLOAT;
	indexedMesh.m_triangleIndexBase = (const unsigned char *)mdl->cpu.indices;
	indexedMesh.m_vertexBase = (const unsigned char *)mdl->cpu.vertices;

	collider->tivArray = new btTriangleIndexVertexArray();
	if (!collider->tivArray)
		return;

	collider->tivArray->addIndexedMesh(indexedMesh);

	collider->shape = new btConvexTriangleMeshShape(collider->tivArray, true);
	if (!collider->shape) {
		delete collider->tivArray;
		return;
	}

	collider->object->setCollisionShape(collider->shape);
}
