#ifndef __QT_PROPERTY_DATA_H__
#define __QT_PROPERTY_DATA_H__

#include <QStyledItemDelegate>
#include <QHash>
#include <QIcon>

// Optional widget
struct QtPropertyOW
{
	QtPropertyOW() : widget(NULL), overlay(false), size(0, 0)
	{ }

	QtPropertyOW(QWidget *_widget, bool _overlay = false, QSize _size = QSize(16, 16))
		: widget(_widget), overlay(_overlay), size(_size)
	{ }

	QWidget *widget;
	QSize size;
	bool overlay;
};

// PropertyData class
class QtPropertyData : public QObject
{
	Q_OBJECT

	friend class QtPropertyItem;

public:
	enum
	{
		FLAG_EMPTY				= 0x0,

		FLAG_IS_DISABLED		= 0x1,
		FLAG_IS_CHECKABLE		= 0x2,
		FLAG_IS_NOT_EDITABLE	= 0x4,
	};

	QtPropertyData();
	QtPropertyData(const QVariant &value);
	virtual ~QtPropertyData() ;

	QVariant GetValue();
	void SetValue(const QVariant &value);

	virtual QIcon GetIcon();
	virtual void SetIcon(const QIcon &icon);

	int GetFlags();
	void SetFlags(int flags);

	QWidget* CreateEditor(QWidget *parent, const QStyleOptionViewItem& option);
	void EditorDone(QWidget *editor);
	void SetEditorData(QWidget *editor);

	void ChildAdd(const QString &key, QtPropertyData *data);
	void ChildAdd(const QString &key, const QVariant &value);
	int ChildCount();
	QtPropertyData* ChildGet(const QString &key);
	QPair<QString, QtPropertyData*> ChildGet(int i);
	void ChildRemove(const QString &key);
	void ChildRemove(QtPropertyData *data);
	void ChildRemove(int i);

signals:
	void ChildAdded(const QString &key, QtPropertyData *data);
	void ChildRemoving(const QString &key, QtPropertyData *data);
    
protected:
	QVariant curValue;
	QIcon curIcon;
	int curFlags;

	QtPropertyData *parent;
	QHash<QString, QtPropertyData *> children;
	QHash<QString, int> childrenOrder;

	void ParentUpdate();

	// Functions should be re-implemented by sub-class
	virtual QVariant GetValueInternal();
	virtual void SetValueInternal(const QVariant &value);
	virtual QWidget* CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option);
	virtual void EditorDoneInternal(QWidget *editor);
	virtual void SetEditorDataInternal(QWidget *editor);
	virtual void ChildChanged(const QString &key, QtPropertyData *data);
	virtual void ChildNeedUpdate();
	
public:
	// Option widgets
	int GetOWCount();
	const QtPropertyOW* GetOW(int index = 0);
	void AddOW(const QtPropertyOW &ow);
	void RemOW(int index);

	QWidget* GetOWViewport();
	void SetOWViewport(QWidget *viewport);

private:
	// Optional widgets data struct and memebers
	QVector<QtPropertyOW> optionalWidgets;
	QWidget *optionalWidgetViewport;
};

#endif // __QT_PROPERTY_DATA_H__
