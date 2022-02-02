#ifndef _NE_EDITOR_ASSET_IMPORT_H_
#define _NE_EDITOR_ASSET_IMPORT_H_

#include <Engine/Types.h>

struct NeAssetImportHandler
{
	char name[64];
	bool (*Match)(const char *path);
	bool (*Import)(const char *path);
};

void Asset_Import(const char *path);
bool Asset_ImportInProgress(void);
void Asset_RegisterImporter(const struct NeAssetImportHandler *ai);

bool Init_AssetImporter(void);
void Term_AssetImporter(void);

#endif /* _NE_EDITOR_ASSET_IMPORT_H_ */
