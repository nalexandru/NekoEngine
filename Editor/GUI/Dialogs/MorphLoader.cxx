#include "MorphLoader.h"

#include <atomic>

#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>

#include <Asset/NMesh.h>
#include <Render/Model.h>
#include <Runtime/Runtime.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>
#include <Editor/Editor.h>
#include <System/System.h>

using namespace std;

NeMorphLoader::NeMorphLoader(const QString &path, QWidget *parent) : QDialog(parent)
{
	setWindowTitle("Morph Loader");
	setWindowIcon(QIcon(":/EdIcon.png"));

	QVBoxLayout *root = new QVBoxLayout(this);
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

	root->addLayout(hLyt);

	_qfd = new QFileDialog(this);
	_qfd->setFileMode(QFileDialog::ExistingFile);
	connect(_qfd, SIGNAL(finished(int)), this, SLOT(FileDialogFinished(int)));

	_path = path;
}

void
NeMorphLoader::Add()
{
	_qfd->show();
	_qfd->raise();
	_qfd->activateWindow();
}

void
NeMorphLoader::Remove()
{
	_list->removeItemWidget(_list->selectedItems()[0]);
}

void
NeMorphLoader::Import()
{
	static atomic_bool f_importWait;

	if (!_list->count()) {
		QMessageBox::warning(nullptr, "Warning", "No morph files selected");
		return;
	}

	struct NMesh mesh{};
	if (!EdAsset_LoadNMesh(&mesh, _path.toStdString().c_str())) {
		QMessageBox::critical(nullptr, "Error", "Failed to load mesh file");
		return;
	}

	QFileInfo mqfi(_path);
	QString meshName = mqfi.baseName();

	NeArray deltas{};
	Rt_InitArray(&deltas, 10, sizeof(struct NeMorphDelta), MH_Editor);

	for (int i = 0 ; i < _list->count(); ++i) {
		struct NMesh nm{};
		struct NeMorph m{};
		const QListWidgetItem *item = _list->item(i);

		f_importWait.store(true);

		NeAssetImportOptions opt{ 1, 1, &nm };
		Asset_Import(item->text().toStdString().c_str(), &opt, []{ f_importWait.store(false); });

		while (f_importWait.load()) ;

		if (!nm.vertexCount) {
			QMessageBox::critical(this, "Error", QString("Failed to load mesh from %1").arg(item->text()));
			continue;
		}

		if (mesh.vertexCount != nm.vertexCount) {
			QMessageBox::critical(this, "Error", QString("The number of vertices does not match for %1").arg(item->text()));
			continue;
		}

		Rt_ClearArray(&deltas, false);
		EdAsset_CalculateDeltas(mesh.vertices, nm.vertices, mesh.vertexCount, &deltas);

		QFileInfo qfi(item->text());
		QString dstPath = QString("%1/Morphs/%2/%3.nmorph").arg(Ed_dataDir).arg(meshName).arg(qfi.baseName());



		QFileInfo dfi(dstPath.toStdString().c_str());
		if (!Sys_DirectoryExists(dfi.dir().path().toStdString().c_str()))
			Sys_CreateDirectory(dfi.dir().path().toStdString().c_str());

		strlcpy(m.name, qfi.baseName().toStdString().c_str(), sizeof(m.name));
		m.hash = Rt_HashString(m.name);
		m.deltaCount = (uint32_t)deltas.count;

		EdAsset_SaveNMorph(&m, (struct NeMorphDelta *) deltas.data, dstPath.toStdString().c_str());
	}

	Rt_TermArray(&deltas);

	accept();
}

void
NeMorphLoader::FileDialogFinished(int r)
{
	if (r == QDialog::Accepted)
		_list->addItem(new QListWidgetItem(_qfd->selectedFiles().at(0)));
	_qfd->hide();
}

NeMorphLoader::~NeMorphLoader() noexcept = default;

/* NekoEditor
 *
 * MorphLoader.cxx
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