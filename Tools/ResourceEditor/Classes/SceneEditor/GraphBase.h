#ifndef __GRAPH_BASE_H__
#define __GRAPH_BASE_H__

#include "DAVAEngine.h"
#include "NodesPropertyControl.h"

using namespace DAVA;

class LandscapeEditorPropertyControl;
class GraphBaseDelegate
{
public:
    virtual bool LandscapeEditorActive() = 0;
    virtual void LandscapeEditorPropertiesCreated(LandscapeEditorPropertyControl *propertyControl) = 0;
};

//class EditorScene;
#include "EditorScene.h"

class GraphBase: 
        public BaseObject, 
        public UIHierarchyDelegate, 
        public NodesPropertyDelegate
{
    
public:
    GraphBase(GraphBaseDelegate *newDelegate, const Rect &rect);
    virtual ~GraphBase();
    
    UIControl * GetGraphPanel();
    UIControl * GetPropertyPanel();
    
    void SetScene(EditorScene *scene);
    
    bool GraphOnScreen();
    bool PropertiesOnScreen();
    void UpdatePropertiesForCurrentNode();
    void RefreshProperties();
    
    virtual void SelectNode(BaseObject *node) = 0;
    virtual void UpdatePropertyPanel() = 0;
    
    virtual void RemoveWorkingNode() {};
    virtual void RefreshGraph();
    
    //NodesPropertyDelegate
    virtual void NodesPropertyChanged();
    
protected:

    //NodesPropertyDelegate
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    
    virtual void FillCell(UIHierarchyCell *cell, void *node) = 0;
    virtual void SelectHierarchyNode(UIHierarchyNode * node) = 0;


    
    void OnRefreshPropertyControl(BaseObject * object, void * userData, void * callerData);

    
    virtual void CreateGraphPanel(const Rect &rect);
    void CreatePropertyPanel(const Rect &rect);
    UIControl *graphPanel;
    UIControl *propertyPanel;

    UIHierarchy * graphTree;

    NodesPropertyControl *propertyControl;
	Rect propertyPanelRect;
    
    GraphBaseDelegate *delegate;
    EditorScene *workingScene;
};



#endif // __GRAPH_BASE_H__