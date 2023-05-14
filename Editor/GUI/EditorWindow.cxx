#include "EditorWindow.h"

#include <QWindow>
#include <QMenuBar>
#include <QTabWidget>
#include <QStatusBar>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileIconProvider>

#include <Runtime/Runtime.h>

#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/Version.h>
#include <System/System.h>
#include <System/Window.h>

#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>

#include "Inspector.h"
#include "SceneHierarchy.h"
#include "Widgets/EngineView.h"
#include "Dialogs/About.h"
#include "Dialogs/HelpViewer.h"
#include "Dialogs/MorphLoader.h"

NeEditorWindow::NeEditorWindow(QWidget *parent) : QMainWindow(parent), _gameProcess(0)
{
	setWindowIcon(QIcon(":/EdIcon.png"));
	setWindowTitle("NekoEditor");
	setMinimumSize(600, 400);

	_iconProvider = new QFileIconProvider();

	_inspector = GUI_CreateInspector();
	GUI_CreateSceneHierarchy();

	_helpViewer = new NeHelpViewer(this);

	_CreateMenu();

	_qfd = new QFileDialog(this);
	_qfd->setFileMode(QFileDialog::ExistingFile);
	connect(_qfd, SIGNAL(finished(int)), this, SLOT(FileDialogFinished(int)));

	QTabWidget *tabWidget = new QTabWidget();
	setCentralWidget(tabWidget);

	QWidget *assetManager = new QWidget();

	QVBoxLayout *rootLyt = new QVBoxLayout(assetManager);

	QHBoxLayout *hLyt = new QHBoxLayout();
	hLyt->setSizeConstraint(QLayout::SetNoConstraint);

	QPushButton *btn = new QPushButton("Import");
	btn->setMaximumWidth(75);
	hLyt->addWidget(btn);

	connect(btn, SIGNAL(clicked(bool)), this, SLOT(ImportAsset()));

	_amPathEdit = new QLineEdit(_amPath);
	_amPathEdit->setReadOnly(true);
	hLyt->addWidget(_amPathEdit);

	rootLyt->addLayout(hLyt);

	_amFileList = new QListWidget();

	if (CVAR_INT32("AssetManager_ViewMode") == 1) {
		_amFileList->setGridSize(QSize(64, 64));
		_amFileList->setViewMode(QListWidget::IconMode);
	}

	_amFileList->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(_amFileList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(AMDoubleClick(QListWidgetItem *)));
	connect(_amFileList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(AMContextMenu(const QPoint &)));

	rootLyt->addWidget(_amFileList);

	tabWidget->addTab(assetManager, "Asset Manager");

	setStatusBar(new QStatusBar());
	statusBar()->addPermanentWidget(
		new QLabel(QString::asprintf("NekoEditor v%s", E_VER_STR))
	);
	statusBar()->showMessage("Ready.");
}

bool
NeEditorWindow::Init()
{
	GUI_InitInspector();
	GUI_InitSceneHierarchy();

//	EngineWidget *_engineWidget = new EngineWidget(this);
//	setCentralWidget(engineWidget);
//	engineWidget->setGeometry(0, 0, 1280, 800);
//	engineWidget->setFixedSize(1280, 800);
//	_engineWidget->Init();

//	return _engineWidget != nullptr;

	_amPath = "/";
	UpdateFileList();

	raise();
	activateWindow();

	return true;
}

void
NeEditorWindow::Frame() noexcept
{
	_inspector->Frame();
//	_engineWidget->repaint();
//	repaint();
}

