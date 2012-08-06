#include "GraphItem.h"

GraphItem::GraphItem(GraphItem *parent)
	:	parentItem(parent)
{
}

GraphItem::~GraphItem()
{
	int32 count = (int32)children.size();
	for(int32 i = 0; i < count; ++i)
	{
		SafeRelease(children[i]);
	}
	children.clear();
}

void GraphItem::AppendChild(GraphItem *item)
{
	children.push_back(item);
}

GraphItem *GraphItem::Child(int32 row)
{
    return children[row];
}

int32 GraphItem::ChildrenCount() const
{
    return (int32)children.size();
}

int32 GraphItem::Row() const
{
    if (parentItem)
	{
		int32 count = parentItem->ChildrenCount();
		for(int32 i = 0; i < count; ++i)
		{
			if(parentItem->Child(i) == this)
			{
				return i;
			}
		}
	}

    return 0;
}

int32 GraphItem::ColumnCount() const
{
    return 1;
}

GraphItem *GraphItem::GetParent()
{
    return parentItem;
}
