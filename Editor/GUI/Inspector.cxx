#include <QIcon>

#include "Inspector.h"

#include <Scene/Scene.h>
#include <Engine/Entity.h>

#include "Widgets/ComponentInspector.h"

static NeInspector *f_dlg;

NeInspector *
GUI_CreateInspector(void)
{
	f_dlg = new NeInspector();
	return f_dlg;
}

bool
GUI_InitInspector(void)
{
	f_dlg->show();
	return true;
}

void
GUI_InspectScene(void)
{
	f_dlg->InspectScene();
}

void
GUI_InspectEntity(NeEntityHandle handle)
{
	f_dlg->InspectEntity(handle);
}

void
GUI_TermInspector(void)
{
	f_dlg->close();
	delete f_dlg;
}

NeInspector::NeInspector(QWidget *parent) : QDialog(parent), _currentEntity(0)
{
	setMinimumSize(350, 600);
	setWindowTitle("Inspector");
	setWindowIcon(QIcon(":/EdIcon.png"));

	QVBoxLayout *top = new QVBoxLayout(this);

	_title = new QLabel("Inspector");
	top->addWidget(_title);

	_container = new QVBoxLayout();
	top->addLayout(_container, 100);

	_container->addStretch(100);

	setLayout(top);
}

void
NeInspector::InspectScene(void)
{
	_title->setText(Scn_activeScene->name);
}

void
NeInspector::InspectEntity(NeEntityHandle handle)
{
	if (_currentEntity == handle)
		return;

	_currentEntity = handle;
	while (QLayoutItem *item = _container->takeAt(0)) {
		delete item->widget();
		delete item;
	}

	if (handle == ES_INVALID_ENTITY) {
		_container->addStretch(100);
		_title->setText("No selection");
		return;
	}

	_title->setText(E_EntityName(handle));

	struct NeArray components;
	E_GetComponents(_currentEntity, &components);

	struct NeEntityComp *c = NULL;
	Rt_ArrayForEach(c, &components, struct NeEntityComp *)
		_container->addWidget(new NeComponentInspector(c));

	_container->addStretch(100);
}

void
NeInspector::Frame() noexcept
{
	for (int32_t i = 0; i < _container->count(); ++i) {
		NeComponentInspector *ci = (NeComponentInspector *)_container->itemAt(i)->widget();
		if (ci)
			ci->Frame();
	}
}

NeInspector::~NeInspector()
{
	delete _title;
	delete _container;
}

/* NekoEditor
 *
 * Inspector.cxx
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