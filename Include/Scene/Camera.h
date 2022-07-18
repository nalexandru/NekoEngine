#ifndef _NE_SCENE_CAMERA_H_
#define _NE_SCENE_CAMERA_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Engine/Component.h>

enum NeProjectionType
{
	PT_Perspective,
	PT_Orthographic
};

struct NeCamera
{
	NE_COMPONENT_BASE;

	struct NeMatrix viewMatrix;
	struct NeMatrix projMatrix;
	struct NeFrustum frustum;

	struct NeVec3 rotation;
	float fov, zNear, zFar, aperture;
	enum NeProjectionType projection;
	
	uint64_t evt;
};

ENGINE_API extern struct NeCamera *Scn_activeCamera;

void Scn_ActivateCamera(struct NeCamera *cam);

#endif /* _NE_SCENE_CAMERA_H_ */

/* NekoEngine
 *
 * frustum.h
 * Author: Alexandru Naiman
 *
 * Frustum functions
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
