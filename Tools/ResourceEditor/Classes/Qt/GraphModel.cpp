#include "GraphModel.h"
#include "GraphItem.h"


GraphModel::GraphModel(QObject *parent)
    : QAbstractItemModel(parent)
{
	rootItem = NULL;
}

GraphModel::~GraphModel()
{
	SafeRelease(rootItem);
}

QModelIndex GraphModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

    GraphItem *parentItem = NULL;
	if (!parent.isValid())
	{
		parentItem = rootItem;
	}
    else
	{
		parentItem = static_cast<GraphItem*>(parent.internalPointer());
	}

    GraphItem *childItem = parentItem->Child(row);
    if (childItem)
	{
		return createIndex(row, column, childItem);
	}

	return QModelIndex();
}

QModelIndex GraphModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
	{
		return QModelIndex();
	}

    GraphItem *childItem = static_cast<GraphItem*>(index.internalPointer());
    GraphItem *parentItem = childItem->GetParent();

    if (parentItem == rootItem)
	{
		return QModelIndex();
	}

    return createIndex(parentItem->Row(), 0, parentItem);
}

int GraphModel::rowCount(const QModelIndex &parent) const
{
    GraphItem *parentItem;
    if (parent.column() > 0)
	{
		return 0;
	}

    if (!parent.isValid())
	{
		parentItem = rootItem;
	}
    else
	{
		parentItem = static_cast<GraphItem*>(parent.internalPointer());
	}

    return parentItem->ChildrenCount();
}

int GraphModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
	{
		return static_cast<GraphItem*>(parent.internalPointer())->ColumnCount();
	}

	return rootItem->ColumnCount();
}

QVariant GraphModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
	{
		return QVariant();
	}

    if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

    GraphItem *item = static_cast<GraphItem*>(index.internalPointer());
    return item->Data(index.column());
}

Qt::ItemFlags GraphModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
	{
		return 0;
	}

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant GraphModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		return rootItem->Data(section);
	}

    return QVariant();
}

