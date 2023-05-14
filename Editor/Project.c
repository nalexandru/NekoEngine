#include <Engine/Config.h>

#include <Editor/Editor.h>
#include <Editor/Project.h>

struct NeProject *Ed_activeProject = NULL;

bool
Ed_LoadProject(const char *path)
{
	const char *dataDir = E_GetCVarStr("Engine_DataDir", "Data")->str;

/*#ifdef SYS_PLATFORM_WINDOWS
	if (Ed_dataDir[1] != ':') {
#else
	if (Ed_dataDir[0] != '/') {
#endif
		char *buff = Sys_Alloc(sizeof(*buff), 4096, MH_Transient);
		getcwd(buff, 4096);

		snprintf(Ed_dataDir, ED_MAX_PATH, "%s%c%s", buff, ED_DIR_SEPARATOR, dataDir);
	} else {*/
		strlcpy(Ed_dataDir, dataDir, ED_MAX_PATH);
//	}

	Ed_activeProject = (struct NeProject *)1;

	return true;
}

bool
Ed_SaveProject(const char *path)
{
	return false;
}

/* NekoEditor
 *
 * Project.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
