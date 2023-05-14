#ifndef NE_EDITOR_GUI_BINDINGS_H
#define NE_EDITOR_GUI_BINDINGS_H

#include <QObject>

#include <Math/Math.h>

class NeDataBinding : public QObject
{
	Q_OBJECT

public:
	explicit NeDataBinding(QObject *parent = nullptr) : QObject(parent) { }
	virtual QString Read() const = 0;
	~NeDataBinding() noexcept { }

public slots:
	virtual void Write(const QString &text) = 0;
};

#define ED_DECLARE_BINDING_CLASS(ctype, type)								\
class Ne ## ctype ## Binding : public NeDataBinding						\
{																		\
	Q_OBJECT																\
public:																	\
	explicit Ne ## ctype ## Binding(type *v, QObject *parent = nullptr)	\
		: NeDataBinding(parent), _v(v) { }								\
	virtual QString Read() const override;								\
	~Ne ## ctype ## Binding() noexcept { }								\
public slots:																\
	virtual void Write(const QString &text) override;						\
private:																	\
	type *_v;															\
}

ED_DECLARE_BINDING_CLASS(Bool, bool);
ED_DECLARE_BINDING_CLASS(Int32, int32_t);
ED_DECLARE_BINDING_CLASS(Int64, int64_t);
ED_DECLARE_BINDING_CLASS(Float, float);
ED_DECLARE_BINDING_CLASS(Double, double);
ED_DECLARE_BINDING_CLASS(String, const char *);
ED_DECLARE_BINDING_CLASS(Pitch, NeQuaternion);
ED_DECLARE_BINDING_CLASS(Yaw, NeQuaternion);
ED_DECLARE_BINDING_CLASS(Roll, NeQuaternion);

#endif /* NE_EDITOR_GUI_BINDINGS_H */

/* NekoEditor
 *
 * Bindings.h
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
