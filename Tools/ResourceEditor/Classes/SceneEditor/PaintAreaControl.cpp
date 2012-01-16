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

TextureRenderObject::~TextureRenderObject()
{
    SafeRelease(sprite);
    SafeRelease(texture);
    SafeRelease(renderData);
}

void TextureRenderObject::Set(const String &texturepath)
{
    SafeRelease(renderData);
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
    float32 x0 = spr->GetRectOffsetValueForFrame(frameToGen, Sprite::X_OFFSET_TO_ACTIVE);
    float32 y0 = spr->GetRectOffsetValueForFrame(frameToGen, Sprite::Y_OFFSET_TO_ACTIVE);
    float32 x1 = x0 + spr->GetRectOffsetValueForFrame(frameToGen, Sprite::ACTIVE_WIDTH);
    float32 y1 = y0 + spr->GetRectOffsetValueForFrame(frameToGen, Sprite::ACTIVE_HEIGHT);
    //    x0 *= sprScale.x;
    //    x1 *= sprScale.y;
    //    y0 *= sprScale.x;
    //    y1 *= sprScale.y;
    
    //triangle 1
    //0, 0
    float32 *pT = spr->GetTextureVerts(frameToGen);
    
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
    showResultSprite = false;
    
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
    
    SetTextureSideSize(Vector2(rect.dx, rect.dy));
}

PaintAreaControl::~PaintAreaControl()
{
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
    
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
    String projectPath = settings->GetString("ProjectPath");
    textureRenderObjects[ETROID_A8_ALPHA]->Set(projectPath+"Data/Landscape/hmp2.png");
//    textureRenderObjects[ETROID_LIGHTMAP_RGB]->Set(projectPath+"Data/Landscape/rgb.png");
    textureRenderObjects[ETROID_LIGHTMAP_RGB]->Set(projectPath+"Data/Landscape/lightmap.png");
//    textureRenderObjects[ETROID_LIGHTMAP_RGB]->Set(projectPath+"Data/Landscape/lightmap-1.png");
}

void PaintAreaControl::ReleaseTextureRenderObjects()
{
    for(int32 i = 0; i < ETROID_COUNT; ++i)
    {
        SafeRelease(textureRenderObjects[i]);
    }
}

void PaintAreaControl::SetTextureSideSize(const Vector2 & sideSize)
{
    textureSideSize = sideSize;
    
    SafeRelease(spriteForAlpha);
    spriteForAlpha = Sprite::CreateAsRenderTarget(textureSideSize.x, textureSideSize.y, Texture::FORMAT_RGBA8888);
    DrawLightmap();

    //TODO: for test
    DrawA8();
    
    
    
    SafeRelease(spriteForResult);
    spriteForResult = Sprite::CreateAsRenderTarget(textureSideSize.x, textureSideSize.y, Texture::FORMAT_RGBA8888);
    RenderManager::Instance()->SetRenderTarget(spriteForResult);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 0.f));
    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));
    RenderManager::Instance()->ResetColor();
    RenderManager::Instance()->RestoreRenderTarget();

    ShowResultTexture(showResultSprite);
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

void PaintAreaControl::SetTexture(eTextures id, const String &path)
{
    textureRenderObjects[id + ETROID_TEXTURE_TEXTURE0]->Set(path);
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
    RenderManager::Instance()->SetRenderTarget(spriteForAlpha);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 0.f));
    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));

    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    
    RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderManager::Instance()->SetState(
                                        RenderStateBlock::STATE_TEXTURE0 
                                        | RenderStateBlock::STATE_COLORMASK_RED 
                                        | RenderStateBlock::STATE_COLORMASK_GREEN 
                                        | RenderStateBlock::STATE_COLORMASK_BLUE 
                                        );
    
    textureRenderObjects[ETROID_LIGHTMAP_RGB]->Draw();
    
    RenderManager::Instance()->ResetColor();
    RenderManager::Instance()->RestoreRenderTarget();
}

