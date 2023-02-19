#include <Scene/Scene.h>
#include <Scene/Systems.h>
#include <Engine/Entity.h>
#include <Engine/ECSystem.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>

static bool _InitTransform(struct NeTransform *xform, const char **args);
static void _TermTransform(struct NeTransform *xform);

E_REGISTER_COMPONENT(TRANSFORM_COMP, struct NeTransform, 16, _InitTransform, _TermTransform)

static bool
_InitTransform(struct NeTransform *xform, const char **args)
{
	xform->position.x = xform->position.y = xform->position.z = 0.f;
	xform->scale.x = xform->scale.y = xform->scale.z = 1.f;
	xform->forward = { 0.f, 0.f, -1.f };
	xform->right = { 1.f, 0.f, 0.f };
	xform->up = { 0.f, 1.f, 0.f };

	M_Store(&xform->rotation, XMQuaternionIdentity());
	M_Store(&xform->mat, XMMatrixIdentity());

	xform->parent = E_INVALID_HANDLE;
	xform->dirty = true;

	for (; args && *args; ++args) {
		const char *arg = *args;
		const size_t len = strlen(arg);

		if (!strncmp(arg, "Parent", len)) {
			struct NeScene *s = Scn_GetScene((uint8_t)xform->_sceneId);

			NeEntityHandle parent = E_FindEntityS(s, (char *)*(++args));
			xform->parent = E_GetComponentHandleS(s, parent, E_ComponentTypeId(TRANSFORM_COMP));

			struct NeTransform *parentPtr = (struct NeTransform *)E_ComponentPtrS(s, xform->parent);
			const NeCompHandle self = xform->_handleId;
			Rt_ArrayAdd(&parentPtr->children, &self);
		} else if (!strncmp(arg, "Position", len)) {
			char *ptr = (char *)*(++args);
			xform->position.x = strtof(ptr, &ptr);
			xform->position.y = strtof(ptr + 2, &ptr);
			xform->position.z = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "Rotation", len)) {
			struct NeVec3 r;
			char *ptr = (char *)*(++args);
			r.x = strtof(ptr, &ptr);
			r.y = strtof(ptr + 2, &ptr);
			r.z = strtof(ptr + 2, &ptr);
			M_Store(&xform->rotation, XMQuaternionRotationRollPitchYaw(r.x, r.y, r.z));
		} else if (!strncmp(arg, "Scale", len)) {
			char *ptr = (char *)*(++args);
			xform->scale.x = strtof(ptr, &ptr);
			xform->scale.y = strtof(ptr + 2, &ptr);
			xform->scale.z = strtof(ptr + 2, &ptr);
		}
	}

	Rt_InitArray(&xform->children, 2, sizeof(NeCompHandle), MH_Scene);

	return true;
}

static void
_TermTransform(struct NeTransform *xform)
{
	Rt_TermArray(&xform->children);
}

E_SYSTEM(SCN_UPDATE_TRANSFORM, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_TRANSFORM, false, void, 1, TRANSFORM_COMP)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	
	if (xform->parent == E_INVALID_HANDLE)
		xform_update(xform);
}

/* NekoEngine
 *
 * Transform.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
