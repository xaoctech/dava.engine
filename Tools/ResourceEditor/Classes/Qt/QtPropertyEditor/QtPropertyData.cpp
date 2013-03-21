#include <QPushButton>
#include "QtPropertyEditor/QtPropertyData.h"

QtPropertyData::QtPropertyData()
	: curFlags(0)
	, parent(NULL)
	, optionalWidget()
	, optionalWidgetViewport(NULL)
	, optionalWidgetOverlay(false)
{ }

QtPropertyData::QtPropertyData(const QVariant &value)
	: curValue(value)
	, curFlags(0)
	, parent(NULL)
	, optionalWidget()
	, optionalWidgetViewport(NULL)
	, optionalWidgetOverlay(false)
{ }

QtPropertyData::~QtPropertyData() 
{
	QHashIterator<QString, QtPropertyData*> i = QHashIterator<QString, QtPropertyData*>(children);
	while(i.hasNext())
	{
		i.next();

		QtPropertyData *data = i.value();
		if(NULL != data)
		{
			delete data;
		}
	}

	if(NULL != optionalWidget)
	{
		delete optionalWidget;
	}
}

QVariant QtPropertyData::GetValue()
{
	QVariant value = GetValueInternal();

	if(value != curValue)
	{
		curValue = value;
		ChildNeedUpdate();
	}

	return curValue;
}

void QtPropertyData::SetValue(const QVariant &value)
{
	SetValueInternal(value);
	curValue = value;

	ChildNeedUpdate();
	ParentUpdate();
}

void QtPropertyData::SetIcon(const QIcon &icon)
{
	curIcon = icon;
}

QIcon QtPropertyData::GetIcon()
{
	return curIcon;
}

int QtPropertyData::GetFlags()
{
	return curFlags;
}

void QtPropertyData::SetFlags(int flags)
{
	curFlags = flags;
}

QWidget* QtPropertyData::CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) 
{ 
	return CreateEditorInternal(parent, option);
}

void QtPropertyData::EditorDone(QWidget *editor)
{
    return EditorDoneInternal(editor);
}

void QtPropertyData::SetEditorData(QWidget *editor)
{
    return SetEditorDataInternal(editor);
}

void QtPropertyData::ParentUpdate()
{
	if(NULL != parent)
	{
		parent->ChildChanged(parent->children.key(this), this);
		parent->ParentUpdate();
	}
}

void QtPropertyData::ChildAdd(const QString &key, QtPropertyData *data)
{
	if(NULL != data && !key.isEmpty())
	{
		int sz = children.size();
        
        children.insert(key, data);
        childrenOrder.insert(key, sz);

		data->parent = this;
	}
}

int QtPropertyData::ChildCount()
{
	return children.size();
}

QPair<QString, QtPropertyData*> QtPropertyData::ChildGet(int i)
{
	QPair<QString, QtPropertyData*> p("", NULL);

	if(i >= 0 && i < children.size())
	{
		p.first = childrenOrder.key(i);
		p.second = children[p.first];
	}

	return p;
}

void QtPropertyData::ChildAdd(const QString &key, const QVariant &value)
{
	ChildAdd(key, new QtPropertyData(value));
}

QtPropertyData * QtPropertyData::ChildGet(const QString &key)
{
	QtPropertyData *data = NULL;

	if(children.contains(key))
	{
		data = children[key];
	}

	return data;
}

QWidget* QtPropertyData::GetOptionalWidget()
{
	return optionalWidget;
}

void QtPropertyData::SetOptionalWidget(QWidget* widget)
{
	if(NULL != optionalWidget)
	{
		delete optionalWidget;
	}

	optionalWidget = widget;

	if(NULL != optionalWidget)
	{
		optionalWidget->setParent(optionalWidgetViewport);
	}
}

void QtPropertyData::SetOptionalWidgetViewport(QWidget *viewport)
{
	optionalWidgetViewport = viewport;

	if(NULL != optionalWidget)
	{
		optionalWidget->setParent(viewport);
	}

	QHashIterator<QString, QtPropertyData*> i(children);
	while (i.hasNext()) 
	{
		i.next();
		i.value()->SetOptionalWidgetViewport(viewport);
	}
}

QWidget* QtPropertyData::GetOptionalWidgetViewport()
{
	return optionalWidgetViewport;
}

bool QtPropertyData::GetOptionalWidgetOverlay()
{
	return optionalWidgetOverlay;
}

void QtPropertyData::SetOptionalWidgetOverlay(bool overlay)
{
	optionalWidgetOverlay = overlay;
}


QVariant QtPropertyData::GetValueInternal()
{
	// should be re-implemented by sub-class

	return curValue;
}

void QtPropertyData::SetValueInternal(const QVariant &value)
{
	// should be re-implemented by sub-class

	curValue = value;
}

QWidget* QtPropertyData::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option)
{
	// should be re-implemented by sub-class

	return NULL;
}

void QtPropertyData::EditorDoneInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::SetEditorDataInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::ChildChanged(const QString &key, QtPropertyData *data)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::ChildNeedUpdate()
{
	// should be re-implemented by sub-class
}

