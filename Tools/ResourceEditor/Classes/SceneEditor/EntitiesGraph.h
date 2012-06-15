#ifndef __ENTITIES_GRAPH_H__
#define __ENTITIES_GRAPH_H__

#include "DAVAEngine.h"
#include "GraphBase.h"

using namespace DAVA;

class EntitiesGraph: public GraphBase
{
public:
	EntitiesGraph(GraphBaseDelegate *newDelegate, const Rect &rect);
	virtual ~EntitiesGraph();

	virtual void SelectNode(BaseObject *node);
	virtual void UpdatePropertyPanel();

protected:
	virtual void SelectHierarchyNode(UIHierarchyNode * node);

	virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode) {return false;}
	virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
	virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
	virtual void FillCell(UIHierarchyCell *cell, void *node);

	virtual void CreateGraphPanel(const Rect &rect);

	Entity * workingEntity;
};



#endif // __ENTITIES_GRAPH_H__