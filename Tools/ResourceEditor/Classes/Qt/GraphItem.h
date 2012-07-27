#ifndef __GRAPH_ITEM_H__
#define __GRAPH_ITEM_H__

#include "DAVAEngine.h"
#include <QVariant>

using namespace DAVA;

class GraphItem: public BaseObject
{
public:
    GraphItem(const String &text, GraphItem *parent = 0);
    virtual ~GraphItem();
    
	GraphItem *GetParent();
	GraphItem *Child(int32 row);
    void AppendChild(GraphItem *child);
	int32 ChildrenCount() const;

	int32 Row() const;
    int32 ColumnCount() const;

	QVariant Data(int32 column) const;

private:

	Vector<GraphItem *>children;

	String itemText;
    GraphItem *parentItem;
};

#endif // __GRAPH_ITEM_H__
