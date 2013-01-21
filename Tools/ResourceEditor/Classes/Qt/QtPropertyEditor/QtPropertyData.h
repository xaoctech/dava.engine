#ifndef __QT_PROPERTY_DATA_H__
#define __QT_PROPERTY_DATA_H__

#include <QStyledItemDelegate>
#include <QIcon>
#include <QMap>

class QtPropertyData
{
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

	QIcon GetIcon();
	void SetIcon(const QIcon &icon);

	int GetFlags();
	void SetFlags(int flags);

	QWidget* CreateEditor(QWidget *parent, const QStyleOptionViewItem& option);
	void EditorDone(QWidget *editor);
	void SetEditorData(QWidget *editor);

protected:
	void ParentUpdate();

	void ChildAdd(const QString &key, QtPropertyData *data);
	void ChildAdd(const QString &key, const QVariant &value);
	QtPropertyData* ChildGet(const QString &key);
	QMapIterator<QString, QtPropertyData*> ChildIterator();

protected:
	bool childrenItemsCreated;

	// Function should be re-implemented by sub-class
	virtual QVariant GetValueInternal();

	// Function should be re-implemented by sub-class
	virtual void SetValueInternal(const QVariant &value);

	// Function should be re-implemented by sub-class
	virtual QWidget* CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option);

    // Function should be re-implemented by sub-class
	virtual void EditorDoneInternal(QWidget *editor);

    // Function should be re-implemented by sub-class
	virtual void SetEditorDataInternal(QWidget *editor);

	// Function should be re-implemented by sub-class
	virtual void ChildChanged(const QString &key, QtPropertyData *data);

	// Function should be re-implemented by sub-class
	virtual void ChildNeedUpdate();

private:
	QVariant curValue;
	QIcon curIcon;
	int curFlags;

	QtPropertyData *parent;
	QMap<QString, QtPropertyData *> children;
};

#endif // __QT_PROPERTY_DATA_H__
