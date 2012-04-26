#ifndef __LANDSCAPE_TOOLS_PANEL_COLOR_H__
#define __LANDSCAPE_TOOLS_PANEL_COLOR_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"

using namespace DAVA;

class LandscapeTool;
class LandscapeToolsPanelColor: public LandscapeToolsPanel
{
    
public:
    LandscapeToolsPanelColor(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanelColor();
};

#endif // __LANDSCAPE_TOOLS_PANEL_COLOR_H__