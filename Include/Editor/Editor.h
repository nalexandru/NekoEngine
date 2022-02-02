#ifndef _NE_EDITOR_EDITOR_H_
#define _NE_EDITOR_EDITOR_H_

#include <Editor/Types.h>
#include <System/PlatformDetect.h>

#define ED_MAX_PATH		4096

#ifdef SYS_PLATFORM_WINDOWS
#	define ED_DIR_SEPARATOR	'\\'
#else
#	define ED_DIR_SEPARATOR	'/'
#endif

extern char Ed_dataDir[];

// This is not part of the exposed IO API
const char *E_RealPath(const char *path);

#endif /* _NE_EDITOR_EDITOR_H_ */
