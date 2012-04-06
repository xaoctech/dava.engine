#ifndef __LANDSCAPE_TOOLS_PANEL_H__
#define __LANDSCAPE_TOOLS_PANEL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LandscapeTool;
class LandscapeToolsPanelDelegate
{
public: 
    
    virtual void OnToolSelected(LandscapeTool *newTool) = 0;
    virtual void OnToolsPanelClose() = 0;
};

class LandscapeToolsPanel: public UIControl
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
    
    LandscapeTool *CurrentTool();

protected:

    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);

    LandscapeToolsPanelDelegate *delegate;
    LandscapeTool *selectedTool;
    
    void OnClose(BaseObject * object, void * userData, void * callerData);
    
};

#endif // __LANDSCAPE_TOOLS_PANEL_H__