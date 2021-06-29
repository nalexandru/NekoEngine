#include <assert.h>

#include <Editor/Editor.h>
#include <Engine/Version.h>
#include <Engine/Application.h>

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct ApplicationInfo App_applicationInfo =
{
	L"NekoEditor",
	E_CPY_STR,
	{ E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD, E_VER_REVISION }
};

static uint64_t _sceneLoadedEvt;

bool
App_InitApplication(int argc, char *argv[])
{
	return Ed_CreateGUI();
}

void
App_Frame(void)
{
#ifndef __APPLE__
	Ed_ProcessCocoaEvents();
#endif
}

void
App_TermApplication(void)
{
}
