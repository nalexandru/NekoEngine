#include <Scene/Camera.h>
#include <Scene/Systems.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Engine/Engine.h>
#include <Engine/Event.h>
#include <Engine/Events.h>

struct NeCamera *Scn_activeCamera = NULL;

static bool _InitCamera(struct NeCamera *cam, const char **args);
static void _TermCamera(struct NeCamera *cam);
static void _RebuildProjection(struct NeCamera *cam, void *args);

E_REGISTER_COMPONENT(CAMERA_COMP, struct NeCamera, 16, _InitCamera, _TermCamera)

void
Scn_ActivateCamera(struct NeCamera *cam)
{
	Scn_activeCamera = cam;
}

static bool
_InitCamera(struct NeCamera *cam, const char **args)
{
	XMStoreFloat4x4A((XMFLOAT4X4A *)&cam->viewMatrix, XMMatrixIdentity());

	cam->fov = 90.f;
	cam->zNear = .1f;
	cam->zFar = 1024.f;
	cam->aperture = 0.0016f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Active", len)) {
			if (!strncmp(*(++args), "true", 4))
				Scn_activeCamera = cam;
		} else if (!strncmp(arg, "Fov", len)) {
			cam->fov = (float)atof(*(++args));
		} else if (!strncmp(arg, "Near", len)) {
			cam->zNear = (float)atof(*(++args));
		} else if (!strncmp(arg, "Far", len)) {
			cam->zFar = (float)atof(*(++args));
		} else if (!strncmp(arg, "Aperture", len)) {
			cam->zFar = (float)atof(*(++args));
		}
	}

	M_InfinitePerspectiveMatrixRZ(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);

	cam->evt = E_RegisterHandler(EVT_SCREEN_RESIZED, (NeEventHandlerProc)_RebuildProjection, cam);

	return true;
}

#include <System/Log.h>
E_SYSTEM(SCN_UPDATE_CAMERA, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_CAM_VIEW, true, void, 2, TRANSFORM_COMP, CAMERA_COMP)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct NeCamera *cam = (struct NeCamera *)comp[1];

	struct NeVec3 pos;
	xform_position(xform, &pos);

	M_Store(&cam->viewMatrix, XMMatrixLookAtRH(M_Load(&pos), XMVectorAdd(M_Load(&pos), M_Load(&xform->forward)), M_Load(&xform->up)));
}

static void
_TermCamera(struct NeCamera *cam)
{
	E_UnregisterHandler(cam->evt);
}

static void
_RebuildProjection(struct NeCamera *cam, void *args)
{
	M_InfinitePerspectiveMatrixRZ(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);
}

/* NekoEngine
 *
 * Camera.cxx
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
