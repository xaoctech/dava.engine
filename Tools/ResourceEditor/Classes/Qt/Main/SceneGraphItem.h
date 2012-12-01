#ifndef __SCENE_GRAPH_ITEM_H__
#define __SCENE_GRAPH_ITEM_H__

#include "GraphItem.h"

class SceneGraphItem: public GraphItem
{
public:
    SceneGraphItem(GraphItem *parent = 0);
    virtual ~SceneGraphItem();
    
	virtual QVariant Data(int32 column);
	virtual void SetUserData(void *data);

protected:
	virtual void ReleaseUserData();
};

#endif // __SCENE_GRAPH_ITEM_H__
