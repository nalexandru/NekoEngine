#include "ComponentInspector.h"

#include <QWindow>

#include <Engine/Entity.h>
#include <Engine/Component.h>

#include <Editor/Editor.h>

#include "DataView.h"

QMap<QString, NeComponentInspector *(*)(const struct NeEntityComp *)> *NeComponentInspector::_classMap = nullptr;

NeComponentInspector::NeComponentInspector(const struct NeEntityComp *c, QWidget *parent)
	: QWidget(parent), _typeId(c->type), _handle(c->handle)
{
	QVBoxLayout *lyt = new QVBoxLayout(this);
	lyt->setContentsMargins(0, 0, 0, 0);

	struct NeCompBase *comp = (struct NeCompBase *)(E_ComponentPtr(_handle));

	_box = new QGroupBox();
	_box->setMinimumHeight(40);
	_box->setTitle(E_ComponentTypeName(_typeId));
	_box->setCheckable(true);
	_box->setChecked(comp->_enabled);
	connect(_box, SIGNAL(toggled(bool)), this, SLOT(StateToggled(bool)));

	lyt->addWidget(_box);

	_rootLayout = new QBoxLayout(QBoxLayout::TopToBottom, _box);
	_rootLayout->setContentsMargins(0, 0, 0, 0);

	_Init();
}

void
NeComponentInspector::Frame() noexcept
{
	for (int32_t i = 0; i < _rootLayout->count(); ++i) {
		NeDataView *dv = (NeDataView *)_rootLayout->itemAt(i)->widget();
		if (dv)
			dv->Frame();
	}
}

void
NeComponentInspector::StateToggled(bool on)
{
	struct NeCompBase *comp = (struct NeCompBase *)(E_ComponentPtr(_handle));
	comp->_enabled = on;
}

void
NeComponentInspector::_Init()
{
	struct NeComponentFields *cf;
	Rt_ArrayForEach(cf, &Ed_componentFields, struct NeComponentFields *) {
		if (cf->type != _typeId)
			continue;

		QList<NeDataField> fields{};
		for (uint32_t i = 0; i < cf->fieldCount; ++i)
			fields.append(cf->fields[i]);

		_rootLayout->addWidget(new NeDataView(E_ComponentPtr(_handle), fields));
	}
}

NeComponentInspector::~NeComponentInspector() noexcept
{
	//
}

NeComponentInspector *
NeComponentInspector::Create(const struct NeEntityComp *c)
{
	EdComponentInspectorCreateProc p = _classMap->value(E_ComponentTypeName(c->type));
	return p ? p(c) : new NeComponentInspector(c);
}

/* NekoEditor
 *
 * ComponentInspector.cxx
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