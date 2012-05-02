#ifndef __DATA_GRAPH_H__
#define __DATA_GRAPH_H__

#include "DAVAEngine.h"
#include "GraphBase.h"

using namespace DAVA;

class DataGraph: public GraphBase
{
    
public:
    DataGraph(GraphBaseDelegate *newDelegate, const Rect &rect);
    virtual ~DataGraph();
    
    virtual void SelectNode(BaseObject *node);
    virtual void UpdatePropertyPanel();
    
    virtual void RefreshGraph();

protected:

    //NodesPropertyDelegate
    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);

    virtual void FillCell(UIHierarchyCell *cell, void *node);
    virtual void SelectHierarchyNode(UIHierarchyNode * node);

    virtual void CreateGraphPanel(const Rect &rect);
    
    void OnRefreshGraph(BaseObject * obj, void *, void *);
    void RecreatePropertiesPanelForNode(DataNode *node);
    
    DataNode * workingNode;
    Set<DataNode *> dataNodes;
};



#endif // __DATA_GRAPH_H__