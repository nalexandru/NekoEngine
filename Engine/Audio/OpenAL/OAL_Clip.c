#include <Audio/Clip.h>
#include <Engine/Asset.h>
#include <System/Memory.h>
#include <Engine/Resource.h>

#include "Internal.h"

static inline bool InitClip(struct NeAudioClip *ac);

bool
Au_CreateClip(const char *name, const struct NeAudioClipCreateInfo *ci, struct NeAudioClip *ac, NeHandle h)
{
	ac->data = ci->samples;
	ac->byteSize = ci->sampleCount * 2;

	return InitClip(ac);
}

bool
Au_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAudioClip *ac, NeHandle h)
{
	uint8_t hdr[4];

	E_ReadStream(&li->stm, hdr, 4);
	E_SeekStream(&li->stm, -4, IO_SEEK_CUR);

	bool rc = false;
	if (hdr[0] == 'O' && hdr[1] == 'g' && hdr[2] == 'g' && hdr[3] == 'S')
		rc = Asset_LoadOGG(&li->stm, ac);
	else if (hdr[0] == 'f' && hdr[1] == 'L' && hdr[2] == 'a' && hdr[3] == 'C')
		rc = Asset_LoadFLAC(&li->stm, ac);
	else if (hdr[0] == 'R' && hdr[1] == 'I' && hdr[2] == 'F' && hdr[3] == 'F')
		rc = Asset_LoadWAV(&li->stm, ac);

	if (!rc)
		return false;

	return InitClip(ac);
}

void
Au_UnloadClip(struct NeAudioClip *ac, NeHandle h)
{
	alDeleteBuffers(1, &ac->handle);
	Sys_Free((void *)ac->data);
}

static inline bool
InitClip(struct NeAudioClip *ac)
{
	alGenBuffers(1, &ac->handle);
	if (alGetError() != AL_NO_ERROR)
		return false;

	ALenum format;
	if (ac->bitsPerSample == 8) {
		format = ac->channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
	} else if (ac->bitsPerSample == 16) {
		format = ac->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	} else if (ac->bitsPerSample == 24) {
		format = ac->channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;

		// promote 24 bit to 32 bit
		float *dst = (float *)ac->data;
		const int32_t *src = (int32_t *)ac->data;
		for (uint32_t i = 0; i < ac->byteSize / 4; ++i)
			dst[i] = (float)src[i] / (float)((1 << 23) - 1);

	} else if (ac->bitsPerSample == 32) {
		format = ac->channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
#ifdef AL_FORMAT_MONO_DOUBLE_EXT
	} else if (ac->bitsPerSample == 48) {
		format = ac->channels == 1 ? AL_FORMAT_MONO_DOUBLE_EXT : AL_FORMAT_STEREO_DOUBLE_EXT;

		// promote 48 bit to 64 bit
		double *dst = (double *)ac->data;
		const int64_t *src = (int64_t *)ac->data;
		for (uint32_t i = 0; i < ac->byteSize / 4; ++i)
			dst[i] = (double)src[i] / (double)((1ll << 47) - 1);

	} else if (ac->bitsPerSample == 64) {
		format = ac->channels == 1 ? AL_FORMAT_MONO_DOUBLE_EXT : AL_FORMAT_STEREO_DOUBLE_EXT;
#endif
	} else {
		goto error;
	}

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
 * OAL_Clip.c
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
