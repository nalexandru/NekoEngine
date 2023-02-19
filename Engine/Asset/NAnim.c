#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Animation/Clip.h>
#include <Asset/NAnim.h>
#include <System/Log.h>

#define NANIM_MOD	"NAnim"

// all values are little-endian
#define NANIM_1_HEADER			0x0000314D494E414Ellu	// NANIM1
#define NANIM_FOOTER			0x004D494E41444E45llu	// ENDANIM
#define NANIM_SEC_FOOTER		0x0054434553444E45llu	// ENDSECT

#define NANIM_CHN_ID			0x004E4843u				// CHN

#define NANIM_END_ID			0x41444E45u				// ENDA

bool
E_LoadNAnimAsset(struct NeStream *stm, struct NeAnimationClip *ac)
{
	ASSET_INFO;

	ASSET_CHECK_GUARD(NANIM_1_HEADER);

	while (!E_EndOfStream(stm)) {
		ASSET_READ_ID();

		if (a.id == NANIM_CHN_ID) {
			struct NeAnimationChannel ch;
			Rt_InitArray(&ac->channels, a.size, sizeof(ch), MH_Asset);
			
			for (uint32_t i = 0; i < a.size; ++i) {
				struct NAnimChannelInfo ci;
				E_ReadStream(stm, &ci, sizeof(ci));
				
				memset(ch.name, 0x0, sizeof(ch.name));
				memcpy(ch.name, ci.name, sizeof(ch.name));
				ch.hash = Rt_HashString(ch.name);

				struct NAnimVectorKey *positionKeys = Sys_Alloc(sizeof(*positionKeys), ci.positionCount, MH_Asset);
				size_t positionSize = ci.positionCount * sizeof(*positionKeys);
				
				struct NAnimQuatKey *rotationKeys = Sys_Alloc(sizeof(*rotationKeys), ci.rotationCount, MH_Asset);
				size_t rotationSize = ci.rotationCount * sizeof(*rotationKeys);
				
				struct NAnimVectorKey *scalingKeys = Sys_Alloc(sizeof(*scalingKeys), ci.scalingCount, MH_Asset);
				size_t scalingSize = ci.scalingCount * sizeof(*scalingKeys);
				
				Rt_InitArray(&ch.positionKeys, ci.positionCount, sizeof(struct NeAnimVectorKey), MH_Asset);
				Rt_FillArray(&ch.positionKeys);
				
				Rt_InitArray(&ch.rotationKeys, ci.rotationCount, sizeof(struct NeAnimQuatKey), MH_Asset);
				Rt_FillArray(&ch.rotationKeys);
				
				Rt_InitArray(&ch.scalingKeys, ci.scalingCount, sizeof(struct NeAnimVectorKey), MH_Asset);
				Rt_FillArray(&ch.scalingKeys);
				
				E_ReadStream(stm, positionKeys, positionSize);
				E_ReadStream(stm, rotationKeys, rotationSize);
				E_ReadStream(stm, scalingKeys, scalingSize);
				
				for (uint32_t j = 0; j < ci.positionCount; ++j) {
			//		M_Vec3(&((struct NeAnimVectorKey *)ch.positionKeys.data)[j].val,
			//			   positionKeys[j].val[0], positionKeys[j].val[1], positionKeys[j].val[2]);
					((struct NeAnimVectorKey *)ch.positionKeys.data)[j].time = positionKeys[j].time;
				}
				
				for (uint32_t j = 0; j < ci.rotationCount; ++j) {
			//		M_Quat(&((struct NeAnimQuatKey *)ch.rotationKeys.data)[j].val,
			//			   rotationKeys[j].quat[0], rotationKeys[j].quat[1], rotationKeys[j].quat[2], rotationKeys[j].quat[3]);
					((struct NeAnimQuatKey *)ch.rotationKeys.data)[j].time = rotationKeys[j].time;
				}
				
				for (uint32_t j = 0; j < ci.scalingCount; ++j) {
			//		M_Vec3(&((struct NeAnimVectorKey *)ch.scalingKeys.data)[j].val,
			//			   scalingKeys[j].val[0], scalingKeys[j].val[1], scalingKeys[j].val[2]);
					((struct NeAnimVectorKey *)ch.scalingKeys.data)[j].time = scalingKeys[j].time;
				}
				
				Rt_ArrayAdd(&ac->channels, &ch);

				Sys_Free(positionKeys);
				Sys_Free(rotationKeys);
				Sys_Free(scalingKeys);
			}
		} else if (a.id == NANIM_INFO_ID) {
			struct NAnimInfo info;
			
			if (a.size != sizeof(info))
				goto error;
			
			E_ReadStream(stm, &info, sizeof(info));
			
			snprintf(ac->name, sizeof(ac->name), "%s", info.name);
			ac->duration = info.duration;
			ac->ticks = info.ticks;
		} else if (a.id == NANIM_END_ID) {
			E_StreamSeek(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
			Sys_LogEntry(NANIM_MOD, LOG_WARNING, "Unknown section id = 0x%x, size = %d", a.id, a.size);
			E_StreamSeek(stm, a.size, IO_SEEK_CUR);
		}

		ASSET_CHECK_GUARD(NANIM_SEC_FOOTER);
	}

	ASSET_CHECK_GUARD(NANIM_FOOTER);

	return true;
	
error:
	
	for (size_t i = 0; i < ac->channels.count; ++i) {
		struct NeAnimationChannel *ch = Rt_ArrayGet(&ac->channels, i);

		Rt_TermArray(&ch->positionKeys);
		Rt_TermArray(&ch->rotationKeys);
		Rt_TermArray(&ch->scalingKeys);
	}

	Rt_TermArray(&ac->channels);

	return false;
}

/* NekoEngine
 *
 * NAnim.c
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
