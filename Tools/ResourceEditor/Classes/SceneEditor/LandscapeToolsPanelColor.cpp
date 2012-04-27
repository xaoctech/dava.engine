#include "LandscapeToolsPanelColor.h"
#include "LandscapeTool.h"

LandscapeToolsPanelColor::LandscapeToolsPanelColor(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   LandscapeToolsPanel(newDelegate, rect)
{
    sizeSlider->SetMinMaxValue(LandscapeTool::SizeColorMin(), LandscapeTool::SizeColorMax());
    sizeSlider->SetValue((LandscapeTool::SizeColorMin() + LandscapeTool::SizeColorMax()) / 2.0f);
    
    strengthSlider->SetMinMaxValue(LandscapeTool::StrengthColorMin(), LandscapeTool::StrengthColorMax());
    strengthSlider->SetValue((LandscapeTool::StrengthColorMin() + LandscapeTool::StrengthColorMax()) / 2.0f);
}


LandscapeToolsPanelColor::~LandscapeToolsPanelColor()
{
}

