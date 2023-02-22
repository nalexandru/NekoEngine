#include <QMessageBox>
#include <QApplication>
#include <QStyleFactory>
#include <QProgressDialog>

#include <Engine/Config.h>
#include <System/PlatformDetect.h>

#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Project.h>
#include <System/Window.h>
#include <System/Thread.h>
#include <Scene/Components.h>
#include <Scene/Transform.h>

#include "EditorWindow.h"

#ifdef SYS_PLATFORM_WINDOWS
#	include <Windows.h>
#	include <Uxtheme.h>

enum PreferredAppMode
{
	Default,
	AllowDark,
	ForceDark,
	ForceLight,
	Max
};

typedef BOOL(WINAPI *SHOULDAPPSUSEDARKMODEPROC)(void); // ordinal 132
typedef BOOL(WINAPI *ALLOWDARKMODEFORWINDOWPROC)(HWND hWnd, bool allow); // ordinal 133
typedef BOOL(WINAPI *SHOULDSYSTEMUSEDARKMODEPROC)(void); // ordinal 138
typedef enum PreferredAppMode(WINAPI *SETPREFERREDAPPMODEPROC)(enum PreferredAppMode appMode); // ordinal 135, in 1903
#endif

struct MBoxInfo
{
	char *title;
	char *message;
	enum NeMessageBoxType type;
};

static bool _showProgress = false;
static QString _progressText;
static QApplication *_app;
static QProgressDialog *_progress;
static NeThread _guiThread;
static NeFutex _guiFutex;
static NeArray _mboxes;

NeEditorWindow *Ed_mainWindow = nullptr;

static inline void _InitTheme(void);

void
Ed_ShowProjectDialog(void)
{
	Ed_LoadProject(NULL);
}

bool
Ed_CreateGUI(int argc, char *argv[])
{
	struct NeDataField f[] =
	{
		{ .type = FT_VEC3, .offset = offsetof(struct NeTransform, position), .name = "Position" },
		{ .type = FT_QUAT, .offset = offsetof(struct NeTransform, rotation), .name = "Rotation" },
		{ .type = FT_VEC3, .offset = offsetof(struct NeTransform, scale), .name = "Scale" },
	};

	struct NeDataField *fields = (NeDataField *)Sys_Alloc(sizeof(f), 1, MH_Editor);
	memcpy(fields, f, sizeof(f));

	struct NeComponentFields cf = { .type = E_ComponentTypeId(TRANSFORM_COMP), .fieldCount = sizeof(f) / sizeof(f[0]), .fields = fields };
	Rt_ArrayAdd(&Ed_componentFields, &cf);

	_app = new QApplication(argc, argv);
	if (!_app)
		return false;

	Ed_mainWindow = new NeEditorWindow();
	Ed_mainWindow->show();

	_guiThread = Sys_CurrentThread();
	Sys_InitFutex(&_guiFutex);

	Rt_InitArray(&_mboxes, 1, sizeof(struct MBoxInfo), MH_Editor);

	_InitTheme();

#ifdef SYS_PLATFORM_X11
	// The second call to _app->processEvents() in EdGUI_Frame will crash in QCoreApplication::arguments()
	// on Linux machines. I believe this is because of the argc reference, so as a workaround i'm calling it here twice.
	// It's not necessary to call _app->processEvents() on Windows and macOS systems. On Windows it will cause problems
	// because it steals the messages from the Engine's loop.
	_app->processEvents();
	_app->processEvents();
#endif

	return true;
}

bool
Ed_InitGUI(void)
{
	return Ed_mainWindow->Init();
}

void
EdGUI_MessageBox(const char *title, const char *message, enum NeMessageBoxType type)
{
	if (Sys_CurrentThread() == _guiThread) {
		switch (type) {
			case MB_Information: QMessageBox::information(Ed_mainWindow, title, message); break;
			case MB_Warning: QMessageBox::warning(Ed_mainWindow, title, message); break;
			case MB_Error: QMessageBox::critical(Ed_mainWindow, title, message); break;
		}
	} else {
		Sys_LockFutex(_guiFutex);
		struct MBoxInfo mbox{ strdup(title), strdup(message), type };
		Rt_ArrayAdd(&_mboxes, &mbox);
		Sys_UnlockFutex(_guiFutex);
	}
}

void
EdGUI_ShowProgressDialog(const char *text)
{
	Sys_LockFutex(_guiFutex);
	_progressText = text;
	_showProgress = true;
	Sys_UnlockFutex(_guiFutex);
}

