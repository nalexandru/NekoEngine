/* NekoEngine
 *
 * components.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Test Application
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#ifndef _TEST_APP_COMPONENTS_H_
#define _TEST_APP_COMPONENTS_H_

#include <ecs/component.h>

struct fps_component
{
	NE_COMPONENT;

	int frames;
	double time;
};

struct dbg_component
{
	NE_COMPONENT;

	bool show_camera_pos;
};

struct movement_component
{
	NE_COMPONENT;

	float pos[3];
	bool positive;
	struct ne_light *light;
};

#define TAPP_FPS_COMPONENT_TYPE		"tapp_fps_component"
#define TAPP_DBG_COMPONENT_TYPE		"tapp_dbg_component"
#define TAPP_MOVEMENT_COMPONENT_TYPE	"tapp_movement_component"

NE_REGISTER_COMPONENT(TAPP_FPS_COMPONENT_TYPE, struct fps_component, 0, 0)
NE_REGISTER_COMPONENT(TAPP_DBG_COMPONENT_TYPE, struct dbg_component, 0, 0)
NE_REGISTER_COMPONENT(TAPP_MOVEMENT_COMPONENT_TYPE, struct movement_component, 0, 0)

#endif /* _TEST_APP_COMPONENTS_H_ */

