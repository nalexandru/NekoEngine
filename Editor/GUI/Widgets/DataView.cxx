#include "DataView.h"

#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

#include <Math/Math.h>
#include <Engine/Component.h>

NeDataView::NeDataView(void *data, const QList<NeDataField> &fields, QWidget *parent) : QWidget(parent)
{
	QLabel *lbl;
	QLineEdit *le;
	NeDataBinding *b;
	QVBoxLayout *root = new QVBoxLayout(this);

	_data = (uint8_t *)data;
	_fields = fields;

#define ADD_LABEL(text)			\
	lbl = new QLabel(text);		\
	lbl->setMinimumWidth(10);	\
	lyt->addWidget(lbl)

#define ADD_LEDIT(type, val)			\
	b = new type(val, this);			\
	le = new QLineEdit(b->Read());	\
	le->setMinimumWidth(50);		\
	_editFields.append(le);			\
	lyt->addWidget(le);				\
	_bindings.append(b);			\
	connect(le, SIGNAL(textEdited(const QString &)), b, SLOT(Write(const QString &)))

	for (const NeDataField &f : _fields) {
		QHBoxLayout *lyt = new QHBoxLayout();
		lbl = new QLabel(f.name);
		lbl->setMinimumWidth(40);
		lyt->addWidget(lbl);

		switch (f.type) {
		case FT_INT32: {
			ADD_LEDIT(NeInt32Binding, (int32_t *)(_data + f.offset));
		} break;
		case FT_INT64: {
			ADD_LEDIT(NeInt64Binding, (int64_t *)(_data + f.offset));
		} break;
		case FT_BOOL: {
			ADD_LEDIT(NeBoolBinding, (bool *)(_data + f.offset));
		} break;
		case FT_FLOAT: {
			ADD_LEDIT(NeFloatBinding, (float *)(_data + f.offset));
		} break;
		case FT_DOUBLE: {
			ADD_LEDIT(NeDoubleBinding, (double *)(_data + f.offset));
		} break;
		case FT_STRING: {
			ADD_LEDIT(NeStringBinding, (const char **)(_data + f.offset));
		} break;
		case FT_VEC2: {
			struct NeVec2 *v = (struct NeVec2 *)(_data + f.offset);

			ADD_LABEL("x");
			ADD_LEDIT(NeFloatBinding, &v->x);

			ADD_LABEL("y");
			ADD_LEDIT(NeFloatBinding, &v->y);
		} break;
		case FT_VEC3: {
			struct NeVec3 *v = (struct NeVec3 *)(_data + f.offset);

			ADD_LABEL("x");
			ADD_LEDIT(NeFloatBinding, &v->x);

			ADD_LABEL("y");
			ADD_LEDIT(NeFloatBinding, &v->y);

			ADD_LABEL("z");
			ADD_LEDIT(NeFloatBinding, &v->z);
		} break;
		case FT_VEC4: {
			struct NeVec4 *v = (struct NeVec4 *)(_data + f.offset);

			ADD_LABEL("x");
			ADD_LEDIT(NeFloatBinding, &v->x);

			ADD_LABEL("y");
			ADD_LEDIT(NeFloatBinding, &v->y);

			ADD_LABEL("z");
			ADD_LEDIT(NeFloatBinding, &v->z);

			ADD_LABEL("w");
			ADD_LEDIT(NeFloatBinding, &v->w);
		} break;
		case FT_QUAT: {
			struct NeQuaternion *v = (struct NeQuaternion *)(_data + f.offset);

			ADD_LABEL("x");
			ADD_LEDIT(NePitchBinding, v);

			ADD_LABEL("y");
			ADD_LEDIT(NeYawBinding, v);

			ADD_LABEL("z");
			ADD_LEDIT(NeRollBinding, v);
		} break;
		default:
		break;
		}

		root->addLayout(lyt);
	}

#undef ADD_LABEL
#undef ADD_LEDIT
}

void
NeDataView::Frame() noexcept
{
	for (int32_t i = 0; i < _editFields.count(); ++i)
		if (!_editFields[i]->hasFocus())
			_editFields[i]->setText(_bindings[i]->Read());
}

NeDataView::~NeDataView() noexcept
{
	//
}

/* NekoEditor
 *
 * DataView.cxx
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