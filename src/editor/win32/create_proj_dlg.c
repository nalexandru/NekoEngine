/* NekoEditor
 *
 * proj_open_dlg.c
 * Author: Alexandru Naiman
 *
 * NekoEditor Win32 Project Open Dialog
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#include <Windows.h>

#include <system/log.h>

#include <editor/ui.h>

#include "resource.h"
#include "ed_win32.h"

static const char* _path;

BOOL CALLBACK
create_proj_dlg_proc(
	HWND hdlg,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	switch (msg) {
	case WM_COMMAND:
		if (LOWORD(wparam) == IDCANCEL) {
			EndDialog(hdlg, 0);
			return FALSE;
		}
		/*else if (LOWORD(wparam) == IDOPEN) {
			_path = "blyat";
			EndDialog(hdlg, 1);
			return FALSE;
		}
		else if (LOWORD(wparam) == IDNEW) {
			EndDialog(hdlg, 2);
			return FALSE;
		}*/
		break;
	}

	return FALSE;
}

const char*
edui_create_proj_dlg(void)
{
	MSG msg;

	int res = DialogBox(win32_ed_instance,
		MAKEINTRESOURCE(IDD_CREATE_PROJ),
		HWND_DESKTOP, create_proj_dlg_proc);

	if (res < 0) {
		log_entry("Win32UI", LOG_CRITICAL, "Failed to show dialog: %d",
			GetLastError());
		return NULL;
	}

	if (!res)
		return NULL;

	if (res == 1)
		return _path;

	// create new project
}
