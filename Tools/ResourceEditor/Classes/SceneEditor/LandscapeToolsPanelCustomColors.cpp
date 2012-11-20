#include "LandscapeToolsPanelCustomColors.h"
#include "ControlsFactory.h"
#include "LandscapeTool.h"
#include "Base/Message.h"

LandscapeToolsPanelCustomColors::LandscapeToolsPanelCustomColors(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :LandscapeToolsPanel(rect)
{
    delegate = newDelegate;
    selectedTool = NULL;
    selectedBrushTool = NULL;
	
    selectionPanel = NULL;

	brushIcon = new UIControl(Rect(0, 0, ControlsFactory::TOOLS_HEIGHT, ControlsFactory::TOOLS_HEIGHT));
	brushIcon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);

	sizeSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - TEXTFIELD_WIDTH,
                                   0, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    
    strengthSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - TEXTFIELD_WIDTH, 
                                       ControlsFactory::TOOLS_HEIGHT / 2, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));

}

void LandscapeToolsPanelCustomColors::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);
}

void LandscapeToolsPanelCustomColors::SetSelectionPanel(LandscapeToolsSelection *newPanel)
{
    
	LandscapeToolsPanel::SetSelectionPanel(newPanel);
	selectionPanel->SetVisible(false);
}

