#include "OpenProject.h"

#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "CreateProject.h"

NeOpenProject::NeOpenProject(QWidget *parent) : QDialog(parent), _path("")
{
	setWindowTitle("NekoEditor");
	setWindowIcon(QIcon(":/EdIcon.png"));

	QVBoxLayout *root = new QVBoxLayout(this);
	QHBoxLayout *hLyt = new QHBoxLayout();

	_list = new QListWidget();
	connect(_list, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(ListOpen(QListWidgetItem *)));

	QVBoxLayout *vLyt = new QVBoxLayout();

	vLyt->addWidget(new QLabel("Recent Projects:"));
	vLyt->addWidget(_list);

	hLyt->addLayout(vLyt);

	vLyt = new QVBoxLayout();

	QPushButton *btn = nullptr;

	btn = new QPushButton(tr("New Project"));
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(New()));
	vLyt->addWidget(btn);

	btn = new QPushButton(tr("Open Project"));
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(Open()));
	vLyt->addWidget(btn);

	vLyt->addStretch(100);

	btn = new QPushButton(tr("Quit"));
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(reject()));
	vLyt->addWidget(btn);

	hLyt->addLayout(vLyt);

	root->addLayout(hLyt);

	_qfd = new QFileDialog(this);
	_qfd->setFileMode(QFileDialog::ExistingFile);
	_qfd->setNameFilter("NeProject Files (*.NeProject)");
	connect(_qfd, SIGNAL(finished(int)), this, SLOT(FileDialogFinished(int)));
}

void
NeOpenProject::New()
{
	NeCreateProject cpDlg(this);
	if (cpDlg.exec() != QDialog::Accepted)
		return;

	// TODO: _path = ...

	accept();
}

void
NeOpenProject::Open()
{
	_qfd->show();
	_qfd->raise();
	_qfd->activateWindow();
}

void
NeOpenProject::ListOpen(QListWidgetItem *item)
{
	// TODO _path = ...
	accept();
}

void
NeOpenProject::FileDialogFinished(int r)
{
	_qfd->hide();
	if (r != QDialog::Accepted)
		return;

	// TODO: _path = ...
	//_list->addItem(new QListWidgetItem(_qfd->selectedFiles().at(0)));

	accept();
}

NeOpenProject::~NeOpenProject() noexcept = default;

/* NekoEditor
 *
 * OpenProject.cxx
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