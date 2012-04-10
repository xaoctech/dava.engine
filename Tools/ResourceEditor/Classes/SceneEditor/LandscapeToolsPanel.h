#ifndef __LANDSCAPE_TOOLS_PANEL_H__
#define __LANDSCAPE_TOOLS_PANEL_H__

#include "DAVAEngine.h"
#include "LandscapeToolsSelection.h"

using namespace DAVA;

class LandscapeToolsPanelDelegate
{
public: 
    
    virtual void OnToolSelected(LandscapeTool *newTool) = 0;
    virtual void OnToolsPanelClose() = 0;
};

class LandscapeToolsPanel: 
    public UIControl,
    public LandscapeToolsSelectionDelegate

{
protected:
    
    enum eConst
    {
        OFFSET = 1,
        SLIDER_WIDTH = 250,
    };
    
public:
    LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanel();
    
    virtual void WillAppear();
    
    LandscapeTool *CurrentTool();
    void SetSelectionPanel(LandscapeToolsSelection *newPanel);
    
    //LandscapeToolsSelectionDelegate
    virtual void OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool);

protected:

    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);

    void OnClose(BaseObject * object, void * userData, void * callerData);
    void OnSelectTool(BaseObject * object, void * userData, void * callerData);

    
    LandscapeToolsPanelDelegate *delegate;
    
    UIControl *toolIcon;
    LandscapeTool *selectedTool;

    LandscapeToolsSelection *selectionPanel;
};

#endif // __LANDSCAPE_TOOLS_PANEL_H__