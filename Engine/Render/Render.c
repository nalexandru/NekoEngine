#include <Engine/Config.h>
#include <Render/Render.h>
#include <System/System.h>

struct RenderEngine Re = { 0 };

static void *_renderLibrary = NULL;

#ifdef _WIN64
#	define DEFAULT_RMOD	"D3D12Render"
#else
#	define DEFAULT_RMOD NULL
#endif

#ifdef RE_BUILTIN
bool Re_InitLibrary(void);
#endif

bool
Re_Init(void)
{
#ifdef RE_BUILTIN
	bool (*InitLibrary)(void) = Re_InitLibrary;
#else
	bool (*InitLibrary)(void) = NULL;
	uint32_t *apiVersion = NULL;
	const char *library = NULL;

	library = E_GetCVarStr(L"Render_Module", "default")->str;
	if (!library)
		return false;
	else if (!strncmp(library, "default", strlen(library)))
		library = DEFAULT_RMOD;
	
	_renderLibrary = Sys_LoadLibrary(library);
	if (!_renderLibrary)
		return false;

	apiVersion = Sys_GetProcAddress(_renderLibrary, "Re_ApiVersion");
	if (!apiVersion)
		return false;

	if (*apiVersion != RE_API_VERSION)
		return false;

	InitLibrary = (bool(*)(void))Sys_GetProcAddress(_renderLibrary, "Re_InitLibrary");
#endif

	if (!InitLibrary())
		return false;

	return Re.Init();
}

void
Re_Term(void)
{
	Re.Term();

#ifndef RE_BUILTIN
	Sys_UnloadLibrary(_renderLibrary);
#endif
}

