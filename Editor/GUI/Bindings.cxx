#include "Bindings.h"

void NeBoolBinding::Write(const QString &text) { /*_v = text.to();*/ }
QString NeBoolBinding::Read() const { return QString::asprintf(*_v ? "True" : "False"); }

void NeInt32Binding::Write(const QString &text) { *_v = text.toInt(); }
QString NeInt32Binding::Read() const { return QString::asprintf("%d", *_v); }

void NeInt64Binding::Write(const QString &text) { *_v = text.toLong(); }
QString NeInt64Binding::Read() const { return QString::asprintf("%zd", *_v); }

void NeFloatBinding::Write(const QString &text) { *_v = text.toFloat(); }
QString NeFloatBinding::Read() const { return QString::asprintf("%.02f", *_v); }

void NeDoubleBinding::Write(const QString &text) { *_v = text.toDouble(); }
QString NeDoubleBinding::Read() const { return QString::asprintf("%.02f", *_v); }

void NeStringBinding::Write(const QString &text) { /* */ }
QString NeStringBinding::Read() const { return *_v; }

void
NePitchBinding::Write(const QString &text)
{
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(text.toFloat()),
		XMConvertToRadians(M_QuatYaw(_v)),
		XMConvertToRadians(M_QuatRoll(_v))
	);
	M_Store(_v, q);
}
QString NePitchBinding::Read() const { return QString::asprintf("%.02f", M_QuatPitch(_v)); }

void
NeYawBinding::Write(const QString &text)
{
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(M_QuatPitch(_v)),
		XMConvertToRadians(text.toFloat()),
		XMConvertToRadians(M_QuatRoll(_v))
	);
	M_Store(_v, q);
}
QString NeYawBinding::Read() const { return QString::asprintf("%.02f", M_QuatYaw(_v)); }

void
NeRollBinding::Write(const QString &text)
{
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(M_QuatPitch(_v)),
		XMConvertToRadians(M_QuatYaw(_v)),
		XMConvertToRadians(text.toFloat())
	);
	M_Store(_v, q);
}
QString NeRollBinding::Read() const { return QString::asprintf("%.02f", M_QuatRoll(_v)); }

/* NekoEditor
 *
 * Bindings.cxx
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
