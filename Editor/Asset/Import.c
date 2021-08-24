#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Editor/Asset/Import.h>

#define ED_AI_MOD	L"AssetImport"

struct ImportArgs
{
	const struct AssetImportHandler *ai;
	char *path;
};

struct Array _importers;

static bool _importInProgress = false;

static void _ImportJob(int i, struct ImportArgs *args);
static void _ImportCompleted(uint64_t id);

// Builtin importers
extern struct AssetImportHandler Ed_glTFImporter;

void
Asset_Import(const char *path)
{
	struct AssetImportHandler *ai = NULL;
	Rt_ArrayForEach(ai, &_importers) {
		if (!ai->Match(path))
			continue;

		struct ImportArgs *args = Sys_Alloc(sizeof(*args), 1, MH_Editor);
		args->ai = ai;
		args->path = Rt_StrDup(path, MH_Editor);

		_importInProgress = true;
		E_ExecuteJob((JobProc)_ImportJob, args, (JobCompletedProc)_ImportCompleted);

		return;
	}

	Sys_LogEntry(ED_AI_MOD, LOG_CRITICAL, L"No importer found for asset: %hs", path);
}

bool
Asset_ImportInProgress(void)
{
	return _importInProgress;
}

void
Asset_RegisterImporter(const struct AssetImportHandler *ai)
{
	Rt_ArrayAdd(&_importers, ai);
}

bool
Init_AssetImporter(void)
{
	if (!Rt_InitArray(&_importers, 10, sizeof(struct AssetImportHandler), MH_System))
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
_ImportJob(int i, struct ImportArgs *args)
{
	if (!args->ai->Import(args->path))
		Sys_LogEntry(ED_AI_MOD, LOG_CRITICAL, L"Import failed for asset: %hs", args->path);

	Sys_Free(args->path);
	Sys_Free(args);
}

static void
_ImportCompleted(uint64_t id)
{
	_importInProgress = false;
}