void
NeEditorWindow::UpdateFileList()
{
	_amFileList->clear();
	_amFileList->addItem(new QListWidgetItem(QIcon(":/Icons/back.png"), "..", _amFileList));

	const char **files = E_ListFiles(_amPath.toStdString().c_str());

	for (const char **i = files; *i != NULL; ++i) {
		if (*i[0] == '.' && !CVAR_BOOL("AssetManager_ShowHiddenFiles"))
			continue;

		QIcon icon{};
		QString path = QString::asprintf((_amPath.length() > 1 ? "%s/%s" : "%s%s"), _amPath.toStdString().c_str(), *i);

		if (E_IsDirectory(path.toStdString().c_str())) {
#ifdef SYS_PLATFORM_APPLE
			if (!CVAR_BOOL("AssetManager_ShowHiddenFiles") && !path.compare("/Data"))
				continue;
#endif
			icon = _iconProvider->icon(QFileIconProvider::Folder);
		} else {
			QFileInfo qfi(E_RealPath(path.toStdString().c_str()));
			const QString &ext = qfi.completeSuffix();

#ifdef SYS_PLATFORM_APPLE
			if (!CVAR_BOOL("AssetManager_ShowHiddenFiles"))
				if (!ext.compare("car") || !ext.compare("icns") || !ext.compare("metallib"))
					continue;
#endif

			if (!ext.compare("shader"))
				icon = QIcon(":/Icons/text.png");
			else if (!ext.compare("ent"))
				icon = QIcon(":/Icons/entity.png");
			else if (!ext.compare("scn"))
				icon = QIcon(":/Icons/scene.png");
			else if (!ext.compare("mat"))
				icon = QIcon(":/Icons/material.png");
			else if (!ext.compare("fnt"))
				icon = QIcon(":/Icons/font.png");
			else if (!ext.compare("nmesh"))
				icon = QIcon(":/Icons/model.png");
			else if (!ext.compare("nanim"))
				icon = QIcon(":/Icons/animation.png");
			else if (!ext.compare("lua"))
				icon = QIcon(":/Icons/script.png");
		#ifndef SYS_PLATFORM_MAC
			else if (!ext.compare("dds"))
				icon = QIcon(":/Icons/image.png");
		#endif
			else
				icon = _iconProvider->icon(qfi);
		}

		if (icon.isNull())
			icon = QIcon(":/Icons/generic.png");

		_amFileList->addItem(new QListWidgetItem(icon, *i, _amFileList));
	}

	E_FreeFileList(files);

	_amPathEdit->setText(_amPath);
}

void
NeEditorWindow::ImportAsset()
{
	if (Asset_ImportInProgress()) {
		QMessageBox::information(this, "Import in Progress", "Please wait until the current import operation finishes");
		return;
	}

	_qfd->show();
	_qfd->raise();
	_qfd->activateWindow();
}

void
NeEditorWindow::AMDoubleClick(QListWidgetItem *item)
{
	const QString path = _AMFilePath(item->text());

	if (E_IsDirectory(path.toStdString().c_str())) {
		_amPath = path;
		UpdateFileList();
	} else {
		Ed_OpenAsset(path.toStdString().c_str());
	}
}

void
NeEditorWindow::AMContextMenu(const QPoint &pt)
{
	QListWidgetItem *item = _amFileList->itemAt(pt);
	if (!item)
		return;

	QMenu menu{};

	if (item->text().contains(".nmesh"))
		menu.addAction("Load Morphs");

	if (menu.actions().count())
		menu.addSeparator();

	menu.addAction("Delete");

	QAction *action = menu.exec(_amFileList->mapToGlobal(pt));
	if (!action)
		return;

	if (!action->text().compare("Load Morphs")) {
		NeMorphLoader ml(_AMFilePath(item->text()));

		ml.exec();
	} else if (!action->text().compare("Delete")) {
		if (QMessageBox::question(nullptr, "Confirm delete", QString("Are sure you want to delete %1 ?").arg(item->text())) != QMessageBox::Yes)
			return;

		// exec delete
	}
}

void
NeEditorWindow::FileDialogFinished(int r)
{
	if (r == QDialog::Accepted)
		Asset_Import(_qfd->selectedFiles().at(0).toStdString().c_str(), NULL, NULL);

	_qfd->hide();
}

// File Menu
void
NeEditorWindow::NewProject()
{

}

void
NeEditorWindow::OpenProject()
{

}

void
NeEditorWindow::SaveProject()
{

}

void
NeEditorWindow::CloseProject()
{

}

void
NeEditorWindow::Quit()
{
	E_Shutdown();
}

// Project Menu
void
NeEditorWindow::Play()
{
	if (!_gameProcess) {
		char *wd = NULL;
		FILE *in, *out, *err;

		char bin[4096];
		Sys_ExecutableLocation(bin, sizeof(bin));

#if defined(SYS_PLATFORM_WINDOWS)
		strlcat(bin, "\\NekoEngine.exe", sizeof(bin));
#elif defined(SYS_PLATFORM_MAC)
		strlcat(bin, "/NekoEngine", sizeof(bin));
#else
		strlcat(bin, "/NekoEngine", sizeof(bin));
#endif

		char *argv[] =
		{
			bin,
			//"-c",
			//"cfgfile",
			//"-d",
			//"datadir",
			//"-s",
			//"scene",
			NULL
		};

		_gameProcess = Sys_Execute((char * const *)argv, wd, &in, &out, &err, true);
		if (_gameProcess == -1) {
			QMessageBox::critical(this, "Error", "Failed to start game");
			return;
		}

		Sys_ShowWindow(false);

		E_ExecuteJob([](int worker, void *child) {
			Sys_WaitForProcessExit(*((intptr_t *)child));
			Sys_ShowWindow(true);
			*((intptr_t *)child) = 0;
		}, (void *)&_gameProcess, nullptr, nullptr);
	} else {
		if (!Sys_TerminateProcess(_gameProcess)) {
			QMessageBox::critical(this, "Error", "Failed to terminate game process");
			return;
		}
	}
}

