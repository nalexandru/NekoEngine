#include <System/Log.h>
#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Runtime/Runtime.h>

#include <Editor/GUI.h>
#include <Editor/Asset/Asset.h>

#define ED_A_MOD	"Asset"

static char _prgTextBuff[2048];

struct OpenJobArgs
{
	struct NeAssetOpenHandler *handler;
	char *path;
};

static bool _MatchMesh(const char *path) { return strstr(path, ".nmesh") != NULL; }
static bool _OpenMesh(const char *path);
static void _OpenJob(int worker, struct OpenJobArgs *args);

static bool _MatchScript(const char *path) { return strstr(path, ".lua") != NULL; }
static bool _OpenScript(const char *path) { EdGUI_ScriptEditor(path); return true; }

struct NeAssetOpenHandler
{
	bool (*Match)(const char *path);
	bool (*Open)(const char *path);
};

static struct NeAssetOpenHandler _handlers[] =
{
	{
		.Match = _MatchMesh,
		.Open = _OpenMesh
	},
	{
		.Match = _MatchScript,
		.Open = _OpenScript
	}
};

void
Ed_OpenAsset(const char *path)
{
	for (size_t i = 0; i < sizeof(_handlers) / sizeof(_handlers[0]); ++i) {
		if (!_handlers[i].Match(path))
			continue;

		struct OpenJobArgs *args = Sys_Alloc(sizeof(*args), 1, MH_Editor);
		args->handler = &_handlers[i];
		args->path = Rt_StrDup(path, MH_Editor);

		E_ExecuteJob((NeJobProc)_OpenJob, args, NULL, NULL);

		break;
	}
}

static bool
_OpenMesh(const char *path)
{
	snprintf(_prgTextBuff, 2048, "Loading %s...", path);
	EdGUI_ShowProgressDialog("Creating entity...");

	char *str = Rt_TransientStrDup(path);
	char *name = strrchr(str, '/');
	*name++ = 0x0;

	char *ptr = strrchr(name, '.');
	*ptr = 0x0;

	NeEntityHandle eh = E_CreateEntity(name, NULL);

	EdGUI_UpdateProgressDialog("Creating transform...");
	E_AddNewComponent(eh, E_ComponentTypeId(TRANSFORM_COMP), NULL);

	EdGUI_UpdateProgressDialog(_prgTextBuff);
	const void *args[] = { "Model", path, NULL };
	E_AddNewComponent(eh, E_ComponentTypeId(MODEL_RENDER_COMP), args);

	EdGUI_HideProgressDialog();

	return true;
}

static void
_OpenJob(int worker, struct OpenJobArgs *args)
{
	if (!args->handler->Open(args->path)) {
		EdGUI_MessageBox("Error", "Failed to open asset", MB_Error);
		Sys_LogEntry(ED_A_MOD, LOG_CRITICAL, "Failed to open asset from file: %s", args->path);
	}
	
	Sys_Free(args->path);
	Sys_Free(args);
}

/* NekoEditor
 *
 * Asset.c
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
