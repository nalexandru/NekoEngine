#ifndef _NE_EDITOR_GUI_H_
#define _NE_EDITOR_GUI_H_

#ifdef __cplusplus
extern "C" {
#endif

enum NeMessageBoxType
{
	MB_Information,
	MB_Warning,
	MB_Error
};

void Ed_ShowProjectDialog(void);

bool Ed_CreateGUI(int argc, char *argv[]);
bool Ed_InitGUI(void);

void EdGUI_ScriptEditor(const char *path);

void EdGUI_MessageBox(const char *title, const char *message, enum NeMessageBoxType type);

void EdGUI_ShowProgressDialog(const char *text);
void EdGUI_UpdateProgressDialog(const char *text);
void EdGUI_HideProgressDialog(void);

void EdGUI_Frame(void);

void Ed_TermGUI(void);

#ifdef __cplusplus
}
#endif

#endif /* _NE_EDITOR_GUI_H_ */

/* NekoEngine
 *
 * GUI.h
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
