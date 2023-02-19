#include <stdlib.h>

#ifdef __APPLE__
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#	include <AL/al.h>
#	include <AL/alc.h>
#endif

#include <Audio/Clip.h>
#include <Engine/Asset.h>
#include <Engine/Types.h>
#include <System/Memory.h>
#include <Engine/Resource.h>

static inline bool _InitClip(struct NeAudioClip *ac);

bool
Au_CreateClip(const char *name, const struct NeAudioClipCreateInfo *ci, struct NeAudioClip *ac, NeHandle h)
{
	ac->data = ci->samples;
	ac->byteSize = ci->sampleCount * 2;

	return _InitClip(ac);
}

bool
Au_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAudioClip *ac, NeHandle h)
{
	if (!E_LoadWaveAsset(&li->stm, ac))
		return false;

	return _InitClip(ac);
}

void
Au_UnloadClip(struct NeAudioClip *ac, NeHandle h)
{
	alDeleteBuffers(1, &ac->handle);
	Sys_Free((void *)ac->data);
}

static inline bool
_InitClip(struct NeAudioClip *ac)
{
	ALenum format;

	alGenBuffers(1, &ac->handle);
	if (alGetError() != AL_NO_ERROR)
		return false;

	if (ac->bitsPerSample == 8)
		format = ac->channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
	else if (ac->bitsPerSample == 16)
		format = ac->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	else
		goto error;

	alBufferData(ac->handle, format, ac->data, ac->byteSize, ac->sampleRate);
	if (alGetError() != AL_NO_ERROR)
		goto error;

	return true;

error:
	alDeleteBuffers(1, &ac->handle);
	return false;
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif

/* NekoEngine
 *
 * AudioClip.c
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
