#include <stdio.h>
#include <assert.h>

#include <Engine/IO.h>
#include <Asset/NAnim.h>
#include <Editor/Asset/Asset.h>

void
Asset_SaveNAnim(const struct NAnim *na, const char *path)
{
	ASSET_WRITE_INIT();

	FILE *fp = fopen(path, "wb");
	assert(fp);

	ASSET_WRITE_GUARD(NANIM_1_HEADER);
	
	ASSET_WRITE_SEC(NANIM_INFO_ID, sizeof(na->info));
	fwrite(&na->info, sizeof(na->info), 1, fp);
	ASSET_WRITE_GUARD(NANIM_SEC_FOOTER);
	
	ASSET_WRITE_SEC(NANIM_CHN_ID, na->channelCount);

	for (uint32_t i = 0; i < na->channelCount; ++i) {
		const struct NAnimChannel *ch = &na->channels[i];
		fwrite(ch->name, sizeof(ch->name), 1, fp);

		fwrite(&ch->positionCount, sizeof(ch->positionCount), 1, fp);
		fwrite(&ch->rotationCount, sizeof(ch->rotationCount), 1, fp);
		fwrite(&ch->scalingCount, sizeof(ch->scalingCount), 1, fp);

		fwrite(ch->positionKeys, sizeof(*ch->positionKeys), ch->positionCount, fp);
		fwrite(ch->rotationKeys, sizeof(*ch->rotationKeys), ch->rotationCount, fp);
		fwrite(ch->scalingKeys, sizeof(*ch->scalingKeys), ch->scalingCount, fp);
	}

	ASSET_WRITE_GUARD(NANIM_SEC_FOOTER);
	ASSET_WRITE_GUARD(NANIM_FOOTER);

	fclose(fp);
}

/* NekoEditor
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
