#include "PaintAreaControl.h"

#include "ControlsFactory.h"

#include "EditorSettings.h"

//***************** TextureRenderObject *********************
TextureRenderObject::TextureRenderObject()
{
    renderData = NULL;
    texture = NULL;
    sprite = NULL;
}

Rect usedRect;

TextureRenderObject::~TextureRenderObject()
{
    SafeRelease(sprite);
    SafeRelease(texture);
    SafeRelease(renderData);
}

void TextureRenderObject::Set(const String &texturepath)
{
    SafeRelease(renderData);
    SafeRelease(texture);
    SafeRelease(sprite);

    textureCoords.clear();
    vertexes.clear();
    
    texture = Texture::CreateFromFile(texturepath);
    if(texture)
    {
        texture->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
        
        sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());
        for (int32 i = 0; i < sprite->GetFrameCount(); ++i) 
        {
            CreateMeshFromSprite(sprite, i);
        }
        
        renderData = new RenderDataObject();
        renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &vertexes.front());
        renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textureCoords.front());
    }
}

void TextureRenderObject::CreateMeshFromSprite(Sprite *spr, int32 frameToGen)
{
    float32 x0 = usedRect.x;
    float32 y0 = usedRect.y;
    float32 x1 = usedRect.dx;
    float32 y1 = usedRect.dy;

    //triangle 1
    //0, 0
    vertexes.push_back(x0);
    vertexes.push_back(y0);
    vertexes.push_back(0);
    //1, 0
    vertexes.push_back(x1);
    vertexes.push_back(y0);
    vertexes.push_back(0);
    //0, 1
    vertexes.push_back(x0);
    vertexes.push_back(y1);
    vertexes.push_back(0);
    //1, 1
    vertexes.push_back(x1);
    vertexes.push_back(y1);
    vertexes.push_back(0);
    
    float32 *pT = spr->GetTextureVerts(frameToGen);
    for (int i = 0; i < 2*4; i++) 
    {
        textureCoords.push_back(*pT);
        pT++;
    }
}

void TextureRenderObject::Draw()
{
    if(texture)
    {
        RenderManager::Instance()->SetTexture(texture);
        RenderManager::Instance()->SetRenderData(renderData);
        RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    }
}


//***************    PaintAreaControl    **********************
PaintAreaControl::PaintAreaControl(const Rect & rect)
    :   UIControl(rect)
{
    usedRect = rect;
    
    showResultSprite = false;
    
    drawingMask = EDM_NONE;
    
    srcBlendMode = BLEND_SRC_ALPHA;
    dstBlendMode = BLEND_ONE;
    paintColor = Color(1.f, 1.f, 1.f, 1.f);
    
    spriteForAlpha = NULL;
    spriteForResult = NULL;
    toolSprite = NULL;
    usedTool = NULL;
    
    startPoint = endPoint = Vector2(0, 0);
    
    currentMousePos = Vector2(-100, -100);
    prevDrawPos = Vector2(-100, -100);

    for(int32 i = 0; i < ETROID_COUNT; ++i)
    {
        textureRenderObjects[i] = NULL;
    }
    
    InitShader();
    
    InitTextureRenderObjects();
    
    renderData = NULL;
    SetTextureSideSize(Vector2(rect.dx, rect.dy));
}

PaintAreaControl::~PaintAreaControl()
{
    SafeRelease(renderData);
    
    ReleaseTextureRenderObjects();
    
    ReleaseShader();
    
    SafeRelease(toolSprite);
    SafeRelease(spriteForAlpha);
    SafeRelease(spriteForResult);
}

void PaintAreaControl::InitTextureRenderObjects()
{
    for(int32 i = 0; i < ETROID_COUNT; ++i)
    {
        textureRenderObjects[i] = new TextureRenderObject();
    }
}

void PaintAreaControl::ReleaseTextureRenderObjects()
{
    for(int32 i = 0; i < ETROID_COUNT; ++i)
    {
        SafeRelease(textureRenderObjects[i]);
    }
}

void PaintAreaControl::Recreate()
{
    SetTextureSideSize(textureSideSize);
}


