#include "LandscapeToolsPanelCustomColors.h"
#include "LandscapeTool.h"


LandscapeToolsPanelCustomColors::LandscapeToolsPanelCustomColors(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   UIControl(rect)
    ,   delegate(newDelegate)
    ,   selectedTool(NULL)
    ,   selectedBrushTool(NULL)
{
    ControlsFactory::CustomizeDialogFreeSpace(this);
    
    selectionPanel = NULL;

    brushIcon = new UIControl(Rect(0, 0, ControlsFactory::TOOLS_HEIGHT, ControlsFactory::TOOLS_HEIGHT));
    brushIcon->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanel::OnBrushTool));
    brushIcon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    AddControl(brushIcon);
    
    
    sizeSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - TEXTFIELD_WIDTH,
                                   0, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    sizeSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanel::OnSizeChanged));
    AddControl(sizeSlider);

	

    AddSliderHeader(sizeSlider, LocalizedString(L"landscapeeditor.size"));
}


LandscapeToolsPanelCustomColors::~LandscapeToolsPanelCustomColors()
{
    SafeRelease(sizeSlider);

    SafeRelease(brushIcon);
}

void LandscapeToolsPanelCustomColors::OnToolSelected(LandscapeToolsSelection * , LandscapeTool *newTool)
{
    DVASSERT(newTool);
    
    newTool->size = sizeSlider->GetValue();
    newTool->strength = strengthSlider->GetValue();

    
    selectedBrushTool = newTool;
    selectedTool = selectedBrushTool;
    brushIcon->SetSprite(selectedBrushTool->sprite, 0);
    
    if(delegate)
    {
        delegate->OnToolSelected(newTool);
    }
}

void LandscapeToolsPanelCustomColors::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);
    
    sizeSlider->SetPosition(Vector2(newSize.x - SLIDER_WIDTH - TEXTFIELD_WIDTH, 0));
    strengthSlider->SetPosition(Vector2(newSize.x - SLIDER_WIDTH - TEXTFIELD_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));

    SetSliderHeaderPoition(sizeSlider, LocalizedString(L"landscapeeditor.size"));
    SetSliderHeaderPoition(strengthSlider, LocalizedString(L"landscapeeditor.strength"));
}
