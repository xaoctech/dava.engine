#ifndef __GRAPH_ITEM_H__
#define __GRAPH_ITEM_H__

#include "DAVAEngine.h"
#include <QVariant>

using namespace DAVA;

class GraphItem: public BaseObject
{
public:
    GraphItem(GraphItem *parent = 0);
    virtual ~GraphItem();
    
	GraphItem *GetParent();
    void SetParent(GraphItem * parent);

	virtual void SetUserData(void *data) = 0;
    void * GetUserData();

	GraphItem *Child(int32 row);
    void AppendChild(GraphItem *child);
	int32 ChildrenCount() const;

	int32 Row() const;
    int32 ColumnCount() const;

	virtual QVariant Data(int32 column) = 0;

protected:

	virtual void ReleaseUserData() = 0;

protected:
	void *userData;

private:
	Vector<GraphItem *>children;
    GraphItem *parentItem;
};

#endif // __GRAPH_ITEM_H__