void PaintAreaControl::DrawA8()
{
    RenderManager::Instance()->SetRenderTarget(spriteForAlpha);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 0.f));
    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));
    
    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    
    RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderManager::Instance()->SetState(
                                        RenderStateBlock::STATE_TEXTURE0 
                                        | RenderStateBlock::STATE_COLORMASK_ALPHA 
                                        );
    
    textureRenderObjects[ETROID_A8_ALPHA]->Draw();
    
    RenderManager::Instance()->ResetColor();
    RenderManager::Instance()->RestoreRenderTarget();
}

void PaintAreaControl::Draw(const DAVA::UIGeometricData &geometricData)
{
    savedGeometricData = geometricData;
    
    DrawCursor();

    RenderManager::Instance()->SetRenderTarget(spriteForResult);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 0.f));
//    RenderManager::Instance()->SetColor(Color(1.f, 1.f, 1.f, 1.f));
//    RenderManager::Instance()->SetColor(Color(0.7f, 0.7f, 0.7f, 0.7f));
    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));
    RenderManager::Instance()->ResetColor();

    //DrawRenderObject();
    
    //DrawShader();

    RenderManager::Instance()->RestoreRenderTarget();

    UIControl::Draw(geometricData);
}



void PaintAreaControl::Input(DAVA::UIEvent *currentInput)
{
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
        
        GeneratePreview();
    }
}

void PaintAreaControl::UpdateMap()
{
//    if(usedTool && toolSprite && usedTool->zoom)
//    {
//        float32 scaleSize = toolSprite->GetWidth() * usedTool->radius * 4 / usedTool->zoom;
////        float32 radiusSize = scaleSize * usedTool->solidRadius;
//
//        Vector2 deltaPos = endPoint - startPoint;
////        if(radiusSize/4 <  deltaPos.Length() || !deltaPos.Length())
//        {
//            RenderManager::Instance()->SetRenderTarget(spriteForAlpha);
//            
//            eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
//            eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
//            RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);
//
//            toolSprite->SetScaleSize(scaleSize, scaleSize);
//            
//            Vector2 pos = (startPoint - savedGeometricData.position)/usedTool->zoom - Vector2(scaleSize, scaleSize)/2;
//            if(pos != prevDrawPos)
//            {
//                toolSprite->SetPosition(pos);
//                
//                paintColor.a = usedTool->height;
//                RenderManager::Instance()->SetColor(paintColor);
//                toolSprite->Draw();
//                RenderManager::Instance()->ResetColor();
//                
//                prevDrawPos = pos;
//            }
//            
//            RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
//            RenderManager::Instance()->RestoreRenderTarget();
//            
//            startPoint = endPoint;
//        }
//    }
}

void PaintAreaControl::GeneratePreview()
{
//    RenderManager::Instance()->SetRenderTarget(spriteForResult);
//    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 0.f));
//    RenderHelper::Instance()->FillRect(Rect(0, 0, textureSideSize.x, textureSideSize.y));
//    RenderManager::Instance()->ResetColor();
//
//    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
//    
//    RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
//    RenderManager::Instance()->SetState(
//                                        RenderStateBlock::STATE_BLEND | 
//                                        RenderStateBlock::STATE_TEXTURE0 | 
//                                        RenderStateBlock::STATE_COLORMASK_RED | 
//                                        RenderStateBlock::STATE_COLORMASK_GREEN | 
//                                        RenderStateBlock::STATE_COLORMASK_BLUE | 
//                                        RenderStateBlock::STATE_CULL);
//    
//
//    RenderManager::Instance()->SetTexture(textures[ET_TEXTURE0], 0);
//    
////
//    RenderManager::Instance()->SetState(
//                                        RenderStateBlock::STATE_BLEND | 
//                                        RenderStateBlock::STATE_TEXTURE1 | 
//                                        RenderStateBlock::STATE_COLORMASK_ALPHA | 
//                                        RenderStateBlock::STATE_CULL);
//
//    RenderManager::Instance()->SetTexture(spriteForAlpha->GetTexture(), 0);
//    
//    RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_2D_STATE_BLEND);
//
//    RenderManager::Instance()->RestoreRenderTarget();
//    
//    SetSprite(spriteForResult, 0);
//    SetSprite(textureRenderObjects[ETROID_LIGHTMAP_RGB]->sprite, 0);
}


