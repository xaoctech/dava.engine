#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include "DAVAEngine.h"
#include "GraphBase.h"

using namespace DAVA;

class SceneGraph: public GraphBase
{
    
public:
    SceneGraph(GraphBaseDelegate *newDelegate, const Rect &rect);
    virtual ~SceneGraph();
    
    virtual void SelectNode(BaseObject *node);
    virtual void RemoveWorkingNode();
    virtual void UpdatePropertyPanel();

    virtual void RefreshGraph();

    void SetSize(const Vector2 &newSize);

protected:

    void RemoveRootNodes();

    //NodesPropertyDelegate
    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
    virtual void DragAndDrop(void *who, void *target, int32 mode);

    virtual void FillCell(UIHierarchyCell *cell, void *node);
    virtual void SelectHierarchyNode(UIHierarchyNode * node);

    virtual void CreateGraphPanel(const Rect &rect);
    
	void OnRemoveRootNodesButtonPressed(BaseObject * obj, void *, void *);
	void OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *);
    void OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *);
    void OnBakeMatricesPressed(BaseObject * obj, void *, void *);
    void OnBuildQuadTreePressed(BaseObject * obj, void *, void *);
    void OnRefreshGraph(BaseObject * obj, void *, void *);

    void RecreatePropertiesPanelForNode(Entity *node);

    
    Entity * workingNode;
};



#endif // __SCENE_GRAPH_H__