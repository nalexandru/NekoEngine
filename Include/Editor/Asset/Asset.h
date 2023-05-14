#ifndef NE_EDITOR_ASSET_ASSET_H
#define NE_EDITOR_ASSET_ASSET_H

#include <Editor/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASSET_WRITE_INIT() 		\
	union {						\
		uint64_t guard;			\
		struct {				\
			uint32_t id;		\
			uint32_t size;		\
		};						\
	} a
#define ASSET_WRITE_SEC(i, v) a.id = i; a.size = v; if (fwrite(&a, sizeof(a), 1, fp) != 1) goto exit
#define ASSET_WRITE_GUARD(x) a.guard = x; if (fwrite(&a, sizeof(a), 1, fp) != 1) goto exit

void Ed_OpenAsset(const char *path);

void EdAsset_OptimizeNMesh(struct NMesh *nm);
bool EdAsset_LoadNMesh(struct NMesh *nm, const char *path);
bool EdAsset_SaveNMesh(const struct NMesh *nm, const char *path);
void EdAsset_FreeNMesh(struct NMesh *nm);

void EdAsset_CalculateDeltas(const struct NeVertex *src, const struct NeVertex *dst, uint32_t count, struct NeArray *deltas);
bool EdAsset_SaveNMorph(const struct NeMorph *nm, const struct NeMorphDelta *deltas, const char *path);

bool EdAsset_SaveNAnim(const struct NAnim *na, const char *path);

bool EdAsset_SavePNG(uint32_t w, uint32_t h, uint8_t *data, const char *path);
bool EdAsset_ConvertToPNG(const uint8_t *data, size_t size, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* NE_EDITOR_ASSET_ASSET_H */

/* NekoEngine
 *
 * Asset.h
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
