#ifndef NE_EDITOR_ASSET_IMPORT_H
#define NE_EDITOR_ASSET_IMPORT_H

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeAssetImportOptions
{
	uint32_t inMemory : 1;
	uint32_t onlyGeometry : 1;
	void *dst;
};

struct NeAssetImportHandler
{
	char name[64];
	bool (*Match)(const char *path);
	bool (*Import)(const char *path, const struct NeAssetImportOptions *options);
};

void Asset_Import(const char *path, const struct NeAssetImportOptions *options, void (*completed)(void));
bool Asset_ImportInProgress(void);
bool Asset_RegisterImporter(const struct NeAssetImportHandler *ai);

bool Init_AssetImporter(void);
void Term_AssetImporter(void);

#define ED_ASSET_IMPORTER(id)																	\
static bool id ## _MatchAsset(const char *path);												\
static bool id ## _ImportAsset(const char *path, const struct NeAssetImportOptions *options);	\
struct NeAssetImportHandler EdImporter ## id =													\
{																								\
	.name = #id,																				\
	.Match = id ## _MatchAsset,																	\
	.Import = id ## _ImportAsset																\
};																								\
NE_INITIALIZER(_NeEdImporterRegister_ ## id) { Asset_RegisterImporter(&EdImporter ## id); }

#ifdef __cplusplus
}
#endif

#endif /* NE_EDITOR_ASSET_IMPORT_H */

/* NekoEngine
 *
 * Import.h
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