void
NeEditorWindow::BuildProject()
{

}

void
NeEditorWindow::ExportProject()
{

}

void
NeEditorWindow::ShowProjectSettings()
{

}

// Tools Menu
void
NeEditorWindow::ShowInspector()
{

}

void
NeEditorWindow::ShowSceneHierarchy()
{

}

void
NeEditorWindow::ShowRenderWindow()
{

}

void
NeEditorWindow::ShowScriptEditor()
{

}

// Help Menu
void
NeEditorWindow::ShowNativeRef()
{
	_helpViewer->show();
	_helpViewer->raise();
	_helpViewer->activateWindow();
}

void
NeEditorWindow::ShowScriptingRef()
{
	_helpViewer->show();
	_helpViewer->raise();
	_helpViewer->activateWindow();
}

void
NeEditorWindow::ShowAbout()
{
	NeAbout dlg;
	dlg.exec();
}

void
NeEditorWindow::ShowAboutQt()
{
	QMessageBox::aboutQt(this, "About Qt");
}

void
NeEditorWindow::_CreateMenu(void)
{
	QMenuBar *mb = new QMenuBar();

	QMenu *m = mb->addMenu("File");
	connect(m->addAction("New"), SIGNAL(triggered(bool)), this, SLOT(NewProject()));
	m->addSeparator();
	connect(m->addAction("Open"), SIGNAL(triggered(bool)), this, SLOT(OpenProject()));
	connect(m->addAction("Save"), SIGNAL(triggered(bool)), this, SLOT(SaveProject()));
	connect(m->addAction("Close"), SIGNAL(triggered(bool)), this, SLOT(CloseProject()));
	m->addSeparator();
	connect(m->addAction("Quit"), SIGNAL(triggered(bool)), this, SLOT(Quit()));

	m = mb->addMenu("Project");
	connect(m->addAction("Play / Stop"), SIGNAL(triggered(bool)), this, SLOT(Play()));
	m->addSeparator();
	connect(m->addAction("Build"), SIGNAL(triggered(bool)), this, SLOT(BuildProject()));
	connect(m->addAction("Export"), SIGNAL(triggered(bool)), this, SLOT(ExportProject()));
	m->addSeparator();
	connect(m->addAction("Settings"), SIGNAL(triggered(bool)), this, SLOT(ShowProjectSettings()));

	m = mb->addMenu("Tools");
	connect(m->addAction("Scene Hierarchy"), SIGNAL(triggered(bool)), this, SLOT(ShowSceneHierarchy()));
	connect(m->addAction("Inspector"), SIGNAL(triggered(bool)), this, SLOT(ShowInspector()));
	connect(m->addAction("Render Window"), SIGNAL(triggered(bool)), this, SLOT(ShowRenderWindow()));
	connect(m->addAction("Script Editor"), SIGNAL(triggered(bool)), this, SLOT(ShowScriptEditor()));
	m->addSeparator();
	m->addAction("Preferences");

	m = mb->addMenu("Help");
	connect(m->addAction("Native API Reference"), SIGNAL(triggered(bool)), this, SLOT(ShowNativeRef()));
	connect(m->addAction("Scripting API Reference"), SIGNAL(triggered(bool)), this, SLOT(ShowScriptingRef()));
	m->addSeparator();
	connect(m->addAction("About"), SIGNAL(triggered(bool)), this, SLOT(ShowAbout()));
	connect(m->addAction("About Qt"), SIGNAL(triggered(bool)), this, SLOT(ShowAboutQt()));

	mb->setNativeMenuBar(true);
	setMenuBar(mb);
}

QString
NeEditorWindow::_AMFilePath(const QString &name)
{
	if (!name.compare("..")) {
		if (int pos = _amPath.lastIndexOf('/'))
			return _amPath.remove(pos, INT_MAX);
		else
			return "/";
	} else {
		return QString::asprintf(_amPath.length() > 1 ? "%s/%s" : "%s%s", _amPath.toStdString().c_str(), name.toStdString().c_str());
	}
}

void
NeEditorWindow::closeEvent(QCloseEvent *event)
{
	QMainWindow::closeEvent(event);
	E_Shutdown();
}

NeEditorWindow::~NeEditorWindow() noexcept
{
	delete _iconProvider;
	delete _helpViewer;

	GUI_TermSceneHierarchy();
	GUI_TermInspector();
}

/* NekoEditor
 *
 * EditorWindow.cxx
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