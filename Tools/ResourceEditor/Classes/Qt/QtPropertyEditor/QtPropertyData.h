#ifndef __QT_PROPERTY_DATA_H__
#define __QT_PROPERTY_DATA_H__

#include <QStyledItemDelegate>
#include <QHash>
#include <QIcon>

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

	QWidget* GetOptionalWidget();
	void SetOptionalWidget(QWidget* widget);
	QWidget* GetOptionalWidgetViewport();
	void SetOptionalWidgetViewport(QWidget *viewport);
	void SetOptionalWidgetOverlay(bool overlay);
	bool GetOptionalWidgetOverlay();
	// TODO: implement
	//void SetOptionalWidgetSize(QSize size);
	//QSize GetOptionalWidgetSize();
	//void SetOptionalWidgetAlign();
	//void GetOptionalWidgetAlign();

    
protected:
	void ParentUpdate();

	//QHashIterator<QString, QtPropertyData*> ChildIterator();

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

	QWidget *optionalWidget;
	QWidget *optionalWidgetViewport;
	bool optionalWidgetOverlay;

	QtPropertyData *parent;
	QHash<QString, QtPropertyData *> children;
	QHash<QString, int> childrenOrder;
};

#endif // __QT_PROPERTY_DATA_H__
