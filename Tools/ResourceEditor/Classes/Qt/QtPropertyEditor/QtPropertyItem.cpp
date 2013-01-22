#include "QtPropertyEditor/QtPropertyItem.h"
#include "QtPropertyEditor/QtPropertyData.h"

QtPropertyItem::QtPropertyItem()
	: QStandardItem()
	, itemData(NULL)
	, itemDataDeleteByParent(false)
{ }

QtPropertyItem::QtPropertyItem(QtPropertyData* data, QtPropertyItem *name)
	: QStandardItem()
	, itemData(data)
	, itemDataDeleteByParent(false)
{
	if(NULL != data && NULL != name)
	{
		bool hasChildren = false;
		QMapIterator<QString, QtPropertyData*> i = data->ChildIterator();

		while(i.hasNext())
		{
			i.next();

			QList<QStandardItem *> subItems;

			QtPropertyItem *subName = new QtPropertyItem(i.key());
			QtPropertyItem *subValue = new QtPropertyItem(i.value(), subName);

			subValue->itemDataDeleteByParent = true;
			subName->setEditable(false);

			subItems.append(subName);
			subItems.append(subValue);

			name->appendRow(subItems);

			hasChildren = true;
		}
		
		//setIcon(data->GetIcon());

		ApplyDataFlags();
		ApplyNameStyle(name);
	}
}

QtPropertyItem::QtPropertyItem(const QVariant &value)
	: QStandardItem()
	, itemData(NULL)
	, itemDataDeleteByParent(false)
{
	itemData = new QtPropertyData(value);
}

QtPropertyItem::~QtPropertyItem()
{
	if(NULL != itemData && !itemDataDeleteByParent)
	{
		delete itemData;
		itemData = NULL;
	}
}

QtPropertyData* QtPropertyItem::GetPropertyData() const
{
	return itemData;
}

int QtPropertyItem::type() const
{
	return QStandardItem::UserType + 1;
}

QVariant QtPropertyItem::data(int role /* = Qt::UserRole + 1 */) const
{
	QVariant v;

	if(NULL != itemData)
	{
		switch(role)
		{
		case Qt::DecorationRole:
			v = itemData->GetIcon();
			break;
		case Qt::DisplayRole:
		case Qt::EditRole:
			v = itemData->GetValue();
			break;
		default:
			break;
		}
	}
	
	if(v.isNull())
	{
		v = QStandardItem::data(role);
	}

	return v;
}

void QtPropertyItem::setData(const QVariant & value, int role /* = Qt::UserRole + 1 */)
{
	switch(role)
	{
	case Qt::EditRole:
		if(NULL != itemData)
		{
			itemData->SetValue(value);
		}
		break;
	default:
		QStandardItem::setData(value, role);
		break;
	}
}

void QtPropertyItem::ApplyNameStyle(QtPropertyItem *name)
{
	// if there are childs, set bold font
	if(name->rowCount() > 0)
	{
		QFont curFont = name->font();
		curFont.setBold(true);
		name->setFont(curFont);
	}
}

void QtPropertyItem::ApplyDataFlags()
{
	if(NULL != itemData)
	{
		int dataFlags = itemData->GetFlags();

		if(dataFlags & QtPropertyData::FLAG_IS_CHECKABLE)
		{
			setCheckable(true);
			if(itemData->GetValue().toBool())
			{
				setCheckState(Qt::Checked);
			}
		}

		if(dataFlags & QtPropertyData::FLAG_IS_DISABLED)
		{
			setEnabled(false);
		}

		if(dataFlags & QtPropertyData::FLAG_IS_NOT_EDITABLE)
		{
			setEditable(false);
		}
	}
}