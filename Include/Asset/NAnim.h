#ifndef NE_ASSET_NANIM_H
#define NE_ASSET_NANIM_H

#include <Render/Model.h>
#include <Runtime/Array.h>

#define NANIM_1_HEADER				0x0000314D494E414Ellu	// NANIM1
#define NANIM_FOOTER				0x004D494E41444E45llu	// ENDANIM
#define NANIM_SEC_FOOTER			0x0054434553444E45llu	// ENDSECT

#define NANIM_CHN_ID				0x004E4843u				// CHN
#define NANIM_INFO_ID				0x4F464E49u				// INFO

#define NANIM_END_ID				0x41444E45u				// ENDA

#define NANIM_INTERP_LINEAR			0
#define NANIM_INTERP_STEP			1
#define NANIM_INTERP_CUBIC_SPLINE	2

struct NAnimVectorKey
{
	float value[3];
	float time;
};

struct NAnimQuatKey
{
	float value[4];
	float time;
};

struct NAnimChannelInfo
{
	char name[256];
	uint32_t interpolation;
	uint32_t positionCount;
	uint32_t rotationCount;
	uint32_t scalingCount;
};

struct NAnimChannel
{
	struct NAnimChannelInfo info;

	struct NAnimVectorKey *positionKeys;
	struct NAnimQuatKey *rotationKeys;
	struct NAnimVectorKey *scalingKeys;
};

struct NAnimInfo
{
	char name[256];
	float duration;
};

struct NAnim
{
	struct NAnimInfo info;
	uint32_t channelCount;
	struct NAnimChannel *channels;
};

#endif /* NE_ASSET_NANIM_H */

/* NekoEngine
 *
 * NAnim.h
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
