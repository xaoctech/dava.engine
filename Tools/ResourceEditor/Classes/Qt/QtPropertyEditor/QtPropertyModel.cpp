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
}

QtPropertyModel::~QtPropertyModel()
{ }

QtPropertyItem* QtPropertyModel::AppendPropertyHeader(const QString &name, QtPropertyItem* parent /*= NULL*/)
{
	QList<QStandardItem *> items;
	QStandardItem* root = (QStandardItem *) parent;
	
	QtPropertyItem *newPropertyName = new QtPropertyItem(name);
	QtPropertyItem *newPropertyValue = new QtPropertyItem();

	QFont headerFont = newPropertyName->font();
	headerFont.setBold(true);

	newPropertyName->setText(name);
	newPropertyName->setFont(headerFont);
	newPropertyName->setEditable(false);
	newPropertyName->setSelectable(false);
	newPropertyName->setBackground(QBrush(Qt::lightGray));

	newPropertyValue->setEditable(false);
	newPropertyValue->setSelectable(false);
	newPropertyValue->setBackground(QBrush(Qt::lightGray));

	items.append(newPropertyName);
	items.append(newPropertyValue);

	if(NULL == root)
	{
		root = invisibleRootItem();
	}

	root->appendRow(items);

	return newPropertyName;
}

QtPropertyItem* QtPropertyModel::AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent /*= NULL*/)
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

	return newPropertyName;
}

void QtPropertyModel::RemoveProperty(QtPropertyItem* item)
{
	removeRow(indexFromItem(item).row());
}

void QtPropertyModel::RemovePropertyAll()
{
	removeRows(0, rowCount());
}
