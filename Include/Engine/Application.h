#ifndef _NE_ENGINE_APPLICATION_H_
#define _NE_ENGINE_APPLICATION_H_

#include <Engine/Types.h>

struct NeApplicationInfo
{
	const char name[64];
	const char copyright[64];
	struct NeVersion version;
};

ENGINE_API extern struct NeApplicationInfo App_applicationInfo;

bool App_InitApplication(int argc, char *argv[]);
void App_Frame(void);
void App_TermApplication(void);

#endif /* _NE_ENGINE_APPLICATION_H_ */
