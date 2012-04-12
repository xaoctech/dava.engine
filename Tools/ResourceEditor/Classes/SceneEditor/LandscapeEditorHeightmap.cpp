#include "LandscapeEditorHeightmap.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelHeightmap.h"
#include "PropertyControlCreator.h"

#include "../EditorScene.h"

#include "ImageRasterizer.h"
#include "HeightmapNode.h"

#pragma mark --LandscapeEditorHeightmap
LandscapeEditorHeightmap::LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, 
                                           EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
	wasTileMaskToolUpdate = false;

    landscapeDebugNode = NULL;
    heightmap = NULL;
    toolImage = NULL;
    
    toolsPanel = new LandscapeToolsPanelHeightmap(this, toolsRect);
    
    
    Image *img1 = Image::CreateFromFile("/Users/klesch/Work/WoT/Framework/wot.sniper/DataSource/heit_lm_1_a16.png");
    if(img1)
    {
        img1->Save("/Users/klesch/Work/WoT/Framework/wot.sniper/DataSource/heit_lm_1_a16_copy.png");
        SafeRelease(img1);
    }
}


LandscapeEditorHeightmap::~LandscapeEditorHeightmap()
{
    SafeRelease(heightmap);
    SafeRelease(toolImage);
    SafeRelease(landscapeDebugNode);
}

void LandscapeEditorHeightmap::Update(float32 timeElapsed)
{
    if(editingIsEnabled)
    {
        UpdateTileMaskTool(timeElapsed);
    }
    
    LandscapeEditorBase::Update(timeElapsed);
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

        Image *image = currentTool->image;
        
        int32 sideSize = currentTool->size;
        Sprite *dstSprite = Sprite::CreateAsRenderTarget(sideSize, sideSize, FORMAT_RGBA8888);
        Texture *srcTex = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), 
                                                  image->GetWidth(), image->GetHeight());
        Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, image->GetWidth(), image->GetHeight());
        
        RenderManager::Instance()->SetRenderTarget(dstSprite);

        RenderManager::Instance()->SetColor(Color::Black());
        RenderHelper::Instance()->FillRect(Rect(0, 0, dstSprite->GetTexture()->GetWidth(), dstSprite->GetTexture()->GetHeight()));

        RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
        RenderManager::Instance()->SetColor(Color::White());

        srcSprite->SetScaleSize(sideSize, sideSize);
        srcSprite->SetPosition(Vector2((dstSprite->GetTexture()->GetWidth() - sideSize)/2.0, 
                                       (dstSprite->GetTexture()->GetHeight() - sideSize)/2.0));
        srcSprite->Draw();
        RenderManager::Instance()->RestoreRenderTarget();

        toolImage = dstSprite->GetTexture()->CreateImageFromMemory();
  
//TODO: for debug        
//        toolImage->Save("/Users/klesch/Work/WoT/Framework/wot.sniper/DataSource/Test.png");
        
        SafeRelease(srcSprite);
        SafeRelease(srcTex);
        SafeRelease(dstSprite);

        RenderManager::Instance()->UnlockNonMain();
    }
}


void LandscapeEditorHeightmap::UpdateTileMaskTool(float32 timeElapsed)
{
    if(currentTool && toolImage && currentTool->strength)
    {
        float32 scaleSize = toolImage->GetWidth();
        Vector2 pos = startPoint - Vector2(scaleSize, scaleSize)/2;
        if(pos != prevDrawPos)
        {
            wasTileMaskToolUpdate = true;
            
            if(currentTool->averageDrawing)
            {
                float32 koef = fabsf(currentTool->strength * timeElapsed / currentTool->maxStrength);
                ImageRasterizer::DrawAverageRGBA(heightmap, currentTool->image, pos.x, pos.y, scaleSize, scaleSize, koef);
            }
            else if(currentTool->relativeDrawing)
            {
                float32 koef = (currentTool->strength * timeElapsed);
                if(inverseDrawingEnabled)
                {
                    koef = -koef;
                }
                ImageRasterizer::DrawRelativeRGBA(heightmap, toolImage, pos.x, pos.y, scaleSize, scaleSize, koef);
            }
            else 
            {
                float32 koef = (currentTool->strength * timeElapsed);
                ImageRasterizer::DrawAbsoluteRGBA(heightmap, toolImage, pos.x, pos.y, scaleSize, scaleSize, koef, currentTool->height);
            }
            
            workingScene->RemoveNode(heightmapNode);
            SafeRelease(heightmapNode);
            heightmapNode = new HeightmapNode(workingScene);
            workingScene->AddNode(heightmapNode);
        }
        startPoint = endPoint;
    }
}

void LandscapeEditorHeightmap::UpdateCursor()
{
	if(currentTool && toolImage)
	{
		float32 scaleSize = currentTool->size;
		Vector2 pos = startPoint - Vector2(scaleSize, scaleSize)/2;

		landscapeDebugNode->SetCursorTexture(cursorTexture);
		landscapeDebugNode->SetBigTextureSize(heightmap->Size());
		landscapeDebugNode->SetCursorPosition(pos);
		landscapeDebugNode->SetCursorScale(scaleSize);
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
}

void LandscapeEditorHeightmap::HideAction()
{
	landscapeDebugNode->CursorDisable();
    SafeRelease(toolImage);
    
    workingScene->AddNode(workingLandscape);
    workingLandscape->BuildLandscapeFromHeightmapImage(workingLandscape->GetRenderingMode(), 
//                                                       workingLandscape->GetHeightMapPathname(), 
                                                       savedPath,
                                                       workingLandscape->GetBoundingBox());
    
    workingScene->RemoveNode(landscapeDebugNode);
    SafeRelease(landscapeDebugNode);
    
    SafeRelease(heightmap);
}

void LandscapeEditorHeightmap::ShowAction()
{
    workingScene->RemoveNode(workingLandscape);

    landscapeDebugNode = new LandscapeDebugNode(workingScene);
    landscapeDebugNode->SetName("Landscape");
    landscapeDebugNode->SetHeightmapPath(workingLandscape->GetHeightMapPathname());
    
    landscapeDebugNode->SetRenderingMode(workingLandscape->GetRenderingMode());
    for(int32 iTex = 0; iTex < LandscapeNode::TEXTURE_COUNT; ++iTex)
    {
        landscapeDebugNode->SetTexture((LandscapeNode::eTextureLevel)iTex, 
                                       workingLandscape->GetTexture((LandscapeNode::eTextureLevel)iTex));
        
        landscapeDebugNode->SetTextureTiling((LandscapeNode::eTextureLevel)iTex, 
                                             workingLandscape->GetTextureTiling((LandscapeNode::eTextureLevel)iTex));
    }

    workingScene->AddNode(landscapeDebugNode);


    heightmap = SafeRetain(workingLandscape->GetHeightmap());
    savedPath = workingLandscape->GetHeightMapPathname();
    landscapeDebugNode->SetDebugHeightmapImage(heightmap, workingLandscape->GetBoundingBox());
    
    landscapeSize = heightmap->Size() - 1;

	landscapeDebugNode->CursorEnable();
}

void LandscapeEditorHeightmap::SaveTextureAction(const String &pathToFile)
{
    if(heightmap)
    {
        String heightmapPath = pathToFile;
        String extension = FileSystem::Instance()->GetExtension(pathToFile);
        if(Heightmap::FileExtension() != extension)
        {
            heightmapPath = FileSystem::Instance()->ReplaceExtension(heightmapPath, Heightmap::FileExtension());
        }
        savedPath = heightmapPath;
        heightmap->Save(heightmapPath);
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




