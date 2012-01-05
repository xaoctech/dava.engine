#include "LandscapeEditorControl.h"

#include "ControlsFactory.h"

//***************    PaintAreaControl    **********************
PaintAreaControl::PaintAreaControl(const Rect & rect)
    :   UIControl(rect)
{
    srcBlendMode = BLEND_SRC_ALPHA;
    dstBlendMode = BLEND_ONE;
    paintColor = Color(1.f, 1.f, 1.f, 1.f);
    
    spriteForDrawing = NULL;
    toolSprite = NULL;
    usedTool = NULL;
    
    startPoint = endPoint = Vector2(0, 0);
    
    currentMousePos = Vector2(-100, -100);
    prevDrawPos = Vector2(-100, -100);
    
    SetTextureSideSize(rect.dx, rect.dy);
}

PaintAreaControl::~PaintAreaControl()
{
    SafeRelease(toolSprite);
    SafeRelease(spriteForDrawing);
}

void PaintAreaControl::SetTextureSideSize(int32 sideSizeW, int32 sideSizeH)
{
    textureSideSize = Vector2(sideSizeW, sideSizeH);

    SafeRelease(spriteForDrawing);
    spriteForDrawing = Sprite::CreateAsRenderTarget(textureSideSize.x, textureSideSize.y, Texture::FORMAT_RGB565);
    SetSprite(spriteForDrawing, 0);
}

void PaintAreaControl::SetTextureSideSize(const Vector2 & sideSize)
{
    textureSideSize = sideSize;
    
    SafeRelease(spriteForDrawing);
    spriteForDrawing = Sprite::CreateAsRenderTarget(textureSideSize.x, textureSideSize.y, Texture::FORMAT_RGB565);
    SetSprite(spriteForDrawing, 0);
}


void PaintAreaControl::SetPaintTool(PaintTool *tool)
{
    usedTool = tool;
    SafeRelease(toolSprite);
    
    toolSprite = Sprite::Create(usedTool->spriteName);
}


void PaintAreaControl::Input(DAVA::UIEvent *currentInput)
{
    UIControl::Input(currentInput);
    
    if(UIEvent::BUTTON_1 == currentInput->tid)
    {
        srcBlendMode = BLEND_SRC_ALPHA;
        dstBlendMode = BLEND_ONE;

        paintColor = Color(1.f, 1.f, 1.f, 1.f);
        currentMousePos = currentInput->point;
    }
    else if(UIEvent::BUTTON_2 == currentInput->tid)
    {
        srcBlendMode = BLEND_SRC_ALPHA;
        dstBlendMode = BLEND_ONE_MINUS_SRC_ALPHA;

        paintColor = Color(0.f, 0.f, 0.f, 1.f);
        currentMousePos = currentInput->point;
    }

    if(UIEvent::PHASE_BEGAN == currentInput->phase)
    {
        startPoint = endPoint = currentInput->point;
        UpdateMap();
    }
    else if(UIEvent::PHASE_DRAG == currentInput->phase)
    {
        endPoint = currentInput->point;
        UpdateMap();
    }
    else if(UIEvent::PHASE_ENDED == currentInput->phase)
    {
        endPoint = currentInput->point;
        UpdateMap();
        prevDrawPos = Vector2(-100, -100);
        
        GeneratePreview();
    }
}

void PaintAreaControl::Draw(const DAVA::UIGeometricData &geometricData)
{
    savedGeometricData = geometricData;
    UIControl::Draw(geometricData);
    
    if(usedTool && toolSprite && usedTool->zoom && -100 != currentMousePos.x)
    {
        RenderManager::Instance()->SetColor(Color(1.f, 0.f, 0.f, 1.f));

        RenderManager::Instance()->ClipPush();
        RenderManager::Instance()->SetClip(geometricData.GetUnrotatedRect());
        
        float32 scaleSize = toolSprite->GetWidth() * usedTool->radius * 2;
        float32 radiusSize = scaleSize * usedTool->solidRadius;

        Vector2 pos = (currentMousePos);
        RenderManager::Instance()->SetColor(Color(1.f, 0.f, 0.f, 1.f));

        RenderHelper::Instance()->DrawCircle(pos, radiusSize);
        
        RenderManager::Instance()->ClipPop();
        
        RenderManager::Instance()->ResetColor();
    }
}

void PaintAreaControl::UpdateMap()
{
    if(usedTool && toolSprite && usedTool->zoom)
    {
        float32 scaleSize = toolSprite->GetWidth() * usedTool->radius * 4 / usedTool->zoom;
//        float32 radiusSize = scaleSize * usedTool->solidRadius;

        Vector2 deltaPos = endPoint - startPoint;
//        if(radiusSize/4 <  deltaPos.Length() || !deltaPos.Length())
        {
            RenderManager::Instance()->SetRenderTarget(spriteForDrawing);
            
            eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
            eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
            RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);

            toolSprite->SetScaleSize(scaleSize, scaleSize);
            
            Vector2 pos = (startPoint - savedGeometricData.position)/usedTool->zoom - Vector2(scaleSize, scaleSize)/2;
            if(pos != prevDrawPos)
            {
                toolSprite->SetPosition(pos);
                
                paintColor.a = usedTool->height;
                RenderManager::Instance()->SetColor(paintColor);
                toolSprite->Draw();
                RenderManager::Instance()->ResetColor();
                
                prevDrawPos = pos;
            }
            
            RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
            RenderManager::Instance()->RestoreRenderTarget();
            
            startPoint = endPoint;
        }
    }
}

void PaintAreaControl::GeneratePreview()
{
    
}


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
    
    KeyedArchive * keyedArchieve = new KeyedArchive();
    keyedArchieve->Load("~doc:/ResourceEditorOptions.archive");
    String projectPath = keyedArchieve->GetString("LastSavedPath", "/");
    if('/' != projectPath[projectPath.length() - 1])
    {
        projectPath += '/';
    }
    SafeRelease(keyedArchieve);
    
    propertyList->AddIntProperty("Size", 1024, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", projectPath, ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath, ".png", PropertyList::PROPERTY_IS_EDITABLE);
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
        SafeDelete(tools[i]);
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
            
        }
        else if("TEXTURE_TEXTURE1/TEXTURE_DETAIL" == forKey)
        {
            
        }
    }
}

bool LandscapeEditorControl::IsValidPath(const String &path)
{
    size_t pos = path.find(".png");
    return (String::npos != pos);
}