void
EdGUI_UpdateProgressDialog(const char *text)
{
	Sys_LockFutex(_guiFutex);
	_progressText = text;
	Sys_UnlockFutex(_guiFutex);
}

void
EdGUI_HideProgressDialog(void)
{
	Sys_LockFutex(_guiFutex);
	_showProgress = false;
	Sys_UnlockFutex(_guiFutex);
}

void
EdGUI_ScriptEditor(const char *path)
{
}

void
EdGUI_Frame(void)
{
	Sys_LockFutex(_guiFutex);

	struct MBoxInfo *mboxInfo;
	Rt_ArrayForEach(mboxInfo, &_mboxes, struct MBoxInfo *) {
		EdGUI_MessageBox(mboxInfo->title, mboxInfo->message, mboxInfo->type);
		free(mboxInfo->title);
		free(mboxInfo->message);
	}
	_mboxes.count = 0;

	Ed_mainWindow->Frame();

	if (_showProgress) {
		if (!_progress) {
			_progress = new QProgressDialog(_progressText, QString(), 0, 0, Ed_mainWindow);
			_progress->setWindowModality(Qt::ApplicationModal);
			_progress->show();
			_progress->raise();
			_progress->activateWindow();
		} else {
			_progress->setLabelText(_progressText);
		}

		_progress->update();
	} else if (_progress) {
		delete _progress;
		_progress = nullptr;
	}

#ifdef SYS_PLATFORM_X11
	_app->processEvents();
#endif

	Sys_UnlockFutex(_guiFutex);
}

void
Ed_TermGUI(void)
{
	Sys_TermFutex(_guiFutex);

	Rt_TermArray(&_mboxes);

	delete _progress;

	Ed_mainWindow->close();
	delete Ed_mainWindow;

	_app->exit(0);
	delete _app;
}

static inline void
_InitTheme(void)
{
	const char *theme = CVAR_STRING("Editor_Theme");
	if (theme)
		_app->setStyle(QStyleFactory::create(theme));

#ifdef SYS_PLATFORM_WINDOWS
	if (!theme) {
		struct NeSysVersion ver = Sys_OperatingSystemVersion();

		bool darkTheme = false;
		if (ver.major >= 10 && ver.revision >= 18363) { // Dark mode supported on Windows 19 1903 and newer
			HMODULE uxtheme = LoadLibrary(L"uxtheme");

		#define LOAD_PROC(x, y, z) x y = (x)GetProcAddress(uxtheme, MAKEINTRESOURCEA(z))
			LOAD_PROC(SHOULDAPPSUSEDARKMODEPROC, ShouldAppsUseDarkMode, 132);
			LOAD_PROC(ALLOWDARKMODEFORWINDOWPROC, AllowDarkModeForWindow, 133);
			LOAD_PROC(SHOULDSYSTEMUSEDARKMODEPROC, ShouldSystemUseDarkMode, 138);
			LOAD_PROC(SETPREFERREDAPPMODEPROC, SetPreferredAppMode, 135);
		#undef LOAD_PROC

			darkTheme = ShouldAppsUseDarkMode();
			if (darkTheme) {
				SetPreferredAppMode(ForceDark);
				AllowDarkModeForWindow((HWND)Ed_mainWindow->winId(), true);
			}

			FreeLibrary(uxtheme);
		}

		if (E_GetCVarBln("Editor_Win32DarkTheme", darkTheme)->bln) {
			QPalette p;
			QColor color = QColor(45, 45, 45), disabledColor = QColor(127, 127, 127);

			p.setColor(QPalette::Window, color);
			p.setColor(QPalette::WindowText, Qt::white);
			p.setColor(QPalette::Base, QColor(18, 18, 18));
			p.setColor(QPalette::AlternateBase, color);
			p.setColor(QPalette::ToolTipBase, Qt::white);
			p.setColor(QPalette::ToolTipText, Qt::white);
			p.setColor(QPalette::Text, Qt::white);
			p.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
			p.setColor(QPalette::Button, color);
			p.setColor(QPalette::ButtonText, Qt::white);
			p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
			p.setColor(QPalette::BrightText, Qt::red);
			p.setColor(QPalette::Link, QColor(42, 130, 218));
			p.setColor(QPalette::Highlight, QColor(42, 130, 218));
			p.setColor(QPalette::HighlightedText, Qt::black);
			p.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

			_app->setStyle(QStyleFactory::create("Fusion"));
			_app->setPalette(p);
			_app->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
		}
	}
#endif
}

/* NekoEditor
 *
 * GUI.cxx
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
