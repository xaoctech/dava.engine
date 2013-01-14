#include "QtPropertyEditor/QtPropertyModel.h"
#include "QtPropertyEditor/QtPropertyItem.h"

QtPropertyModel::QtPropertyModel(QObject* parent /* = 0 */)
	: QStandardItemModel(parent)
{
	QStringList headerLabels;

	headerLabels.append("Name");
	headerLabels.append("Value");

	setColumnCount(2);
	setHorizontalHeaderLabels(headerLabels);

	QObject::connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(ItemChanged(QStandardItem *)));
}

QtPropertyModel::~QtPropertyModel()
{ }

QPair<QtPropertyItem*, QtPropertyItem*> QtPropertyModel::AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent /*= NULL*/)
{
	QList<QStandardItem *> items;
	QStandardItem* root = (QStandardItem *) parent;

	QtPropertyItem *newPropertyName = new QtPropertyItem(name);
	QtPropertyItem *newPropertyValue = new QtPropertyItem(data, newPropertyName);

	newPropertyName->setEditable(false);

	items.append(newPropertyName);
	items.append(newPropertyValue);

	if(NULL == root)
	{
		root = invisibleRootItem();
	}

	root->appendRow(items);

	return QPair<QtPropertyItem*, QtPropertyItem*>(newPropertyName, newPropertyValue);
}

void QtPropertyModel::RemoveProperty(QtPropertyItem* item)
{
	removeRow(indexFromItem(item).row());
}

void QtPropertyModel::RemovePropertyAll()
{
	removeRows(0, rowCount());
}

void QtPropertyModel::ItemChanged(QStandardItem* item)
{
	QtPropertyItem *propItem = (QtPropertyItem *) item;

	if(NULL != propItem)
	{
		if(propItem->isCheckable())
		{
			propItem->GetPropertyData()->SetValue(QVariant(propItem->checkState() == Qt::Checked));
		}
	}
}