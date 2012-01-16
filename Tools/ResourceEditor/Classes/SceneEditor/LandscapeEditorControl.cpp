#include "LandscapeEditorControl.h"

#include "ControlsFactory.h"

#include "EditorSettings.h"

#include "PaintAreaControl.h"

//***************    LandscapeEditorControl    **********************
LandscapeEditorControl::LandscapeEditorControl(const Rect & rect)
    :   UIControl(rect)
{
    ControlsFactory::CusomizeBottomLevelControl(this);
    
    CreateLeftPanel();
    CreateRightPanel();
    CreatePaintAreaPanel();
}


LandscapeEditorControl::~LandscapeEditorControl()
{
    ReleasePaintAreaPanel();
    ReleaseRightPanel();
    ReleaseLeftPanel();
}


void LandscapeEditorControl::CreateLeftPanel()
{
    Rect fullRect = GetRect();
    
    Rect leftRect = Rect(0, 0, ControlsFactory::LEFT_SIDE_WIDTH, fullRect.dy);
    leftPanel = ControlsFactory::CreatePanelControl(leftRect);
    AddControl(leftPanel);
    
    propertyList = new PropertyList(Rect(0, 0, leftRect.dx, leftRect.dy), this);
    leftPanel->AddControl(propertyList);
    
    propertyList->AddIntProperty("Size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetIntPropertyValue("Size", 1024);
    
//    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", "", ".png", PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", "");
//    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", "", ".png", PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", "");

    String projectPath = EditorSettings::Instance()->GetDataSourcePath();
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath + "Data/Landscape/tex3.png");
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath + "Data/Landscape/detail_gravel.png");
    
    propertyList->AddBoolProperty("Show Result", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetBoolPropertyValue("Show Result", true);
}

void LandscapeEditorControl::ReleaseLeftPanel()
{
    SafeRelease(propertyList);
    SafeRelease(leftPanel);
}

void LandscapeEditorControl::CreateRightPanel()
{
    Rect fullRect = GetRect();
    
    Rect rightRect = Rect(fullRect.dx - ControlsFactory::RIGHT_SIDE_WIDTH, 0, ControlsFactory::RIGHT_SIDE_WIDTH, fullRect.dy);
    rightPanel = ControlsFactory::CreatePanelControl(rightRect);
    AddControl(rightPanel);
}

void LandscapeEditorControl::ReleaseRightPanel()
{
    SafeRelease(rightPanel);
}

void LandscapeEditorControl::CreatePaintAreaPanel()
{
    Rect fullRect = GetRect();
    
    Rect toolsRect = Rect(ControlsFactory::LEFT_SIDE_WIDTH + OFFSET, 0, 
                          fullRect.dx - ControlsFactory::LEFT_SIDE_WIDTH - ControlsFactory::RIGHT_SIDE_WIDTH - OFFSET*2, TOOLS_HEIGHT);
    toolsPanel = ControlsFactory::CreatePanelControl(toolsRect);
    AddControl(toolsPanel);
    
    selectedTool = NULL;
    
    const String sprites[] = 
    {
        "~res:/Gfx/LandscapeEditor/Tools/RadialGradientIcon",
        "~res:/Gfx/LandscapeEditor/Tools/SpikeGradientIcon",
        "~res:/Gfx/LandscapeEditor/Tools/CircleIcon",
        "~res:/Gfx/LandscapeEditor/Tools/NoiseIcon",
        "~res:/Gfx/LandscapeEditor/Tools/ErodeIcon",
        "~res:/Gfx/LandscapeEditor/Tools/WaterErodeIcon",
    };

    const float32 actualRadius[] = 
    {
        0.5f,
        0.35f,
        0.7f,
        0.5f,
        0.6f,
        0.6f,
    };

    
    int32 x = 0;
    int32 y = (TOOLS_HEIGHT - TOOL_BUTTON_SIDE) / 2;
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i, x += (TOOL_BUTTON_SIDE + 1))
    {
        tools[i] = new PaintTool((PaintTool::eBrushType) i, sprites[i], actualRadius[i]);
        
        toolButtons[i] = new UIControl(Rect(x, y, TOOL_BUTTON_SIDE, TOOL_BUTTON_SIDE));
        toolButtons[i]->SetSprite(tools[i]->spriteName, 0);
        toolButtons[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeEditorControl::OnToolSelected));

        toolsPanel->AddControl(toolButtons[i]);
    }
    
    radius = CreateSlider(Rect(toolsRect.dx - SLIDER_WIDTH, 0, SLIDER_WIDTH, TOOLS_HEIGHT / 2));
    radius->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeEditorControl::OnRadiusChanged));
    height = CreateSlider(Rect(toolsRect.dx - SLIDER_WIDTH, TOOLS_HEIGHT / 2, SLIDER_WIDTH, TOOLS_HEIGHT / 2));
    height->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeEditorControl::OnHeightChanged));
    
    Rect zoomRect = radius->GetRect();
    zoomRect.x -= 2 * zoomRect.dx;
    zoom = CreateSlider(zoomRect);
    zoom->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeEditorControl::OnZoomChanged));
    
    toolsPanel->AddControl(radius);
    toolsPanel->AddControl(height);
    toolsPanel->AddControl(zoom);
    
    AddSliderHeader(zoom, L"Zoom:");
    AddSliderHeader(radius, L"Radius:");
    AddSliderHeader(height, L"Height:");
    

    Rect paintRect = Rect(toolsRect.x, toolsRect.y + toolsRect.dy + OFFSET, 
                          toolsRect.dx, fullRect.dy - (toolsRect.y + toolsRect.dy + OFFSET*2));
    
    scrollView = new UIScrollView(paintRect);
    scrollView->SetContentSize(Vector2(1024, 1024));
    AddControl(scrollView);
    
    paintArea = new PaintAreaControl(Rect(0, 0, 1024, 1024));
    scrollView->AddControl(paintArea);
}

