#ifndef _NE_EDITOR_GUI_BINDINGS_H_
#define _NE_EDITOR_GUI_BINDINGS_H_

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

#endif /* _NE_EDITOR_GUI_BINDINGS_H_ */