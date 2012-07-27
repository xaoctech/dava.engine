#include "GraphModel.h"
#include "GraphItem.h"


GraphModel::GraphModel(QObject *parent)
    : QAbstractItemModel(parent)
{
	rootItem = new GraphItem(String("Graph Title"));	//TODO: create virtual function for Title
    setupModelData(rootItem);
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

void GraphModel::setupModelData(GraphItem *parent)
{
    GraphItem *item1 = new GraphItem(String("TopLevel"), parent);
    parent->AppendChild(item1);

    GraphItem *item12 = new GraphItem(String("Child1"), item1);
    item1->AppendChild(item12);

    GraphItem *item13 = new GraphItem(String("Child2"), item1);
    item1->AppendChild(item13);

    GraphItem *item14 = new GraphItem(String("Child3"), item1);
    item1->AppendChild(item14);
}