void PaintAreaControl::SetTextureSideSize(const Vector2 & sideSize)
{
    textureSideSize = sideSize;
    
    SafeRelease(spriteForAlpha);
    spriteForAlpha = Sprite::CreateAsRenderTarget(textureSideSize.x, textureSideSize.y, Texture::FORMAT_RGBA8888);
    DrawLightmap();

    SafeRelease(spriteForResult);
    spriteForResult = Sprite::CreateAsRenderTarget(textureSideSize.x, textureSideSize.y, Texture::FORMAT_RGBA8888);

    ShowResultTexture(showResultSprite);
    
    // Create renderdata for shsader
    vertexes.clear();
    textureCoords.clear();
    SafeRelease(renderData);

    Rect r = GetRect();
    float32 x0 = 0;
    float32 y0 = 0;
    float32 x1 = textureSideSize.x;
    float32 y1 = textureSideSize.y;
    
    //triangle 1
    //0, 0
    vertexes.push_back(x0);
    vertexes.push_back(y0);
    vertexes.push_back(0);
    textureCoords.push_back(0);
    textureCoords.push_back(0);
    
    
    //1, 0
    vertexes.push_back(x1);
    vertexes.push_back(y0);
    vertexes.push_back(0);
    textureCoords.push_back(1);
    textureCoords.push_back(0);
    
    
    //0, 1
    vertexes.push_back(x0);
    vertexes.push_back(y1);
    vertexes.push_back(0);
    textureCoords.push_back(0);
    textureCoords.push_back(1);
    
    //1, 1
    vertexes.push_back(x1);
    vertexes.push_back(y1);
    vertexes.push_back(0);
    textureCoords.push_back(1);
    textureCoords.push_back(1);

    renderData = new RenderDataObject();
    renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &vertexes.front());
    renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textureCoords.front());
}


void PaintAreaControl::SetPaintTool(PaintTool *tool)
{
    usedTool = tool;
    SafeRelease(toolSprite);
    
    toolSprite = Sprite::Create(usedTool->spriteName);
}

void PaintAreaControl::ShowResultTexture(bool show)
{
    showResultSprite = show;
    if(showResultSprite)
    {
        SetSprite(spriteForResult, 0);
    }
    else
    {
        SetSprite(spriteForAlpha, 0);
    }
}

void PaintAreaControl::SetTexture(eTextureRenderObjectIDs id, const String &path)
{
    textureRenderObjects[id]->Set(path);
    Recreate();
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
    SafeRelease(blendedShader);
}

void PaintAreaControl::DrawShader()
{
    if (textureRenderObjects[ETROID_TEXTURE_TEXTURE0])
    {
        RenderManager::Instance()->SetTexture(textureRenderObjects[ETROID_TEXTURE_TEXTURE0]->texture, 0);   
    }
    if (textureRenderObjects[ETROID_TEXTURE_TEXTURE1])
    {
        RenderManager::Instance()->SetTexture(textureRenderObjects[ETROID_TEXTURE_TEXTURE1]->texture, 1);
    }

    RenderManager::Instance()->SetTexture(spriteForAlpha->GetTexture(), 2);
    
    RenderManager::Instance()->SetShader(blendedShader);
    RenderManager::Instance()->FlushState();
    blendedShader->SetUniformValue(uniformTexture0, 0);
    blendedShader->SetUniformValue(uniformTexture1, 1);
    blendedShader->SetUniformValue(uniformTextureMask, 2);
    
    RenderManager::Instance()->SetRenderData(renderData);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    
    RenderManager::Instance()->SetTexture(0, 1);
    RenderManager::Instance()->SetTexture(0, 2);
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

void PaintAreaControl::DrawLightmap()
{
    //save states
    eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
    eBlendMode dstMode = RenderManager::Instance()->GetDestBlend();
    uint32 oldState = RenderManager::Instance()->GetState();
    
    //fill texture with initial color
    RenderManager::Instance()->SetRenderTarget(spriteForAlpha);
    RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
    RenderManager::Instance()->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));

    //draw rect with color mask
    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderManager::Instance()->SetState(
                                        RenderStateBlock::STATE_COLORMASK_RED 
                                        | RenderStateBlock::STATE_COLORMASK_GREEN 
                                        | RenderStateBlock::STATE_COLORMASK_BLUE 
                                        );

    if(textureRenderObjects[ETROID_LIGHTMAP_RGB]->sprite)
    {
        textureRenderObjects[ETROID_LIGHTMAP_RGB]->Draw();
    }
    
    RenderManager::Instance()->ResetColor();
    RenderManager::Instance()->RestoreRenderTarget();
    
    //restore states
    RenderManager::Instance()->SetState(oldState);
    RenderManager::Instance()->SetBlendMode(srcMode, dstMode);
}

