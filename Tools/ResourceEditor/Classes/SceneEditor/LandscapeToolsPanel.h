#ifndef __LANDSCAPE_TOOLS_PANEL_H__
#define __LANDSCAPE_TOOLS_PANEL_H__

#include "DAVAEngine.h"
#include "PaintTool.h"

using namespace DAVA;

class LandscapeToolsPanelDelegate
{
public: 
    
    virtual void OnToolSelected(PaintTool *newTool) = 0;
};

class PaintTool;
class UICheckBox;
class LandscapeToolsPanel: public UIControl
{
    enum eConst
    {
        OFFSET = 1,
        SLIDER_WIDTH = 100,
    };
    
public:
    LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanel();
    
    virtual void WillAppear();
    
    PaintTool *CurrentTool();
//    bool StrightDrawing();

protected:

	void OnToolSelected(BaseObject * object, void * userData, void * callerData);

    UIControl *toolButtons[PaintTool::EBT_COUNT];
    PaintTool *tools[PaintTool::EBT_COUNT];
    PaintTool *selectedTool;

    UISlider *radius;
    UISlider *intension;
    UISlider *zoom;
    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);
	void OnRadiusChanged(BaseObject * object, void * userData, void * callerData);
	void OnIntensionChanged(BaseObject * object, void * userData, void * callerData);
	void OnZoomChanged(BaseObject * object, void * userData, void * callerData);

//    UICheckBox *strightDrawing;
    
    LandscapeToolsPanelDelegate *delegate;
};

#endif // __LANDSCAPE_TOOLS_PANEL_H__