#include "QtPropertyEditor/QtPropertyData.h"

QtPropertyData::QtPropertyData()
	: curFlags(0)
	, parent(NULL)
{ }

QtPropertyData::QtPropertyData(const QVariant &value)
	: curValue(value)
	, curFlags(0)
	, parent(NULL)
{ }

QtPropertyData::~QtPropertyData() 
{
	QMapIterator<QString, QtPropertyData*> i = QMapIterator<QString, QtPropertyData*>(children);
	while(i.hasNext())
	{
		i.next();

		QtPropertyData *data = i.value();
		if(NULL != data)
		{
			delete data;
		}
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
		if(!children.contains(key))
		{
			children.insert(key, data);
		}
		else
		{
			if(NULL != children[key])
			{
				delete children[key];
			}

			children[key] = data;
		}

		data->parent = this;
	}
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

QMapIterator<QString, QtPropertyData*> QtPropertyData::ChildIterator()
{
	return QMapIterator<QString, QtPropertyData*>(children);
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

void QtPropertyData::ChildChanged(const QString &key, QtPropertyData *data)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::ChildNeedUpdate()
{
	// should be re-implemented by sub-class
}

