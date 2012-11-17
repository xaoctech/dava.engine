#ifndef __LANDSCAPE_TOOLS_PANEL_CUSTOM_COLOR_H__
#define __LANDSCAPE_TOOLS_PANEL_CUSTOM_COLOR_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"


using namespace DAVA;

class LandscapeTool;
class LandscapeToolsPanelCustomColors: public LandscapeToolsPanel
{
    
public:
	LandscapeToolsPanelCustomColors(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);

	virtual void SetSize(const Vector2 &newSize);

	void SetSelectionPanel(LandscapeToolsSelection *newPanel);
};

#endif // __LANDSCAPE_TOOLS_PANEL_CUSTOM_COLOR_H__