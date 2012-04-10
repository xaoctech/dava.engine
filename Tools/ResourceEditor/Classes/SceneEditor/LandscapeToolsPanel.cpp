#include "LandscapeToolsPanel.h"

#include "ControlsFactory.h"
#include "LandscapeTool.h"

LandscapeToolsPanel::LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   UIControl(rect)
    ,   selectedTool(NULL)
    ,   delegate(newDelegate)
{
    ControlsFactory::CustomizeDialogFreeSpace(this);
    
    Rect closeRect(rect.dx - ControlsFactory::BUTTON_HEIGHT, 0, ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT);
    UIButton *closeButton = ControlsFactory::CreateCloseWindowButton(closeRect);
    closeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanel::OnClose));
    AddControl(closeButton);
    SafeRelease(closeButton);
    
    toolIcon = new UIControl(Rect(0, 0, ControlsFactory::TOOLS_HEIGHT, ControlsFactory::TOOLS_HEIGHT));
    toolIcon->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanel::OnSelectTool));
    AddControl(toolIcon);
    
    selectionPanel = NULL;
}


LandscapeToolsPanel::~LandscapeToolsPanel()
{
    SafeRelease(toolIcon);
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

void LandscapeToolsPanel::OnClose(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(delegate)
    {
        delegate->OnToolsPanelClose();
    }
}

void LandscapeToolsPanel::OnSelectTool(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectionPanel)
    {
        selectionPanel->Show();
    }
}

void LandscapeToolsPanel::WillAppear()
{
    if(selectionPanel)
    {
        selectedTool = selectionPanel->Tool();
        if(selectedTool)
        {
            toolIcon->SetSprite(selectedTool->sprite, 0);
        }
        else 
        {
            toolIcon->SetSprite(NULL, 0);
        }
    }
}

#pragma mark  --LandscapeToolsSelectionDelegate
void LandscapeToolsPanel::OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool)
{
    DVASSERT(newTool);
    selectedTool = newTool;
    toolIcon->SetSprite(selectedTool->sprite, 0);
    
    if(delegate)
    {
        delegate->OnToolSelected(newTool);
    }
}

