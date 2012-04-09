#include "LandscapeEditorHeightmap.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelHeightmap.h"
#include "PropertyControlCreator.h"

#include "../EditorScene.h"

#include "ImageRasterizer.h"



#pragma mark --LandscapeEditorHeightmap
LandscapeEditorHeightmap::LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, 
                                           EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
	wasTileMaskToolUpdate = false;

    landscapeDebugNode = NULL;
    heightImage = NULL;
    toolImage = NULL;
    
    toolsPanel = new LandscapeToolsPanelHeightmap(this, toolsRect);

    drawSprite = NULL;
}

LandscapeEditorHeightmap::~LandscapeEditorHeightmap()
{
    SafeRelease(drawSprite);
    
    
    SafeRelease(toolImage);
    SafeRelease(heightImage);
    SafeRelease(landscapeDebugNode);
}

void LandscapeEditorHeightmap::Draw(const UIGeometricData &geometricData)
{
//    if(drawSprite)
//    {
//        RenderManager::Instance()->SetColor(Color::Black());
//        RenderHelper::Instance()->FillRect(Rect(0, 0, 50, 50));
//
//        RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
//
//        RenderManager::Instance()->SetColor(Color::White());
//
//        drawSprite->SetPosition(0, 0);
//        drawSprite->SetScaleSize(50, 50);
//        drawSprite->Draw();
//    }
    
//    if(toolImage)
//    {
//        RenderManager::Instance()->LockNonMain();
//        
//        Texture *tex = Texture::CreateFromData((PixelFormat)toolImage->GetPixelFormat(), toolImage->GetData(), 
//                                               toolImage->GetWidth(), toolImage->GetHeight());
//        
//        Sprite *sprite = Sprite::CreateFromTexture(tex, 0, 0, toolImage->GetWidth(), toolImage->GetHeight());
//        
//        eBlendMode src = RenderManager::Instance()->GetSrcBlend();
//        eBlendMode dest = RenderManager::Instance()->GetDestBlend();
//        RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
//
//        RenderManager::Instance()->SetColor(Color::Black());
//        RenderHelper::Instance()->FillRect(Rect(0, 0, 50, 50));
//
//        
//        RenderManager::Instance()->SetColor(Color::White());
//        
//        sprite->SetPosition(Vector2(0, 0));
//        sprite->SetScaleSize(50, 50);
//        sprite->Draw();
//        
//        RenderManager::Instance()->SetBlendMode(src, dest);
//        
//        
//        SafeRelease(sprite);
//        SafeRelease(tex);
//        
//        RenderManager::Instance()->UnlockNonMain();
//    }
    
    LandscapeEditorBase::Draw(geometricData);
}

void LandscapeEditorHeightmap::Update(float32 timeElapsed)
{
    if(editingIsEnabled)
    {
        UpdateTileMaskTool(timeElapsed);
    }
    
    LandscapeEditorBase::Update(timeElapsed);
}


void LandscapeEditorHeightmap::CreateMaskTexture()
{
    SafeRelease(heightImage);
    
    heightImage = Image::CreateFromFile(workingLandscape->GetHeightMapPathname());
    landscapeDebugNode->SetDebugHeightmapImage(heightImage, workingLandscape->GetBoundingBox());
    
    landscapeSize = heightImage->GetWidth();
}

void LandscapeEditorHeightmap::UpdateToolImage()
{
    if(toolImage && currentTool)
    {
        if(toolImage->width != currentTool->size)
        {
            SafeRelease(toolImage);
        }
    }

    if(currentTool && !toolImage)
    {
        RenderManager::Instance()->LockNonMain();

//        toolImage = SafeRetain(currentTool->image);
        Image *image = currentTool->image;
        
        int32 sideSize = currentTool->size;
//        int32 sideSize = image->GetWidth();
        
//        Sprite *dstSprite = Sprite::CreateAsRenderTarget(sideSize, sideSize, FORMAT_A8);
        Sprite *dstSprite = Sprite::CreateAsRenderTarget(sideSize, sideSize, FORMAT_RGBA8888);

        Texture *srcTex = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), 
                                                  image->GetWidth(), image->GetHeight());
        
        Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, image->GetWidth(), image->GetHeight());
        
        RenderManager::Instance()->SetRenderTarget(dstSprite);

        RenderManager::Instance()->SetColor(Color::Black());
        RenderHelper::Instance()->FillRect(Rect(0, 0, sideSize, sideSize));

        
        RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
        RenderManager::Instance()->SetColor(Color::White());

        srcSprite->SetScaleSize(sideSize, sideSize);
        srcSprite->SetPosition(Vector2(0, 0));
        srcSprite->Draw();
        RenderManager::Instance()->RestoreRenderTarget();

        toolImage = dstSprite->GetTexture()->CreateImageFromMemory();
        
        toolImage->Save("/Users/klesch/Work/WoT/Framework/wot.sniper/DataSource/3d/test.png");
        currentTool->image->Save("/Users/klesch/Work/WoT/Framework/wot.sniper/DataSource/3d/test1.png");
        
