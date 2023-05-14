#ifndef NE_ENGINE_EVENTS_H
#define NE_ENGINE_EVENTS_H

#include <Engine/Event.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVT_SCENE_LOAD_STARTED			"BeginSceneLoad"
#define EVT_SCENE_LOADED				"SceneLoaded"
#define EVT_SCENE_ACTIVATED				"SceneActivated"
#define EVT_SCREEN_RESIZED				"ScreenResized"
#define EVT_COMPONENT_REGISTERED			"ComponentRegistered"
#define EVT_COMPONENT_FIELDS_REGISTERED	"ComponentFieldsRegistered"

#define EVT_ENTITY_CREATED				"EntityCreated"
#define EVT_ENTITY_DESTROYED			"EntityDestroyed"

#define EVT_COMPONENT_CREATED			"ComponentCreated"
#define EVT_COMPONENT_DESTROYED			"ComponentDestroyed"

#ifdef __cplusplus
}
#endif

#endif /* NE_ENGINE_EVENTS_H */

/* NekoEngine
 *
 * Events.h
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
