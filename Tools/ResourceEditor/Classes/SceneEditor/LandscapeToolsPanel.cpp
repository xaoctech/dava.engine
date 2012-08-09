#include "LandscapeToolsPanel.h"

#include "ControlsFactory.h"
#include "LandscapeTool.h"

LandscapeToolsPanel::LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
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
    
    strengthSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - TEXTFIELD_WIDTH, 
                                       ControlsFactory::TOOLS_HEIGHT / 2, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    strengthSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanel::OnStrengthChanged));
    AddControl(strengthSlider);
    
    AddSliderHeader(sizeSlider, LocalizedString(L"landscapeeditor.size"));
    AddSliderHeader(strengthSlider, LocalizedString(L"landscapeeditor.strength"));
}


LandscapeToolsPanel::~LandscapeToolsPanel()
{
    SafeRelease(strengthSlider);
    SafeRelease(sizeSlider);

    SafeRelease(brushIcon);
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

UICheckBox *LandscapeToolsPanel::CreateCkeckbox(const Rect &rect, const WideString &text)
{
    UICheckBox *checkbox = new UICheckBox("~res:/Gfx/UI/chekBox", rect);
    checkbox->SetDelegate(this);
    AddControl(checkbox);
    
    Rect textRect;
    textRect.x = rect.x + rect.dx + OFFSET;
    textRect.y = rect.y;
    textRect.dx = TEXT_WIDTH;
    textRect.dy = rect.dy;
    
    UIStaticText *textControl = new UIStaticText(textRect);
    textControl->SetText(text);
    textControl->SetFont(ControlsFactory::GetFontLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_LEFT);
    AddControl(textControl);
    SafeRelease(textControl);
    
    return checkbox;
}


LandscapeTool * LandscapeToolsPanel::CurrentTool()
{
    return selectedTool;
}

void LandscapeToolsPanel::SetSelectionPanel(LandscapeToolsSelection *newPanel)
{
    selectionPanel = newPanel;
    if(selectionPanel)
    {
        selectionPanel->SetDelegate(this);
    }
}


void LandscapeToolsPanel::OnBrushTool(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool == selectedBrushTool)
    {
        if(selectionPanel)
        {
            selectionPanel->Show();
        }
    }
    else 
    {
        selectedTool = selectedBrushTool;
        if(delegate)
        {
            delegate->OnToolSelected(selectedTool);
        }
    }
    
    ToolIconSelected(brushIcon);
}

void LandscapeToolsPanel::WillAppear()
{
    if(selectionPanel)
    {
        selectedBrushTool = selectionPanel->Tool();
        selectedTool = selectedBrushTool;
        if(selectedBrushTool)
        {
            brushIcon->SetSprite(selectedBrushTool->sprite, 0);
        }
        else 
        {
            brushIcon->SetSprite(NULL, 0);
        }
        ToolIconSelected(brushIcon);
    }
}

void LandscapeToolsPanel::Input(DAVA::UIEvent *currentInput)
{
    if(UIEvent::PHASE_KEYCHAR == currentInput->phase)
    { 
       if('+' == currentInput->keyChar) 
       {
           float32 sz = sizeSlider->GetValue();
           float32 maxVal = sizeSlider->GetMaxValue();
           
           sz += maxVal * 0.05f;
           sizeSlider->SetValue(Min(sz, maxVal));
           sizeSlider->PerformEvent(UIControl::EVENT_VALUE_CHANGED);
       }
       else if('-' == currentInput->keyChar)
       {
           float32 sz = sizeSlider->GetValue();
           float32 maxVal = sizeSlider->GetMaxValue();
           
           sz -= maxVal * 0.05f;
           sizeSlider->SetValue(Max(sz, sizeSlider->GetMinValue()));
           sizeSlider->PerformEvent(UIControl::EVENT_VALUE_CHANGED);
       }
    }
}

void LandscapeToolsPanel::OnStrengthChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedBrushTool)
    {
        selectedBrushTool->strength = strengthSlider->GetValue();
    }
}

void LandscapeToolsPanel::OnSizeChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedBrushTool)
    {
        selectedBrushTool->size = sizeSlider->GetValue();
    }
}

void LandscapeToolsPanel::ToolIconSelected(UIControl *focused)
{
    brushIcon->SetDebugDraw(focused == brushIcon);
}



#pragma mark  --LandscapeToolsSelectionDelegate
void LandscapeToolsPanel::OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool)
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

#pragma mark  --UICheckBoxDelegate
void LandscapeToolsPanel::ValueChanged(UICheckBox *forCheckbox, bool newValue)
{
}

