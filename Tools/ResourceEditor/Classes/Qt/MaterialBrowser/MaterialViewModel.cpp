#include "MaterialBrowser/MaterialViewModel.h"

MaterialViewModel::MaterialViewModel(QObject *parent /* = 0 */)
	: treeItem(NULL)
{

}

int MaterialViewModel::rowCount(const QModelIndex &parent /* = QModelIndex */) const
{
	if(NULL != treeItem)
	{
		return treeItem->ChildCount();
	}

	return 0;
}

QVariant MaterialViewModel::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const
{
	QVariant v;

	if(index.isValid())
	{
		MaterialTreeItem *item = treeItem->Child(index.row());
		if(NULL != item)
		{
			switch(role)
			{
			case Qt::DisplayRole:
				v = item->Data(0);
				break;
			case Qt::DecorationRole:
				break;
			}
		}
	}

	return v;
}

void MaterialViewModel::SetTreeItem(MaterialTreeItem* item)
{
	DAVA::SafeRelease(treeItem);
	treeItem = item;
	DAVA::SafeRetain(treeItem);
}