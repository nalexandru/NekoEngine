#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include <Editor/GUI.h>
#include <Editor/Asset/Import.h>

#define ED_AI_MOD	"AssetImport"

struct NeAssetImportArgs
{
	const struct NeAssetImportHandler *ai;
	char *path;
	struct NeAssetImportOptions options;
};

struct NeArray f_importers;

static bool f_importInProgress = false;

static void ImportJob(int i, struct NeAssetImportArgs *args);
static void ImportCompleted(uint64_t id, void (*completed)(void));

void
Asset_Import(const char *path, const struct NeAssetImportOptions *options, void (*completed)(void))
{
	struct NeAssetImportHandler *ai = NULL;
	Rt_ArrayForEach(ai, &f_importers) {
		if (!ai->Match(path))
			continue;

		struct NeAssetImportArgs *args = Sys_Alloc(sizeof(*args), 1, MH_Editor);
		args->ai = ai;
		args->path = Rt_StrDup(path, MH_Editor);

		if (options)
			args->options = *options;

		f_importInProgress = true;
		E_ExecuteJob((NeJobProc)ImportJob, args, (NeJobCompletedProc)ImportCompleted, completed);

		return;
	}

	EdGUI_MessageBox("Error", "No importer found for the specified file format !", MB_Error);
	Sys_LogEntry(ED_AI_MOD, LOG_CRITICAL, "No importer found for asset: %s", path);
}

bool
Asset_ImportInProgress(void)
{
	return f_importInProgress;
}

bool
Asset_RegisterImporter(const struct NeAssetImportHandler *ai)
{
	if (!f_importers.data)
		if (!Rt_InitArray(&f_importers, 10, sizeof(struct NeAssetImportHandler), MH_System))
			return false;
	return Rt_ArrayAdd(&f_importers, ai);
}

bool
Init_AssetImporter(void)
{
	return true;
}

void
Term_AssetImporter(void)
{
	Rt_TermArray(&f_importers);
}

static void
ImportJob(int i, struct NeAssetImportArgs *args)
{
	if (!args->ai->Import(args->path, &args->options))
		Sys_LogEntry(ED_AI_MOD, LOG_CRITICAL, "Import failed for asset: %s", args->path);

	Sys_Free(args->path);
	Sys_Free(args);
}

static void
ImportCompleted(uint64_t id, void (*completed)(void))
{
	if (completed)
		completed();

	f_importInProgress = false;
}

/* NekoEditor
 *
 * Import.c
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
