#ifndef _NE_EDITOR_GUI_WIDGETS_COMPONENT_INSPECTOR_H_
#define _NE_EDITOR_GUI_WIDGETS_COMPONENT_INSPECTOR_H_

#include <Engine/Types.h>

#include <QMap>
#include <QWidget>
#include <QGroupBox>
#include <QBoxLayout>

class NeComponentInspector;

typedef NeComponentInspector *(*EdComponentInspectorCreateProc)(const struct NeEntityComp *);

template<typename T> NeComponentInspector *
edCreateCompInsp(const struct NeEntityComp *c, QWidget *parent = nullptr) { return new T(c, parent); }

class NeComponentInspector : public QWidget
{
	Q_OBJECT

public:
	explicit NeComponentInspector(const struct NeEntityComp *c, QWidget *parent = nullptr);
	virtual void Frame() noexcept;
	virtual ~NeComponentInspector() noexcept;

	static NeComponentInspector *Create(const struct NeEntityComp *c);
	static void Register(const QString &name, EdComponentInspectorCreateProc p)
	{
		if (!_classMap)
			_classMap = new QMap<QString, EdComponentInspectorCreateProc>();
		_classMap->insert(name, p);
	}

protected slots:
	void StateToggled(bool on);

protected:
	NeCompTypeId _typeId;
	NeCompHandle _handle;

	QGroupBox *_box;
	QBoxLayout *_rootLayout;

	virtual void _Init();

private:
	static QMap<QString, EdComponentInspectorCreateProc> *_classMap;
};

#define ED_COMPONENT_INSPECTOR(name, type)								\
	E_INITIALIZER(_NeEdCompInspRegister_ ## name) {					\
		NeComponentInspector::Register(name, &edCreateCompInsp<T>);	\
	}

#endif /* _NE_EDITOR_GUI_WIDGETS_COMPONENT_INSPECTOR_H_ */

/* NekoEditor
 *
 * ComponentInspector.h
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