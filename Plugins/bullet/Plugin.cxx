#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Plugin.h>
#include <Script/Script.h>
#include <Script/Interface.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Engine/Component.h>

#include "Plugin.h"

#ifdef _WIN32
#	define EXPORT __declspec(dllexport) extern "C"
#else
#	define EXPORT extern "C"
#endif

static bool RayCast(struct NeScene *scn, struct NeVec3 *start, struct NeVec3 *end, struct NeHitInfo *hi);

struct NePhysics f_interface =
{
	RayCast,
	(void(*)(void *, struct NeVec3 *))BT_GetBoxHalfExtents,
	(void(*)(void *, const struct NeVec3 *))BT_SetBoxHalfExtents,
	(float(*)(void *))BT_GetSphereRadius,
	(void(*)(void *, float))BT_SetSphereRadius,
	(void(*)(void *, float *, float *))BT_GetCapsuleDimensions,
	(void(*)(void *, float, float))BT_SetCapsuleDimensions,
	(void(*)(void *))BT_UpdateMesh
};

EXPORT struct NePlugin PluginInfo =
{
	.identifier = NE_PLUGIN_ID,
	.apiVersion = NE_PLUGIN_API,
	.name = "NekoEngine Bullet Physics",
	.copyright = "(c) 2023 Alexandru Naiman",
	.version = { 0, 1, 0, 2 },
	.loadOrder = NEP_LOAD_PRE_ECS
};

EXPORT struct NePluginInterface PluginInterfaces[] =
{
		{ "NePhysics", &f_interface },
		{ NULL, NULL }
};

btCollisionConfiguration *BT_configuration = nullptr;

EXPORT bool
InitPlugin(void)
{
	if (!(BT_configuration = new btDefaultCollisionConfiguration()))
		return false;

	Sys_LogEntry(BT_PLUGIN, LOG_INFORMATION, "Bullet version %d", BT_BULLET_VERSION);

	return true;
}

EXPORT void
TermPlugin(void)
{

}

static bool
RayCast(struct NeScene *scn, struct NeVec3 *start, struct NeVec3 *end, struct NeHitInfo *hi)
{
	const struct NeArray *entities = E_GetAllComponentsS(scn, NE_PHYSICS_WORLD_ID);
	if (!entities->count)
		return false;

	const btVector3 s{ start->x, start->y, start->z }, e{ end->x, end->y, end->z };
	btCollisionWorld::ClosestRayResultCallback cb(s, e);

	const btCollisionWorld *world = (btCollisionWorld *)Rt_ArrayGet(entities, 0);
	world->rayTest(s, e, cb);
	if (!cb.hasHit())
		return false;

	hi->position.x = cb.m_hitPointWorld.x();
	hi->position.y = cb.m_hitPointWorld.x();
	hi->position.z = cb.m_hitPointWorld.x();
	hi->entity = cb.m_collisionObject->getUserPointer();

	return true;
}

/* NekoEngine Bullet Plugin
 *
 * plugin.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
