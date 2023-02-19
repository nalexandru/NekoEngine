#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Animation/Animation.h>

bool
Anim_CreateClip(const char *name, const struct NeAnimationClipCreateInfo *ci, struct NeAnimationClip *ac, NeHandle h)
{
	memcpy(ac->name, ci->name, sizeof(ac->name));

	ac->ticks = ci->ticks;
	ac->duration = ci->duration;

	if (!Rt_InitArray(&ac->channels, ci->channelCount, sizeof(struct NeAnimationChannel), MH_Asset))
		return false;

	Rt_FillArray(&ac->channels);

	for (uint32_t i = 0; i < ci->channelCount; ++i) {
		struct NeAnimationChannel *ch = Rt_ArrayGet(&ac->channels, i);

		ch->hash = RT_HASH(ci->channels[i].name);
		memcpy(ch->name, ci->channels[i].name, sizeof(ch->name));

		if (!Rt_InitArray(&ch->positionKeys, ci->channels[i].positionCount, sizeof(struct NeAnimVectorKey), MH_Asset))
			goto error;
		Rt_FillArray(&ch->positionKeys);
		memcpy(ch->positionKeys.data, ci->channels[i].positionKeys, Rt_ArrayByteSize(&ch->positionKeys));

		if (!Rt_InitArray(&ch->rotationKeys, ci->channels[i].rotationCount, sizeof(struct NeAnimQuatKey), MH_Asset))
			goto error;
		Rt_FillArray(&ch->rotationKeys);
		memcpy(ch->rotationKeys.data, ci->channels[i].rotationKeys, Rt_ArrayByteSize(&ch->rotationKeys));

		if (!Rt_InitArray(&ch->scalingKeys, ci->channels[i].scalingCount, sizeof(struct NeAnimVectorKey), MH_Asset))
			goto error;
		Rt_FillArray(&ch->scalingKeys);
		memcpy(ch->scalingKeys.data, ci->channels[i].scalingKeys, Rt_ArrayByteSize(&ch->scalingKeys));
	}

	return true;

error:
	for (uint32_t i = 0; i < ci->channelCount; ++i) {
		struct NeAnimationChannel *ch = Rt_ArrayGet(&ac->channels, i);

		if (!ch->hash)
			continue;

		if (ch->positionKeys.data)
			Rt_TermArray(&ch->positionKeys);
		if (ch->rotationKeys.data)
			Rt_TermArray(&ch->rotationKeys);
		if (ch->scalingKeys.data)
			Rt_TermArray(&ch->scalingKeys);
	}

	Rt_TermArray(&ac->channels);
	
	return false;
}

bool
Anim_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAnimationClip *ac, NeHandle h)
{
	return E_LoadNAnimAsset(&li->stm, ac);
}

void
Anim_UnloadClip(struct NeAnimationClip *ac, NeHandle h)
{
	struct NeAnimationChannel *ch;
	Rt_ArrayForEach(ch, &ac->channels) {
		Rt_TermArray(&ch->positionKeys);
		Rt_TermArray(&ch->rotationKeys);
		Rt_TermArray(&ch->scalingKeys);
	}
	Rt_TermArray(&ac->channels);
}

/* NekoEngine
 *
 * AnimationClip.c
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