//        drawSprite = SafeRetain(srcSprite);
        drawSprite = SafeRetain(dstSprite);
        
        SafeRelease(srcSprite);
        SafeRelease(srcTex);
        SafeRelease(dstSprite);

        RenderManager::Instance()->UnlockNonMain();
    }
}


void LandscapeEditorHeightmap::UpdateTileMaskTool(float32 timeElapsed)
{
//    UpdateToolImage();

//    uint64 deltaTime = endTime - startTime;
//    if(deltaTime)
    {
        if(currentTool && toolImage && currentTool->strength)
        {
            float32 scaleSize = toolImage->GetWidth();
//          Vector2 deltaPos = endPoint - startPoint;
            {
                Vector2 pos = startPoint - Vector2(scaleSize, scaleSize)/2;
                if(pos != prevDrawPos)
                {
                    wasTileMaskToolUpdate = true;
                    
                    float32 koef = currentTool->strength * timeElapsed;
//                    ImageRasterizer::DrawRelative(heightImage, toolImage, pos.x, pos.y, scaleSize, scaleSize, koef);
                    ImageRasterizer::DrawRelativeRGBA(heightImage, toolImage, pos.x, pos.y, scaleSize, scaleSize, koef);
//                    ImageRasterizer::DrawAverage(heightImage, toolImage, pos.x, pos.y, scaleSize, scaleSize, deltaTime / 1000.0f);
                }
                startPoint = endPoint;
            }
        }
    }
}

void LandscapeEditorHeightmap::InputAction(int32 phase)
{
    switch(phase)
    {
        case UIEvent::PHASE_BEGAN:
        {
            editingIsEnabled = true;
            UpdateToolImage();

            break;
        }

        case UIEvent::PHASE_ENDED:
        {
            editingIsEnabled = false;
            break;
        }
            
        default:
            break;
    }
    
//    UpdateTileMaskTool(); 
}

void LandscapeEditorHeightmap::HideAction()
{
    SafeRelease(toolImage);
    
    workingScene->AddNode(workingLandscape);
    
    workingScene->RemoveNode(landscapeDebugNode);
    SafeRelease(landscapeDebugNode);
}

void LandscapeEditorHeightmap::ShowAction()
{
    workingScene->RemoveNode(workingLandscape);
    

    landscapeDebugNode = new LandscapeDebugNode(workingScene);
    
    landscapeDebugNode->SetRenderingMode(workingLandscape->GetRenderingMode());
    for(int32 iTex = 0; iTex < LandscapeNode::TEXTURE_COUNT; ++iTex)
    {
        landscapeDebugNode->SetTexture((LandscapeNode::eTextureLevel)iTex, 
                                       workingLandscape->GetTexture((LandscapeNode::eTextureLevel)iTex));
        
        landscapeDebugNode->SetTextureTiling((LandscapeNode::eTextureLevel)iTex, 
                                             workingLandscape->GetTextureTiling((LandscapeNode::eTextureLevel)iTex));
    }

    workingScene->AddNode(landscapeDebugNode);
    
    
    CreateMaskTexture();
}

void LandscapeEditorHeightmap::SaveTextureAction(const String &pathToFile)
{
    if(heightImage)
    {
        heightImage->Save(pathToFile);
        SafeRelease(heightImage);
        SafeRelease(savedTexture);
    }
}

NodesPropertyControl *LandscapeEditorHeightmap::GetPropertyControl(const Rect &rect)
{
    NodesPropertyControl *propsControl = 
    PropertyControlCreator::Instance()->CreateControlForNode(workingLandscape, rect, false);
    
    return propsControl;
}

#pragma mark  --LandscapeToolsPanelDelegate
void LandscapeEditorHeightmap::OnToolSelected(LandscapeTool *newTool)
{
    LandscapeEditorBase::OnToolSelected(newTool);
    SafeRelease(toolImage);
}


