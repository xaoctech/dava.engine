#include "LandscapeToolsPanelColor.h"

#include "ControlsFactory.h"

LandscapeToolsPanelColor::LandscapeToolsPanelColor(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   LandscapeToolsPanel(newDelegate, rect)
{
    const String sprites[] = 
    {
        "~res:/Gfx/LandscapeEditor/Tools/RadialGradientIcon",
        "~res:/Gfx/LandscapeEditor/Tools/SpikeGradientIcon",
        "~res:/Gfx/LandscapeEditor/Tools/CircleIcon",
        "~res:/Gfx/LandscapeEditor/Tools/NoiseIcon",
        "~res:/Gfx/LandscapeEditor/Tools/ErodeIcon",
        "~res:/Gfx/LandscapeEditor/Tools/WaterErodeIcon",
    };
    
    int32 x = 0;
    int32 y = (ControlsFactory::TOOLS_HEIGHT - ControlsFactory::TOOL_BUTTON_SIDE) / 2;
    for(int32 i = 0; i < LandscapeTool::EBT_COUNT_COLOR; ++i, x += (ControlsFactory::TOOL_BUTTON_SIDE + OFFSET))
    {
        tools[i] = new LandscapeTool((LandscapeTool::eBrushType) i, sprites[i], "");
        
        toolButtons[i] = new UIControl(Rect(x, y, ControlsFactory::TOOL_BUTTON_SIDE, ControlsFactory::TOOL_BUTTON_SIDE));
        toolButtons[i]->SetSprite(tools[i]->spriteName, 0);
        toolButtons[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanelColor::OnToolSelected));
        
        AddControl(toolButtons[i]);
    }
    
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
    
    for(int32 i = 0; i < LandscapeTool::EBT_COUNT_COLOR; ++i)
    {
        SafeRelease(toolButtons[i]);
        SafeRelease(tools[i]);
    }
}


void LandscapeToolsPanelColor::WillAppear()
{
    if(!selectedTool)
    {
        toolButtons[0]->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
    }
    
    UIControl::WillAppear();
}

void LandscapeToolsPanelColor::OnToolSelected(DAVA::BaseObject *object, void *userData, void *callerData)
{
    UIControl *button = (UIControl *)object;
    
    for(int32 i = 0; i < LandscapeTool::EBT_COUNT_COLOR; ++i)
    {
        if(button == toolButtons[i])
        {
            selectedTool = tools[i];
            toolButtons[i]->SetDebugDraw(true);
            
            intension->SetValue(selectedTool->intension);
            zoom->SetValue(selectedTool->zoom);
        }
        else
        {
            toolButtons[i]->SetDebugDraw(false);
        }
    }
    
    if(delegate)
    {
        delegate->OnToolSelected(selectedTool);
    }
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

