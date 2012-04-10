#ifndef __LANDSCAPE_TOOLS_SELECTION_H__
#define __LANDSCAPE_TOOLS_SELECTION_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LandscapeTool;
class LandscapeToolsSelection;
class LandscapeToolsSelectionDelegate
{
public: 
    virtual void OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool) = 0;
};

class LandscapeToolsSelection: public UIControl, public UIListDelegate
{
    
public:
    LandscapeToolsSelection(LandscapeToolsSelectionDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsSelection();

    virtual void WillAppear();
    
    LandscapeTool *Tool();
    
    void SetDelegate(LandscapeToolsSelectionDelegate *newDelegate);

    void SetBodyControl(UIControl *parent);

    void Show();
    void Close();
    
    //UIListDelegate
    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);

    
protected:
    
    void OnToolSelected(BaseObject * object, void * userData, void * callerData);
    void OnClose(BaseObject * object, void * userData, void * callerData);
    
    UIControl *GetToolControl(int32 indexAtRow, UIListCell *cell);

    void EnumerateTools();
    void ReleaseTools();

    
    UIControl *parentBodyControl;

    LandscapeTool *selectedTool;
    Vector<LandscapeTool *>tools;
    
    UIList *toolsList;
    
    LandscapeToolsSelectionDelegate *delegate;
};

#endif // __LANDSCAPE_TOOLS_SELECTION_H__