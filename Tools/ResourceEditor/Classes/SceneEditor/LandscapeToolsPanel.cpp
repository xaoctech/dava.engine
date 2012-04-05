#include "LandscapeToolsPanel.h"

#include "ControlsFactory.h"
#include "EditorSettings.h"

#include "UICheckBox.h"


LandscapeToolsPanel::LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   UIControl(rect)
    ,   selectedTool(NULL)
    ,   delegate(newDelegate)
{
    ControlsFactory::CustomizeDialogFreeSpace(this);
    
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
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i, x += (ControlsFactory::TOOL_BUTTON_SIDE + OFFSET))
    {
        tools[i] = new PaintTool((PaintTool::eBrushType) i, sprites[i]);
        
        toolButtons[i] = new UIControl(Rect(x, y, ControlsFactory::TOOL_BUTTON_SIDE, ControlsFactory::TOOL_BUTTON_SIDE));
        toolButtons[i]->SetSprite(tools[i]->spriteName, 0);
        toolButtons[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanel::OnToolSelected));
        
        AddControl(toolButtons[i]);
    }
    
    zoom = CreateSlider(Rect(rect.dx - SLIDER_WIDTH, 0, 
                               SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    zoom->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanel::OnZoomChanged));
    zoom->SetMinMaxValue(PaintTool::ZoomMin(), PaintTool::ZoomMax());
    zoom->SetValue((PaintTool::ZoomMin() + PaintTool::ZoomMax()) / 2.0f);
    
    
    intension = CreateSlider(Rect(rect.dx - SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2, 
                                  SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    intension->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanel::OnIntensionChanged));
    intension->SetMinMaxValue(PaintTool::IntensionMin(), PaintTool::IntensionMax());
    intension->SetValue((PaintTool::IntensionMin() + PaintTool::IntensionMax()) / 2.0f);
    
    AddControl(intension);
    AddControl(zoom);
    
    AddSliderHeader(zoom, LocalizedString(L"landscapeeditor.zoom"));
    AddSliderHeader(intension, LocalizedString(L"landscapeeditor.intension"));
}


LandscapeToolsPanel::~LandscapeToolsPanel()
{
    SafeRelease(intension);
    SafeRelease(zoom);
    
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i)
    {
        SafeRelease(toolButtons[i]);
        SafeRelease(tools[i]);
    }
}


UISlider * LandscapeToolsPanel::CreateSlider(const Rect & rect)
{
    UISlider *slider = new UISlider(rect);
    slider->SetMinMaxValue(0.f, 1.0f);
    slider->SetValue(0.5f);
    
    slider->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    slider->SetMinDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMinLeftRightStretchCap(5);

    slider->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    slider->SetMaxDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMaxLeftRightStretchCap(5);

    slider->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    
    return slider;
}

void LandscapeToolsPanel::AddSliderHeader(UISlider *slider, const WideString &text)
{
    Rect rect = slider->GetRect();
    rect.x -= rect.dx - OFFSET;
    
    UIStaticText *textControl = new UIStaticText(rect);
    textControl->SetText(text);
    textControl->SetFont(ControlsFactory::GetFontLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_RIGHT);
    AddControl(textControl);
    SafeRelease(textControl);
}


void LandscapeToolsPanel::WillAppear()
{
    if(!selectedTool)
    {
        toolButtons[0]->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
    }
    
    UIControl::WillAppear();
}

void LandscapeToolsPanel::OnToolSelected(DAVA::BaseObject *object, void *userData, void *callerData)
{
    UIControl *button = (UIControl *)object;
    
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i)
    {
        if(button == toolButtons[i])
        {
            selectedTool = tools[i];
            toolButtons[i]->SetDebugDraw(true);
            
            intension->SetValue(selectedTool->intension);
            zoom->SetValue(selectedTool->zoom);
            
//            selectedTool->zoom = zoom->GetValue();
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

void LandscapeToolsPanel::OnIntensionChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->intension = intension->GetValue();
    }
}

void LandscapeToolsPanel::OnZoomChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->zoom = zoom->GetValue();
    }
}

PaintTool * LandscapeToolsPanel::CurrentTool()
{
    return selectedTool;
}

