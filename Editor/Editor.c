#include <stdio.h>
#include <assert.h>

#include <Scene/Scene.h>
#include <System/System.h>

#include <Engine/Config.h>
#include <Engine/Events.h>
#include <Engine/Version.h>
#include <Engine/ECSystem.h>
#include <Engine/Application.h>

#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Project.h>
#include <Editor/Asset/Import.h>

#include <Scene/Components.h>

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

NE_APPLICATION("NekoEditor", E_CPY_STR, E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD, E_VER_REVISION);

char Ed_dataDir[ED_MAX_PATH] = { 0 };
struct NeArray Ed_componentFields = {0 };

static uint64_t _sceneLoadedEvt;

static void _SceneLoaded(void *user, void *args);
//static void _ComponentRegistered(void *user, struct NeCompType *type);

bool
App_EarlyInit(int argc, char *argv[])
{
	Ed_ShowProjectDialog();
	if (!Ed_activeProject)
		return false;

	if (!Ed_componentFields.data)
		Rt_InitArray(&Ed_componentFields, 10, sizeof(struct NeComponentFields), MH_Editor);

//	E_SetCVarBln("Window_CreateHidden", true);

	if (!Ed_CreateGUI(argc, argv))
		return false;

	return true;
}

bool
App_InitApplication(int argc, char *argv[])
{
	if (!Init_AssetImporter())
		return false;

	_sceneLoadedEvt = E_RegisterHandler(EVT_SCENE_LOADED, _SceneLoaded, NULL);
	
	Scn_StartSceneLoad("/Scenes/EditorTest.scn");

	return Ed_InitGUI();
}

void
App_Frame(void)
{
	EdGUI_Frame();
}

void
App_TermApplication(void)
{
	Ed_TermGUI();

	Term_AssetImporter();

	struct NeComponentFields *ins;
	Rt_ArrayForEach(ins, &Ed_componentFields)
		Sys_Free(ins->fields);

	Rt_TermArray(&Ed_componentFields);
}

void
_SceneLoaded(void *user, void *args)
{
	Scn_ActivateScene((struct NeScene *)args);
}

/* NekoEditor
 *
 * Editor.c
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
