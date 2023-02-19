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
