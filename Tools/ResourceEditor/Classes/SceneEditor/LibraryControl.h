#ifndef __LIBRARY_CONTROL_H__
#define __LIBRARY_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LibraryControl : public UIControl, public UIHierarchyDelegate
{
    enum eConst
    {
        CELL_HEIGHT = 20,
        BUTTON_HEIGHT = 20,
    };
    
public:
    LibraryControl(const Rect & rect);
    virtual ~LibraryControl();
    
    virtual void WillAppear();
	virtual void Update(float32 timeElapsed);

    void SetPath(const String &path);
    
protected:

    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    
    UIHierarchy * filesTree;
    
    // general
    Font *fontLight;
    Font *fontDark;
    
    UIButton *refreshButton;
    
    String folderPath;
};



#endif // __LIBRARY_CONTROL_H__