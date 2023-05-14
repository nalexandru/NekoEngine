#include <stdio.h>

#include <Math/Math.h>
#include <Asset/NMesh.h>
#include <Asset/NMorph.h>
#include <Editor/Asset/Asset.h>

void
EdAsset_CalculateDeltas(const struct NeVertex *src, const struct NeVertex *dst, uint32_t count, struct NeArray *deltas)
{
	for (uint32_t i = 0; i < count; ++i) {
		if (XMVector3Equal(XMLoadFloat3((XMFLOAT3 *)&src[i]), XMLoadFloat3((XMFLOAT3 *)&dst[i])))
			continue;

		struct NeMorphDelta delta =
		{
			.vertex = i,
			.x = dst[i].x, .y = dst[i].y, .z = dst[i].z,
			.nx = dst[i].nx, .ny = dst[i].ny, .nz = dst[i].nz,
			.tx = dst[i].tx, .ty = dst[i].ty, .tz = dst[i].tz
		};
		Rt_ArrayAdd(deltas, &delta);
	}
}

bool
EdAsset_SaveNMorph(const struct NeMorph *nm, const struct NeMorphDelta *deltas, const char *path)
{
	bool rc = false;
	ASSET_WRITE_INIT();

	FILE *fp = fopen(path, "wb");
	if (!fp)
		return false;

	ASSET_WRITE_GUARD(NMORPH_1_HEADER);

	ASSET_WRITE_SEC(NMESH_MORPH_INFO_ID, sizeof(*nm));
	if (fwrite(nm, sizeof(*nm), 1, fp) != 1)
		goto exit;
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_SEC(NMESH_MORPH_DELTA_ID, sizeof(*deltas) * nm->deltaCount);
	if (fwrite(deltas, sizeof(*deltas), nm->deltaCount, fp) != nm->deltaCount)
		goto exit;
	ASSET_WRITE_GUARD(NMESH_SEC_FOOTER);

	ASSET_WRITE_GUARD(NMORPH_FOOTER);

	rc = true;
exit:
	fclose(fp);
	return rc;
}

/* NekoEditor
 *
 * NMorph.c
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
