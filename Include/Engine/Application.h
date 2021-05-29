#ifndef _NE_ENGINE_APPLICATION_H_
#define _NE_ENGINE_APPLICATION_H_

#include <Engine/Types.h>

struct ApplicationInfo
{
	const wchar_t name[64];
	const wchar_t copyright[64];
	struct Version version;
};

ENGINE_API extern struct ApplicationInfo App_applicationInfo;

bool App_InitApplication(int argc, char *argv[]);
void App_Render(uint32_t frameId);
void App_TermApplication(void);

#endif /* _NE_ENGINE_APPLICATION_H_ */
