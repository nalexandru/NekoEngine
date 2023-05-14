#ifndef NE_HAIKU_PLATFORM_H
#define NE_HAIKU_PLATFORM_H

#include <View.h>
#include <Window.h>
#include <Application.h>

class NeEngineApplication : public BApplication
{
public:
	NeEngineApplication();
};

class NeEngineView : public BView
{
public:
	NeEngineView(BRect frame, const char *name);

	void Draw(BRect dirty);

	virtual void KeyDown(const char *bytes, int32 numBytes) override;
	virtual void KeyUp(const char *bytes, int32 numBytes) override;
	virtual void MouseDown(BPoint where) override;
	virtual void MouseUp(BPoint where) override;
	virtual void MouseMoved(BPoint where, uint32 code, const BMessage *dragMessage) override;
};

class NeEngineWindow : public BWindow
{
public:
	NeEngineWindow(BRect frame, const char *title);
	
	virtual void WindowActivated(bool active) override;
	virtual bool QuitRequested() override;

private:
	NeEngineView *_view;
};

#endif /* NE_HAIKU_PLATFORM_H */

/* NekoEngine
 *
 * HaikuPlatform.h
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
