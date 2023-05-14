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
#include "Dialogs/OpenProject.h"

#ifdef SYS_PLATFORM_WINDOWS
#	include <windows.h>
#	include <uxtheme.h>

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

static bool f_showProgress = false;
static QString f_progressText;
static QApplication *f_app;
static QProgressDialog *f_progress;
static NeThread f_guiThread;
static NeFutex f_guiFutex;
static NeArray f_mboxes;

NeEditorWindow *Ed_mainWindow = nullptr;

static inline void InitTheme(void);

bool
Ed_CreateGUI(int argc, char *argv[])
{
	f_app = new QApplication(argc, argv);
	if (!f_app)
		return false;

	InitTheme();

	NeOpenProject opDlg(nullptr);
	if (opDlg.exec() != QDialog::Accepted)
		return false;

	struct NeDataField f[] =
	{
		{ .type = FT_VEC3, .offset = offsetof(struct NeTransform, position), .name = "Position" },
		{ .type = FT_QUAT, .offset = offsetof(struct NeTransform, rotation), .name = "Rotation" },
		{ .type = FT_VEC3, .offset = offsetof(struct NeTransform, scale), .name = "Scale" },
	};

	struct NeDataField *fields = (NeDataField *)Sys_Alloc(sizeof(f), 1, MH_Editor);
	memcpy(fields, f, sizeof(f));

	struct NeComponentFields cf = { .type = NE_TRANSFORM_ID, .fieldCount = sizeof(f) / sizeof(f[0]), .fields = fields };
	Rt_TSArrayAdd(&Ed_componentFields, &cf);

	if (!Ed_LoadProject(opDlg.Path().toStdString().c_str()))
		return false;

	Ed_mainWindow = new NeEditorWindow();
	Ed_mainWindow->show();

	f_guiThread = Sys_CurrentThread();
	Sys_InitFutex(&f_guiFutex);

	Rt_InitArray(&f_mboxes, 1, sizeof(struct MBoxInfo), MH_Editor);

#ifdef SYS_PLATFORM_X11
	// The second call to f_app->processEvents() in EdGUI_Frame will crash in QCoreApplication::arguments()
	// on Linux machines. I believe this is because of the argc reference, so as a workaround i'm calling it here twice.
	// It's not necessary to call f_app->processEvents() on Windows and macOS systems. On Windows it will cause problems
	// because it steals the messages from the Engine's loop.
	f_app->processEvents();
	f_app->processEvents();
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
	if (Sys_CurrentThread() == f_guiThread) {
		switch (type) {
			case MB_Information: QMessageBox::information(Ed_mainWindow, title, message); break;
			case MB_Warning: QMessageBox::warning(Ed_mainWindow, title, message); break;
			case MB_Error: QMessageBox::critical(Ed_mainWindow, title, message); break;
		}
	} else {
		NeFutexLock lock(f_guiFutex);
		struct MBoxInfo mbox{ strdup(title), strdup(message), type };
		Rt_ArrayAdd(&f_mboxes, &mbox);
	}
}

void
EdGUI_ShowProgressDialog(const char *text)
{
	NeFutexLock lock(f_guiFutex);
	f_progressText = text;
	f_showProgress = true;
}

void
EdGUI_UpdateProgressDialog(const char *text)
{
	NeFutexLock lock(f_guiFutex);
	f_progressText = text;
}

void
EdGUI_HideProgressDialog(void)
{
	NeFutexLock lock(f_guiFutex);
	f_showProgress = false;
}

void
EdGUI_ScriptEditor(const char *path)
{
}

void
EdGUI_Frame(void)
{
	NeFutexLock lock(f_guiFutex);

	struct MBoxInfo *mboxInfo;
	Rt_ArrayForEach(mboxInfo, &f_mboxes, struct MBoxInfo *) {
		EdGUI_MessageBox(mboxInfo->title, mboxInfo->message, mboxInfo->type);
		free(mboxInfo->title);
		free(mboxInfo->message);
	}
	f_mboxes.count = 0;

	Ed_mainWindow->Frame();

	if (f_showProgress) {
		if (!f_progress) {
			f_progress = new QProgressDialog(f_progressText, QString(), 0, 0, Ed_mainWindow);
			f_progress->setWindowModality(Qt::ApplicationModal);
			f_progress->show();
			f_progress->raise();
			f_progress->activateWindow();
		} else {
			f_progress->setLabelText(f_progressText);
		}

		f_progress->update();
	} else if (f_progress) {
		delete f_progress;
		f_progress = nullptr;
	}

#ifdef SYS_PLATFORM_X11
	f_app->processEvents();
#endif
}

