#include "LandscapeToolsPanelColor.h"

#include "ControlsFactory.h"
#include "LandscapeTool.h"

#pragma mark  --LandscapeToolsPanelColor
LandscapeToolsPanelColor::LandscapeToolsPanelColor(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   LandscapeToolsPanel(newDelegate, rect)
{
    zoom = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - ControlsFactory::BUTTON_HEIGHT, 0, 
                               SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    zoom->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanelColor::OnZoomChanged));
    zoom->SetMinMaxValue(LandscapeTool::ZoomMin(), LandscapeTool::ZoomMax());
    zoom->SetValue((LandscapeTool::ZoomMin() + LandscapeTool::ZoomMax()) / 2.0f);
    
    
    intension = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - ControlsFactory::BUTTON_HEIGHT, ControlsFactory::TOOLS_HEIGHT / 2, 
                                  SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    intension->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanelColor::OnIntensionChanged));
    intension->SetMinMaxValue(LandscapeTool::IntensionMin(), LandscapeTool::IntensionMax());
    intension->SetValue((LandscapeTool::IntensionMin() + LandscapeTool::IntensionMax()) / 2.0f);
    
    AddControl(intension);
    AddControl(zoom);
    
    AddSliderHeader(zoom, LocalizedString(L"landscapeeditor.zoom"));
    AddSliderHeader(intension, LocalizedString(L"landscapeeditor.intension"));
}


LandscapeToolsPanelColor::~LandscapeToolsPanelColor()
{
    SafeRelease(intension);
    SafeRelease(zoom);
}

void LandscapeToolsPanelColor::OnIntensionChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->intension = intension->GetValue();
    }
}

void LandscapeToolsPanelColor::OnZoomChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->zoom = zoom->GetValue();
    }
}


#pragma mark  --LandscapeToolsSelectionDelegate
void LandscapeToolsPanelColor::OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool)
{
    newTool->intension = intension->GetValue();
    newTool->zoom = zoom->GetValue();
    
    LandscapeToolsPanel::OnToolSelected(forControl, newTool);
}

