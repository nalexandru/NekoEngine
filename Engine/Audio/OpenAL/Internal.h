#ifndef NE_AUDIO_OPENAL_INTERNAL_H
#define NE_AUDIO_OPENAL_INTERNAL_H

#if defined(__APPLE__)
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(USE_CREATIVE_OPENAL)
#	include <al.h>
#	include <alc.h>
#	include <alext.h>
#else
#	include <AL/al.h>
#	include <AL/alc.h>
#	include <AL/alext.h>
#endif

/*
 * Apple removed these at some point but they are still used:
 * https://opensource.apple.com/source/OpenAL/OpenAL-48.7/Source/OpenAL/oalImp.h
 */
#ifdef __APPLE__
#	define AL_FORMAT_MONO_FLOAT32               0x10010
#	define AL_FORMAT_STEREO_FLOAT32             0x10011
#endif

struct NeAudioSource
{
	NE_COMPONENT_BASE;

	ALuint id;
	NeHandle clip;
	struct NeVec3 lastPosition;
};

#endif /* NE_AUDIO_OPENAL_INTERNAL_H */

/* NekoEngine
 *
 * Internal.h
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