void
Ed_TermGUI(void)
{
	Sys_TermFutex(f_guiFutex);

	Rt_TermArray(&f_mboxes);

	delete f_progress;

	Ed_mainWindow->close();
	delete Ed_mainWindow;

	f_app->exit(0);
	delete f_app;
}

static inline void
InitTheme(void)
{
	const char *theme = CVAR_STRING("Editor_Theme");
	if (!theme) {
#ifdef SYS_PLATFORM_WINDOWS
		struct NeSysVersion ver = Sys_OperatingSystemVersion();

		if (ver.major >= 10 && ver.revision >= 18363) { // Dark mode supported on Windows 19 1903 and newer
			HMODULE uxtheme = LoadLibrary(L"uxtheme");

		#define LOAD_PROC(x, y, z) x y = (x)GetProcAddress(uxtheme, MAKEINTRESOURCEA(z))
			LOAD_PROC(SHOULDAPPSUSEDARKMODEPROC, ShouldAppsUseDarkMode, 132);
			LOAD_PROC(ALLOWDARKMODEFORWINDOWPROC, AllowDarkModeForWindow, 133);
			LOAD_PROC(SHOULDSYSTEMUSEDARKMODEPROC, ShouldSystemUseDarkMode, 138);
			LOAD_PROC(SETPREFERREDAPPMODEPROC, SetPreferredAppMode, 135);
		#undef LOAD_PROC

			BOOL darkTheme = ShouldAppsUseDarkMode();
			if (darkTheme) {
				SetPreferredAppMode(ForceDark);
				//AllowDarkModeForWindow((HWND)Ed_mainWindow->winId(), true);
			}

			FreeLibrary(uxtheme);
		}
#endif
		return;
	}

	const size_t len = strlen(theme);
	if (!strncmp(theme, "NELight", len)) {
		QPalette p;
		QColor color = QColor(0xF5, 0xEC, 0xE5);
		QColor textColor = QColor(0x5A, 0x51, 0x4C);
		QColor disabledColor = QColor(0x7F, 0x7F, 0x7F);
		QColor highlightColor = QColor(0xBE, 0x55, 0x04);

		p.setColor(QPalette::Window, color);
		p.setColor(QPalette::WindowText, textColor);
		p.setColor(QPalette::Base, QColor(0xEF, 0xE6, 0xDD));
		p.setColor(QPalette::AlternateBase, color);
		p.setColor(QPalette::ToolTipBase, Qt::black);
		p.setColor(QPalette::ToolTipText, textColor);
		p.setColor(QPalette::Text, textColor);
		p.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
		p.setColor(QPalette::Button, color);
		p.setColor(QPalette::ButtonText, textColor);
		p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
		p.setColor(QPalette::BrightText, Qt::red);
		p.setColor(QPalette::Link, highlightColor);
		p.setColor(QPalette::Highlight, highlightColor);
		p.setColor(QPalette::HighlightedText, Qt::black);
		p.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

		f_app->setStyle(QStyleFactory::create("Fusion"));
		f_app->setPalette(p);
		f_app->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid black; }");
	} else if (!strncmp(theme, "NEDark", len)) {
		QPalette p;
		QColor color = QColor(0x24, 0x18, 0x18);
		QColor textColor = QColor(0xB5, 0xB1, 0xA6);
		QColor disabledColor = QColor(0x7F, 0x7F, 0x7F);
		QColor highlightColor = QColor(0xBE, 0x55, 0x04);

		p.setColor(QPalette::Window, color);
		p.setColor(QPalette::WindowText, textColor);
		p.setColor(QPalette::Base, QColor(0x1F, 0x1B, 0x18));
		p.setColor(QPalette::AlternateBase, color);
		p.setColor(QPalette::ToolTipBase, Qt::white);
		p.setColor(QPalette::ToolTipText, textColor);
		p.setColor(QPalette::Text, textColor);
		p.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
		p.setColor(QPalette::Button, color);
		p.setColor(QPalette::ButtonText, textColor);
		p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
		p.setColor(QPalette::BrightText, Qt::red);
		p.setColor(QPalette::Link, highlightColor);
		p.setColor(QPalette::Highlight, highlightColor);
		p.setColor(QPalette::HighlightedText, Qt::black);
		p.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

		f_app->setStyle(QStyleFactory::create("Fusion"));
		f_app->setPalette(p);
		f_app->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
	} else {
		f_app->setStyle(QStyleFactory::create(theme));
	}
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
