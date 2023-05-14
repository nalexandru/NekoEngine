#include "CreateProject.h"

#include <QLabel>
#include <QMessageBox>
#include <QFormLayout>
#include <QPushButton>

#include <Asset/NMesh.h>
#include <Render/Model.h>
#include <Runtime/Runtime.h>
#include <Editor/Asset/Asset.h>

NeCreateProject::NeCreateProject(QWidget *parent) : QDialog(parent)
{
	setWindowTitle("Create Project");
	setWindowIcon(QIcon(":/EdIcon.png"));

	QFormLayout *root = new QFormLayout(this);

	_nameEdit = new QLineEdit();
	root->addRow(tr("&Name:"), _nameEdit);

	QPushButton *btn = nullptr;
	QHBoxLayout *hLyt = nullptr;

	hLyt = new QHBoxLayout();
	btn = new QPushButton(tr("&Cancel"));
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(reject()));
	hLyt->addWidget(btn);

	btn = new QPushButton(tr("&Create"));
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(Create()));
	hLyt->addWidget(btn);
	root->addRow(hLyt);

	btn->setDefault(true);

/*	QVBoxLayout *root = new QVBoxLayout(this);
	root->addWidget(new QLabel(QString("Target: %1").arg(path)));

	QHBoxLayout *hLyt = new QHBoxLayout();

	_list = new QListWidget();
	hLyt->addWidget(_list);

	QVBoxLayout *vLyt = new QVBoxLayout();

	QPushButton *btn = nullptr;

	btn = new QPushButton("Add");
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(Add()));
	vLyt->addWidget(btn);

	btn = new QPushButton("Remove");
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(Remove()));
	vLyt->addWidget(btn);

	vLyt->addStretch(100);

	btn = new QPushButton("Import");
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(Import()));
	vLyt->addWidget(btn);

	hLyt->addLayout(vLyt);

	root->addLayout(hLyt);*/

	_qfd = new QFileDialog(this);
	_qfd->setFileMode(QFileDialog::ExistingFile);
	connect(_qfd, SIGNAL(finished(int)), this, SLOT(FileDialogFinished(int)));
}

void
NeCreateProject::Create()
{
	accept();
}

void
NeCreateProject::FileDialogFinished(int r)
{
	/*if (r == QDialog::Accepted)
		_list->addItem(new QListWidgetItem(_qfd->selectedFiles().at(0)));*/
	_qfd->hide();
}

NeCreateProject::~NeCreateProject() noexcept = default;

/* NekoEditor
 *
 * CreateProject.cxx
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