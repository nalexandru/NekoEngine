#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Editor/Asset/Import.h>

#define ED_AI_MOD	"AssetImport"

struct NeAssetImportArgs
{
	const struct NeAssetImportHandler *ai;
	char *path;
};

struct NeArray _importers;

static bool _importInProgress = false;

static void _ImportJob(int i, struct NeAssetImportArgs *args);
static void _ImportCompleted(uint64_t id, void *args);

// Builtin importers
extern struct NeAssetImportHandler Ed_glTFImporter;

void
Asset_Import(const char *path)
{
	struct NeAssetImportHandler *ai = NULL;
	Rt_ArrayForEach(ai, &_importers) {
		if (!ai->Match(path))
			continue;

		struct NeAssetImportArgs *args = Sys_Alloc(sizeof(*args), 1, MH_Editor);
		args->ai = ai;
		args->path = Rt_StrDup(path, MH_Editor);

		_importInProgress = true;
		E_ExecuteJob((NeJobProc)_ImportJob, args, (NeJobCompletedProc)_ImportCompleted, NULL);

		return;
	}

	Sys_LogEntry(ED_AI_MOD, LOG_CRITICAL, "No importer found for asset: %s", path);
}

bool
Asset_ImportInProgress(void)
{
	return _importInProgress;
}

void
Asset_RegisterImporter(const struct NeAssetImportHandler *ai)
{
	Rt_ArrayAdd(&_importers, ai);
}

bool
Init_AssetImporter(void)
{
	if (!Rt_InitArray(&_importers, 10, sizeof(struct NeAssetImportHandler), MH_System))
		return false;

	Rt_ArrayAdd(&_importers, &Ed_glTFImporter);

	return true;
}

void
Term_AssetImporter(void)
{
	Rt_TermArray(&_importers);
}

static void
_ImportJob(int i, struct NeAssetImportArgs *args)
{
	if (!args->ai->Import(args->path))
		Sys_LogEntry(ED_AI_MOD, LOG_CRITICAL, "Import failed for asset: %s", args->path);

	Sys_Free(args->path);
	Sys_Free(args);
}

static void
_ImportCompleted(uint64_t id, void *args)
{
	_importInProgress = false;
}