void LandscapeEditorControl::ReleasePaintAreaPanel()
{
    SafeRelease(zoom);
    SafeRelease(scrollView);
    SafeRelease(radius);
    SafeRelease(height);
    
    
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i)
    {
        SafeRelease(toolButtons[i]);
        SafeRelease(tools[i]);
    }
    
    SafeRelease(paintArea);
    SafeRelease(toolsPanel);
}

UISlider * LandscapeEditorControl::CreateSlider(const Rect & rect)
{
    UISlider *slider = new UISlider(rect);
    slider->SetMinMaxValue(0.f, 1.0f);
    slider->SetValue(0.5f);
    
    slider->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    slider->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    slider->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    
    return slider;
}

void LandscapeEditorControl::AddSliderHeader(UISlider *slider, const WideString &text)
{
    Rect rect = slider->GetRect();
    rect.x -= rect.dx - OFFSET;
    
    UIStaticText *textControl = new UIStaticText(rect);
    textControl->SetText(text);
    textControl->SetFont(ControlsFactory::GetFontDark());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_RIGHT);
    toolsPanel->AddControl(textControl);
    SafeRelease(textControl);
}


void LandscapeEditorControl::WillAppear()
{
    scrollView->SetScale(zoom->GetValue());

    paintArea->SetTexture(PaintAreaControl::ET_TEXTURE0, propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE0"));
    paintArea->SetTexture(PaintAreaControl::ET_TEXTURE1, propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL"));

    paintArea->ShowResultTexture(propertyList->GetBoolPropertyValue("Show Result"));

    
    if(!selectedTool)
    {
        toolButtons[0]->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
    }
    
    
    UIControl::WillAppear();
}

void LandscapeEditorControl::OnToolSelected(DAVA::BaseObject *object, void *userData, void *callerData)
{
    UIControl *button = (UIControl *)object;
    
    for(int32 i = 0; i < PaintTool::EBT_COUNT; ++i)
    {
        if(button == toolButtons[i])
        {
            selectedTool = tools[i];
            toolButtons[i]->SetDebugDraw(true);
            
            radius->SetValue(selectedTool->radius);
            height->SetValue(selectedTool->height);
            
            selectedTool->zoom = zoom->GetValue();
            
            paintArea->SetPaintTool(selectedTool);
        }
        else
        {
            toolButtons[i]->SetDebugDraw(false);
        }
    }
}

void LandscapeEditorControl::OnRadiusChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->radius = radius->GetValue();
    }
}

void LandscapeEditorControl::OnHeightChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->height = height->GetValue();
    }
}

void LandscapeEditorControl::OnZoomChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    scrollView->SetScale(zoom->GetValue());
    if(selectedTool)
    {
        selectedTool->zoom = zoom->GetValue();
    }
}



void LandscapeEditorControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    if("Size" == forKey)
    {
        Vector2 texSize(newValue, newValue);
        paintArea->SetTextureSideSize(texSize);
        paintArea->SetSize(texSize);
        scrollView->SetContentSize(texSize);
    }
}

void LandscapeEditorControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(IsValidPath(newValue))
    {
        if("TEXTURE_TEXTURE0" == forKey)
        {
            paintArea->SetTexture(PaintAreaControl::ET_TEXTURE0, newValue);
        }
        else if("TEXTURE_TEXTURE1/TEXTURE_DETAIL" == forKey)
        {
            paintArea->SetTexture(PaintAreaControl::ET_TEXTURE1, newValue);
        }
    }
}

void LandscapeEditorControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("Show Result" == forKey)
    {
        paintArea->ShowResultTexture(propertyList->GetBoolPropertyValue(forKey));
    }
}

bool LandscapeEditorControl::IsValidPath(const String &path)
{
    size_t pos = path.find(".png");
    return (String::npos != pos);
}

