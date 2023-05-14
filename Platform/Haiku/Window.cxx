#include <Screen.h>

#include <Input/Input.h>
#include <Engine/Engine.h>
#include <System/Window.h>

#include "HaikuPlatform.h"

NeEngineView::NeEngineView(BRect frame, const char *name) :
	BView(frame, name, B_FOLLOW_ALL, B_WILL_DRAW)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	SetLowColor(0, 0, 0);
}

void
NeEngineView::Draw(BRect dirty)
{
}

void
NeEngineView::KeyDown(const char *bytes, int32 numBytes)
{
	BMessage *msg = Window()->CurrentMessage();
	
	int32 key;
	if (msg->FindInt32("key", &key) < B_OK)
		return;
		
	UnlockLooper();
//	In_Key(code, true);
	LockLooper();	
}

void
NeEngineView::KeyUp(const char *bytes, int32 numBytes)
{
	BMessage *msg = Window()->CurrentMessage();
	
	int32 key;
	if (msg->FindInt32("key", &key) < B_OK)
		return;
		
	UnlockLooper();
//	In_Key(code, false);
	LockLooper();	
}

void
NeEngineView::MouseDown(BPoint where)
{
	SetMouseEventMask(B_POINTER_EVENTS);
	BMessage *msg = Window()->CurrentMessage();
	uint32 btns;
	
	UnlockLooper();
	if (msg->FindInt32("buttons", (int32*)&btns) >= B_OK) {
		switch (btns) {
		case B_PRIMARY_MOUSE_BUTTON: In_buttonState[BTN_MOUSE_LMB] = true; break;
		case B_SECONDARY_MOUSE_BUTTON: In_buttonState[BTN_MOUSE_RMB] = true; break;
		case B_TERTIARY_MOUSE_BUTTON: In_buttonState[BTN_MOUSE_MMB] = true; break;
		}
	}
	LockLooper();
}

void
NeEngineView::MouseUp(BPoint where)
{
	SetMouseEventMask(B_POINTER_EVENTS);
	BMessage *msg = Window()->CurrentMessage();
	uint32 btns;
	
	UnlockLooper();
	if (msg->FindInt32("buttons", (int32*)&btns) >= B_OK) {
		switch (btns) {
		case B_PRIMARY_MOUSE_BUTTON: In_buttonState[BTN_MOUSE_LMB] = false; break;
		case B_SECONDARY_MOUSE_BUTTON: In_buttonState[BTN_MOUSE_RMB] = false; break;
		case B_TERTIARY_MOUSE_BUTTON: In_buttonState[BTN_MOUSE_MMB] = false; break;
		}
	}
	LockLooper();
}

void
NeEngineView::MouseMoved(BPoint where, uint32 code, const BMessage *dragMessage)
{
	UnlockLooper();
	In_mouseAxis[0] = where.x / (*E_screenWidth / 2);
	In_mouseAxis[1] = where.y / (*E_screenHeight / 2);
	LockLooper();
}

NeEngineWindow::NeEngineWindow(BRect frame, const char *title) :
	BWindow(frame, title, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	_view = new NeEngineView(Frame().OffsetToCopy(B_ORIGIN), "view");
	AddChild(_view, NULL);
	_view->MakeFocus();
}

void
NeEngineWindow::WindowActivated(bool active)
{
}

bool
NeEngineWindow::QuitRequested()
{
	E_Shutdown();
	return true;
}

bool
Sys_CreateWindow(void)
{	
	E_screen = new NeEngineWindow(BRect(0.f, 0.f, (float)*E_screenWidth - 1.f, (float)*E_screenHeight - 1.f), "NekoEngine");
	((NeEngineWindow *)E_screen)->CenterOnScreen();
	((NeEngineWindow *)E_screen)->Show();
	
	return true;
}

void
Sys_SetEngineWindow(void *wnd)
{
	E_screen = wnd;
}

void
Sys_SetWindowTitle(const char *title)
{
	((NeEngineWindow *)E_screen)->SetTitle(title);
}

void
Sys_MoveWindow(int x, int y)
{
	((NeEngineWindow *)E_screen)->MoveTo((float)x, (float)y);
}

void
Sys_ShowWindow(bool show)
{
	if (show)
		((NeEngineWindow *)E_screen)->Show();
	else
		((NeEngineWindow *)E_screen)->Hide();
}

void
Sys_WorkArea(int *top, int *left, int *right, int *bottom)
{
	BScreen screen = BScreen(((NeEngineWindow *)E_screen));
	BRect frame = screen.Frame();
	
	if (top)
		*top = (int)frame.top;
	
	if (left)
		*left = (int)frame.left;
		
	if (right)
		*right = (int)frame.right;
		
	if (bottom)
		*bottom = (int)frame.bottom;
}

void
Sys_NonClientMetrics(int32_t *titleBarHeight, int32_t *borderHeight, int32_t *borderWidth)
{
	if (titleBarHeight)
		*titleBarHeight = 0;
		
	if (borderHeight)
		*borderHeight = 0;
		
	if (borderWidth)
		*borderWidth = 0;
}

void
Sys_DestroyWindow(void)
{
	delete ((NeEngineWindow *)E_screen);
}

/* NekoEngine
 *
 * Window.cxx
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
