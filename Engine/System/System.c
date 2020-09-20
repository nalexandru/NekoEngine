#include <System/Log.h>
#include <System/System.h>

bool Sys_InitPlatform(void);
void Sys_TermPlatform(void);

bool
Sys_Init(void)
{
	if (!Sys_InitPlatform())
		return false;

#ifdef _DEBUG
	if (!Sys_InitDbgOut())
		return false;
#endif

	return true;
}

void
Sys_Term(void)
{
#ifdef _DEBUG
	Sys_TermDbgOut();
#endif

	Sys_TermPlatform();
}

