#ifndef __LANDSCAPE_TOOLS_PANEL_COLOR_H__
#define __LANDSCAPE_TOOLS_PANEL_COLOR_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeTool.h"

using namespace DAVA;

class LandscapeToolsPanelColor: public LandscapeToolsPanel
{
    
public:
    LandscapeToolsPanelColor(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanelColor();
    
    virtual void WillAppear();
    
protected:

	void OnToolSelected(BaseObject * object, void * userData, void * callerData);

    UIControl *toolButtons[LandscapeTool::EBT_COUNT_COLOR];
    LandscapeTool *tools[LandscapeTool::EBT_COUNT_COLOR];

    UISlider *intension;
    UISlider *zoom;
	void OnIntensionChanged(BaseObject * object, void * userData, void * callerData);
	void OnZoomChanged(BaseObject * object, void * userData, void * callerData);
};

#endif // __LANDSCAPE_TOOLS_PANEL_COLOR_H__