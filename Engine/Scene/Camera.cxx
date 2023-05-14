#include <Scene/Camera.h>
#include <Scene/Systems.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Engine/Engine.h>
#include <Engine/Event.h>
#include <Engine/Events.h>

static bool InitCamera(struct NeCamera *cam, const char **args);
static void TermCamera(struct NeCamera *cam);
static void RebuildProjection(struct NeCamera *cam, void *args);

NE_REGISTER_COMPONENT(NE_CAMERA, struct NeCamera, 16, InitCamera, nullptr, TermCamera)

static bool
InitCamera(struct NeCamera *cam, const char **args)
{
	XMStoreFloat4x4A((XMFLOAT4X4A *)&cam->viewMatrix, XMMatrixIdentity());

	cam->fov = 90.f;
	cam->zNear = .1f;
	cam->zFar = 1024.f;
	cam->aperture = 0.0016f;
	cam->projection = PT_Perspective;
	cam->activate = false;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strnlen(arg, UINT16_MAX);

		if (!strncmp(arg, "Active", len)) {
			cam->activate = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "Fov", len)) {
			cam->fov = (float)atof(*(++args));
		} else if (!strncmp(arg, "Near", len)) {
			cam->zNear = (float)atof(*(++args));
		} else if (!strncmp(arg, "Far", len)) {
			cam->zFar = (float)atof(*(++args));
		} else if (!strncmp(arg, "Aperture", len)) {
			cam->zFar = (float)atof(*(++args));
		} else if (!strncmp(arg, "Projection", len)) {
			const char *val = *(++args);
			const size_t vLen = strnlen(val, UINT16_MAX);

			if (!strncmp(val, "Perspective", vLen))
				cam->projection = PT_Perspective;
			else if (!strncmp(val, "Orthographic", vLen))
				cam->projection = PT_Orthographic;
		}
	}

	if (cam->projection == PT_Perspective)
		M_InfinitePerspectiveMatrixRZ(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);
	else if (cam->projection == PT_Orthographic)
		M_Store(&cam->projMatrix, XMMatrixOrthographicOffCenterRH(-100.f, (float)*E_screenWidth, (float)*E_screenHeight, 100.f, cam->zFar, cam->zNear));

	cam->evt = E_RegisterHandler(EVT_SCREEN_RESIZED, (NeEventHandlerProc)RebuildProjection, cam);

	return true;
}

NE_SYSTEM(SCN_UPDATE_CAMERA, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_CAM_VIEW, true, void, 2, NE_TRANSFORM, NE_CAMERA)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct NeCamera *cam = (struct NeCamera *)comp[1];

	struct NeVec3 pos;
	Xform_Position(xform, &pos);

	M_Store(&cam->viewMatrix, XMMatrixLookAtRH(M_Load(&pos), XMVectorAdd(M_Load(&pos), M_Load(&xform->forward)), M_Load(&xform->up)));
}

NE_SYSTEM(SCN_ACTIVATE_CAMERA, ECSYS_GROUP_MANUAL, 0, false, struct NeScene, 1, NE_CAMERA)
{
	struct NeCamera *cam = (struct NeCamera *)comp[0];
	if (!cam->activate)
		return;

	args->camera = E_ComponentHandle(cam);
}

static void
TermCamera(struct NeCamera *cam)
{
	E_UnregisterHandler(cam->evt);
}

static void
RebuildProjection(struct NeCamera *cam, void *args)
{
	if (cam->projection == PT_Perspective)
		M_InfinitePerspectiveMatrixRZ(&cam->projMatrix, cam->fov, (float)*E_screenWidth / (float)*E_screenHeight, cam->zNear);
	else if (cam->projection)
		M_Store(&cam->projMatrix, XMMatrixOrthographicOffCenterRH(-100.f, (float)*E_screenWidth, (float)*E_screenHeight, 100.f, cam->zFar, cam->zNear));

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
