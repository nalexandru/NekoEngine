#ifndef _NE_EDITOR_PROJECT_H_
#define _NE_EDITOR_PROJECT_H_

#include <Editor/Types.h>
#include <Runtime/Runtime.h>

struct NeProject
{
	char name[256];
	char author[256];

	char defaultScene[256];

	struct NeArray supportedPlatforms;
};

extern struct NeProject *Ed_activeProject;

bool Ed_LoadProject(const char *path);
bool Ed_SaveProject(const char *path);

#endif /* _NE_EDITOR_PROJECT_H_ */
