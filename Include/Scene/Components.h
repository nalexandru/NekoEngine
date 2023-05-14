#ifndef NE_SCENE_COMPONENTS_H
#define NE_SCENE_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Engine/Types.h>

#define NE_CAMERA			"Camera"
#define NE_TRANSFORM		"Transform"
#define NE_MODEL_RENDER		"ModelRender"
#define NE_AUDIO_SOURCE		"AudioSource"
#define NE_AUDIO_LISTENER	"AudioListener"
#define NE_UI_CONTEXT		"UIContext"
#define NE_ANIMATOR			"Animator"
#define NE_LIGHT			"Light"
#define NE_PROFILER_DRAW	"ProfilerDraw"
#define NE_MODEL_MORPH		"ModelMorph"

extern NeCompTypeId NE_CAMERA_ID;
extern NeCompTypeId NE_TRANSFORM_ID;
extern NeCompTypeId NE_MODEL_RENDER_ID;
extern NeCompTypeId NE_AUDIO_SOURCE_ID;
extern NeCompTypeId NE_AUDIO_LISTENER_ID;
extern NeCompTypeId NE_UI_CONTEXT_ID;
extern NeCompTypeId NE_ANIMATOR_ID;
extern NeCompTypeId NE_LIGHT_ID;
extern NeCompTypeId NE_PROFILER_DRAW_ID;
extern NeCompTypeId NE_MODEL_MORPH_ID;

#ifdef __cplusplus
}
#endif

#endif /* NE_SCENE_COMPONENTS_H */

/* NekoEngine
 *
 * Components.h
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
