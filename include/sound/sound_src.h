/* NekoEngine
 *
 * sound_clip.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Sound Subsystem
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

#ifndef _NE_SOUND_SRC_H_
#define _NE_SOUND_SRC_H_

#include <stdbool.h>

#include <engine/math.h>
#include <engine/status.h>
#include <ecs/component.h>
#include <sound/sound_clip.h>
#include <engine/components.h>

struct ne_sound_src;
typedef struct ne_sound_src ne_sound_src;

ne_sound_src	*snd_src_create(void);

void		 snd_src_pitch(ne_sound_src *src, float p);
void		 snd_src_gain(ne_sound_src *src, float p);
void		 snd_src_cone_inner_angle(ne_sound_src *src, float p);
void		 snd_src_cone_outer_angle(ne_sound_src *src, float p);
void		 snd_src_cone_outer_gain(ne_sound_src *src, float p);

void		 snd_src_direction(ne_sound_src *src, kmVec3 *v);
void		 snd_src_position(ne_sound_src *src, kmVec3 *v);
void		 snd_src_velocity(ne_sound_src *src, kmVec3 *v);

void		 snd_src_loop(ne_sound_src *src, bool loop);

void		 snd_src_max_distance(ne_sound_src *src, float d);
void		 snd_src_ref_distance(ne_sound_src *src, float d);

void		 snd_src_play(ne_sound_src *src);
void		 snd_src_pause(ne_sound_src *src);
void		 snd_src_stop(ne_sound_src *src);
void		 snd_src_rewind(ne_sound_src *src);
bool		 snd_src_playing(ne_sound_src *src);

ne_sound_clip	*snd_src_set_clip(ne_sound_src *src, ne_sound_clip *clip);

void		 snd_src_destroy(ne_sound_src *clip);

struct ne_sound_src_comp
{
	NE_COMPONENT;
	ne_sound_src *src;
};

#ifdef _NE_ENGINE_INTERNAL_

ne_status	snd_src_comp_create(void *src, const void **args);
void		snd_src_comp_destroy(void *src);

NE_REGISTER_COMPONENT(SOUND_SRC_COMP_TYPE, struct ne_sound_src_comp, snd_src_comp_create, snd_src_comp_destroy)

#endif /* _NE_ENGINE_INTERNAL */

#endif /* _NE_SOUND_SRC_H_ */

