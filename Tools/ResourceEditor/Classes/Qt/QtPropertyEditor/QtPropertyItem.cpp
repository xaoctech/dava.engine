#include "QtPropertyEditor/QtPropertyItem.h"
#include "QtPropertyEditor/QtPropertyData.h"

QtPropertyItem::QtPropertyItem(QtPropertyData* data /*= NULL*/)
	: QStandardItem()
	, itemData(data)
{ }

QtPropertyItem::~QtPropertyItem()
{
	if(NULL != itemData)
	{
		delete itemData;
		itemData = NULL;
	}
}

void QtPropertyItem::SetPropertyData(QtPropertyData* data)
{
	if(NULL != itemData)
	{
		delete itemData;
		itemData = NULL;
	}

	data = itemData;
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

	switch(role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
		if(NULL != itemData)
		{
			v = itemData->GetValue();
		}
		break;
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
	}
}
