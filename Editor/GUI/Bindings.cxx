#include "Bindings.h"

void NeBoolBinding::Write(const QString &text) { /*_v = text.to();*/ }
QString NeBoolBinding::Read() const { return QString::asprintf(*_v ? "True" : "False"); }

void NeInt32Binding::Write(const QString &text) { *_v = text.toInt(); }
QString NeInt32Binding::Read() const { return QString::asprintf("%d", *_v); }

void NeInt64Binding::Write(const QString &text) { *_v = text.toLong(); }
QString NeInt64Binding::Read() const { return QString::asprintf("%lld", *_v); }

void NeFloatBinding::Write(const QString &text) { *_v = text.toFloat(); }
QString NeFloatBinding::Read() const { return QString::asprintf("%.02f", *_v); }

void NeDoubleBinding::Write(const QString &text) { *_v = text.toDouble(); }
QString NeDoubleBinding::Read() const { return QString::asprintf("%.02f", *_v); }

void NeStringBinding::Write(const QString &text) { /* */ }
QString NeStringBinding::Read() const { return *_v; }

void
NePitchBinding::Write(const QString &text)
{
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(text.toFloat(), M_QuatYaw(_v), M_QuatRoll(_v));
	M_Store(_v, q);
}
QString NePitchBinding::Read() const { return QString::asprintf("%.02f", M_QuatPitch(_v)); }

void
NeYawBinding::Write(const QString &text)
{
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(M_QuatPitch(_v), text.toFloat(), M_QuatRoll(_v));
	M_Store(_v, q);
}
QString NeYawBinding::Read() const { return QString::asprintf("%.02f", M_QuatYaw(_v)); }

void
NeRollBinding::Write(const QString &text)
{
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(M_QuatPitch(_v), M_QuatYaw(_v), text.toFloat());
	M_Store(_v, q);
}
QString NeRollBinding::Read() const { return QString::asprintf("%.02f", M_QuatRoll(_v)); }
