#include "LandscapeEditorControl.h"

#include "ControlsFactory.h"

#include "EditorSettings.h"

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
    
    for (int32 i = 0; i < ET_COUNT; ++i)
    {
        textures[i] = NULL;
    }
    
    SetTextureSideSize(Vector2(rect.dx, rect.dy));
    
    renderData = NULL;
    InitShader();
}

PaintAreaControl::~PaintAreaControl()
{
    ReleaseShader();
    
    for (int32 i = 0; i < ET_COUNT; ++i)
    {
        SafeRelease(textures[i]);
    }

    SafeRelease(toolSprite);
    SafeRelease(spriteForDrawing);
}

void PaintAreaControl::SetTextureSideSize(const Vector2 & sideSize)
{
    textureSideSize = sideSize;
    
    SafeRelease(spriteForDrawing);
    spriteForDrawing = Sprite::CreateAsRenderTarget(textureSideSize.x, textureSideSize.y, Texture::FORMAT_RGBA8888);
//    SetSprite(spriteForDrawing, 0);
    
    RenderManager::Instance()->SetRenderTarget(spriteForDrawing);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 1.f));
    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));
    RenderManager::Instance()->ResetColor();
    RenderManager::Instance()->RestoreRenderTarget();
}


void PaintAreaControl::SetPaintTool(PaintTool *tool)
{
    usedTool = tool;
    SafeRelease(toolSprite);
    
    toolSprite = Sprite::Create(usedTool->spriteName);
}

void PaintAreaControl::SetTexture(eTextures id, const String &path)
{
    SafeRelease(textures[id]);
    textures[id] = Texture::CreateFromFile(path);
}

void PaintAreaControl::InitShader()
{
    blendedShader = new Shader();
    blendedShader->LoadFromYaml("~res:/Shaders/Landscape/blended-texture.shader");
    blendedShader->Recompile();
    
    uniformTexture0 = blendedShader->FindUniformLocationByName("texture0");
    uniformTexture1 = blendedShader->FindUniformLocationByName("texture1");
    uniformTextureMask = blendedShader->FindUniformLocationByName("textureMask");
}

void PaintAreaControl::ReleaseShader()
{
    SafeRelease(renderData);
    SafeRelease(blendedShader);
}

void PaintAreaControl::DrawShader()
{
    if (textures[ET_TEXTURE0])
    {
        RenderManager::Instance()->SetTexture(textures[ET_TEXTURE0], 0);   
    }
    if (textures[ET_TEXTURE1])
    {
        RenderManager::Instance()->SetTexture(textures[ET_TEXTURE1], 1);
    }
//    if (textures[TEXTURE_TEXTUREMASK])
    {
        RenderManager::Instance()->SetTexture(spriteForDrawing->GetTexture(), 2);
    }
    
    RenderManager::Instance()->SetShader(blendedShader);
    RenderManager::Instance()->FlushState();
    blendedShader->SetUniformValue(uniformTexture0, 0);
    blendedShader->SetUniformValue(uniformTexture1, 1);
    blendedShader->SetUniformValue(uniformTextureMask, 2);
//    blendedShader->SetUniformValue(uniformCameraPosition, cameraPos);    
    
    
    RenderManager::Instance()->SetTexture(0, 1);
    RenderManager::Instance()->SetTexture(0, 2);
}

void PaintAreaControl::DrawRenderObject()
{
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);

    RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
//    RenderManager::Instance()->SetState(
//                                        RenderStateBlock::STATE_BLEND | 
//                                        RenderStateBlock::STATE_TEXTURE0 | 
//                                        RenderStateBlock::STATE_CULL);
    
    int32 frame = 0;
    RenderManager::Instance()->SetTexture(spriteForDrawing->GetTexture(frame));
    RenderManager::Instance()->SetRenderData(renderData);
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, frame * 4, 4);

//    RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_2D_STATE);
}

void PaintAreaControl::DrawCursor()
{
    if(usedTool && toolSprite && usedTool->zoom && -100 != currentMousePos.x)
    {
        RenderManager::Instance()->ClipPush();
        RenderManager::Instance()->SetClip(savedGeometricData.GetUnrotatedRect());
        
        float32 scaleSize = toolSprite->GetWidth() * usedTool->radius * 2;
        float32 radiusSize = scaleSize * usedTool->solidRadius;
        
        Vector2 pos = (currentMousePos);
        RenderManager::Instance()->SetColor(Color(1.f, 0.f, 0.f, 1.f));
        
        RenderHelper::Instance()->DrawCircle(pos, radiusSize);
        
        RenderManager::Instance()->ClipPop();
        
        RenderManager::Instance()->ResetColor();
    } 
}

void PaintAreaControl::Draw(const DAVA::UIGeometricData &geometricData)
{
    savedGeometricData = geometricData;
    
    DrawCursor();
    
//    DrawShader();
    
    DrawRenderObject();
    
    UIControl::Draw(geometricData);
}


void PaintAreaControl::CreateMeshFromSprite(int32 frameToGen)
{
    float32 x0 = spriteForDrawing->GetRectOffsetValueForFrame(frameToGen, Sprite::X_OFFSET_TO_ACTIVE);
    float32 y0 = spriteForDrawing->GetRectOffsetValueForFrame(frameToGen, Sprite::Y_OFFSET_TO_ACTIVE);
    float32 x1 = x0 + spriteForDrawing->GetRectOffsetValueForFrame(frameToGen, Sprite::ACTIVE_WIDTH);
    float32 y1 = y0 + spriteForDrawing->GetRectOffsetValueForFrame(frameToGen, Sprite::ACTIVE_HEIGHT);
//    x0 *= sprScale.x;
//    x1 *= sprScale.y;
//    y0 *= sprScale.x;
//    y1 *= sprScale.y;
    
    //triangle 1
    //0, 0
    float32 *pT = spriteForDrawing->GetTextureVerts(frameToGen);
    
    verts.push_back(x0);
    verts.push_back(y0);
    verts.push_back(0);
    
    
    //1, 0
    verts.push_back(x1);
    verts.push_back(y0);
    verts.push_back(0);
    
    
    //0, 1
    verts.push_back(x0);
    verts.push_back(y1);
    verts.push_back(0);
    
    //1, 1
    verts.push_back(x1);
    verts.push_back(y1);
    verts.push_back(0);
    
    for (int i = 0; i < 2*4; i++) 
    {
        textureCoords.push_back(*pT);
        pT++;
    }
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
        dstBlendMode = BLEND_SRC_ALPHA_SATURATE;

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
    SafeRelease(renderData);
    verts.clear();
    textureCoords.clear();
    
    for (int i = 0; i < spriteForDrawing->GetFrameCount(); i++) 
    {
        CreateMeshFromSprite(i);
    }
    renderData = new RenderDataObject();
    renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());
    renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textureCoords.front());
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

    paintArea->SetTexture(PaintAreaControl::ET_TEXTURE0, propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE0"));
    paintArea->SetTexture(PaintAreaControl::ET_TEXTURE1, propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL"));

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

bool LandscapeEditorControl::IsValidPath(const String &path)
{
    size_t pos = path.find(".png");
    return (String::npos != pos);
}

