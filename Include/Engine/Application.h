#ifndef _E_APPLICATION_H_
#define _E_APPLICATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Engine/Types.h>

struct ApplicationInfo
{
	const wchar_t name[64];
	const wchar_t copyright[64];
	struct Version version;
};

ENGINE_API extern struct ApplicationInfo App_ApplicationInfo;

bool App_InitApplication(int argc, char *argv[]);
void App_TermApplication(void);

#ifdef __cplusplus
}
#endif

#endif /* _E_APPLICATION_H_ */