void PaintAreaControl::Draw(const DAVA::UIGeometricData &geometricData)
{
    savedGeometricData = geometricData;
    
    DrawCursor();

    //save states
    eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
    eBlendMode dstMode = RenderManager::Instance()->GetDestBlend();
    uint32 oldState = RenderManager::Instance()->GetState();

    RenderManager::Instance()->SetRenderTarget(spriteForResult);
    RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 0.f));
    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));
    RenderManager::Instance()->ResetColor();

    DrawShader();
    
    RenderManager::Instance()->RestoreRenderTarget();

    //restore states
    RenderManager::Instance()->SetState(oldState);
    RenderManager::Instance()->SetBlendMode(srcMode, dstMode);

    UIControl::Draw(geometricData);
}



void PaintAreaControl::Input(DAVA::UIEvent *currentInput)
{
    bool spaceIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SPACE);
    if(spaceIsPressed) return;
    
    UIControl::Input(currentInput);
    
    if(UIEvent::BUTTON_1 == currentInput->tid)
    {
        srcBlendMode = BLEND_SRC_ALPHA;
        dstBlendMode = BLEND_ONE;

        paintColor = Color(1.f, 1.f, 1.f, 1.0f);
        currentMousePos = currentInput->point;
    }
    else if(UIEvent::BUTTON_2 == currentInput->tid)
    {
        srcBlendMode = BLEND_SRC_ALPHA;
        dstBlendMode = BLEND_SRC_ALPHA_SATURATE;

        paintColor = Color(0.f, 0.f, 0.f, 1.0f);
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
    }
}

void PaintAreaControl::UpdateMap()
{
    if(usedTool && toolSprite && usedTool->zoom)
    {
        float32 scaleSize = toolSprite->GetWidth() * usedTool->radius * 4 / (usedTool->zoom * PaintAreaControl::ZOOM_MULTIPLIER);
        Vector2 deltaPos = endPoint - startPoint;
        {
            Vector2 pos = (startPoint - savedGeometricData.position)/(usedTool->zoom * PaintAreaControl::ZOOM_MULTIPLIER)- Vector2(scaleSize, scaleSize)/2;
            if(pos != prevDrawPos)
            {
                //save states
                eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
                eBlendMode dstMode = RenderManager::Instance()->GetDestBlend();
                uint32 oldState = RenderManager::Instance()->GetState();
                
                RenderManager::Instance()->SetRenderTarget(spriteForAlpha);
                
                int32 drawState =  RenderStateBlock::STATE_BLEND;
                if(drawingMask & EDM_RED)
                    drawState |= RenderStateBlock::STATE_COLORMASK_RED;
                if(drawingMask & EDM_GREEN)
                    drawState |= RenderStateBlock::STATE_COLORMASK_GREEN;
                if(drawingMask & EDM_BLUE)
                    drawState |= RenderStateBlock::STATE_COLORMASK_BLUE;
                if(drawingMask & EDM_ALPHA)
                    drawState |= RenderStateBlock::STATE_COLORMASK_ALPHA;
                
                
                RenderManager::Instance()->SetState(drawState);
                RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);
                
                paintColor.a = usedTool->intension;
                RenderManager::Instance()->SetColor(paintColor);

                //set tool sprite
                toolSprite->SetScaleSize(scaleSize, scaleSize);
                toolSprite->SetPosition(pos);
                
                toolSprite->Draw();
                prevDrawPos = pos;
                
                
                RenderManager::Instance()->RestoreRenderTarget();
                
                //restore states
                RenderManager::Instance()->SetState(oldState);
                RenderManager::Instance()->SetBlendMode(srcMode, dstMode);
            }
            
            startPoint = endPoint;
        }
    }
}

int32 PaintAreaControl::GetDrawingMask()
{
    return drawingMask;
}

void PaintAreaControl::SetDrawingMask(int32 newMask)
{
    drawingMask = newMask;
}

void PaintAreaControl::SaveMask(const String &pathToFile)
{
    if(spriteForAlpha)
    {
        Image *im = spriteForAlpha->GetTexture()->CreateImageFromMemory();   
        if(im)
        {
            im->Save(pathToFile);
        }
    }
}


