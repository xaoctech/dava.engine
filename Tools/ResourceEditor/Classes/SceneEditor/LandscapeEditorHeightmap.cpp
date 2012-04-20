#include "LandscapeEditorHeightmap.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelHeightmap.h"
#include "PropertyControlCreator.h"

#include "../EditorScene.h"

#include "ImageRasterizer.h"
#include "HeightmapNode.h"

#include "UNDOManager.h"

#pragma mark --LandscapeEditorHeightmap
LandscapeEditorHeightmap::LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, 
                                           EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
    landscapeDebugNode = NULL;
    heightmap = NULL;
    toolImage = NULL;
    
    toolsPanel = new LandscapeToolsPanelHeightmap(this, toolsRect);
    
    prevToolSize = 0.f;
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
        if(currentTool)
        {
            if(LandscapeTool::TOOL_BRUSH == currentTool->type)
            {
                UpdateTileMaskTool(timeElapsed);
            }
            else if(LandscapeTool::TOOL_DROPPER == currentTool->type)
            {
                currentTool->height = GetDropperHeight();
            }
        }
    }
    
    LandscapeEditorBase::Update(timeElapsed);
}

void LandscapeEditorHeightmap::UpdateToolImage()
{
    if(toolImage && currentTool)
    {
        if(prevToolSize != currentTool->size)
        {
            SafeRelease(toolImage);
        }
    }

    if(currentTool && !toolImage)
    {
        prevToolSize = currentTool->size;

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
        srcSprite->SetPosition(Vector2((dstSprite->GetTexture()->GetWidth() - sideSize)/2.0f, 
                                       (dstSprite->GetTexture()->GetHeight() - sideSize)/2.0f));
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
    if(toolImage && currentTool->strength && LandscapeTool::TOOL_BRUSH == currentTool->type)
    {
        int32 scaleSize = toolImage->GetWidth();
        Vector2 pos = startPoint - Vector2(scaleSize, scaleSize)/2;
        {
            if(currentTool->averageDrawing)
            {
                float32 koef = (currentTool->averageStrength * timeElapsed);
                ImageRasterizer::DrawAverageRGBA(heightmap, toolImage, pos.x, pos.y, scaleSize, scaleSize, koef);
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
                Vector3 landSize;
                AABBox3 transformedBox;
                workingLandscape->GetBoundingBox().GetTransformedBox(workingLandscape->GetWorldTransform(), transformedBox);
                landSize = transformedBox.max - transformedBox.min;
                
                float32 maxHeight = landSize.z;
                float32 height = currentTool->height / maxHeight * Heightmap::MAX_VALUE;
                
                float32 koef = (currentTool->averageStrength * timeElapsed);
                ImageRasterizer::DrawAbsoluteRGBA(heightmap, toolImage, pos.x, pos.y, scaleSize, scaleSize, koef, height);
            }
            
            heightmapNode->UpdateHeightmapRect(Rect(pos.x, pos.y, scaleSize, scaleSize));
        }
        startPoint = endPoint;
    }
}

float32 LandscapeEditorHeightmap::GetDropperHeight()
{
    Vector3 landSize;
    AABBox3 transformedBox;
    workingLandscape->GetBoundingBox().GetTransformedBox(workingLandscape->GetWorldTransform(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;

    int32 index = startPoint.x + startPoint.y * heightmap->Size();
    float32 height = heightmap->Data()[index];
    float32 maxHeight = landSize.z;
    return (height / Heightmap::MAX_VALUE * maxHeight);
}

void LandscapeEditorHeightmap::UpdateCursor()
{
	if(currentTool)
	{
		int32 scaleSize = currentTool->size;
		Vector2 pos = startPoint - Vector2(scaleSize, scaleSize)/2;

		landscapeDebugNode->SetCursorTexture(cursorTexture);
		landscapeDebugNode->SetBigTextureSize(landscapeSize);
		landscapeDebugNode->SetCursorPosition(pos);
		landscapeDebugNode->SetCursorScale(scaleSize);
        
        heightmapNode->cursor->SetCursorTexture(cursorTexture);
		heightmapNode->cursor->SetBigTextureSize(landscapeSize);
		heightmapNode->cursor->SetPosition(pos);
		heightmapNode->cursor->SetScale(scaleSize);
	}
}

void LandscapeEditorHeightmap::InputAction(int32 phase, bool intersects)
{
    bool dropper = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL);
    if(dropper)
    {
        switch(phase)
        {
            case UIEvent::PHASE_BEGAN:
            case UIEvent::PHASE_DRAG:
            case UIEvent::PHASE_ENDED:
            {
                currentTool->height = GetDropperHeight();
                break;
            }
                
            default:
                break;
        }

    }
    else 
    {
        switch(phase)
        {
            case UIEvent::PHASE_BEGAN:
            {
                editingIsEnabled = true;
                UNDOManager::Instance()->SaveHightmap(heightmap);
                UpdateToolImage();
                
                break;
            }
                
            case UIEvent::PHASE_DRAG:
            {
                if(editingIsEnabled && !intersects)
                {
                    editingIsEnabled = false;
                }
                else if(!editingIsEnabled && intersects)
                {
                    editingIsEnabled = true;
                    UNDOManager::Instance()->SaveHightmap(heightmap);
                    UpdateToolImage();
                }
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
}

void LandscapeEditorHeightmap::HideAction()
{
	landscapeDebugNode->CursorDisable();
    SafeRelease(toolImage);
    
    workingScene->AddNode(workingLandscape);
    workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() & ~SceneNode::DEBUG_DRAW_GRID);
    workingLandscape->BuildLandscapeFromHeightmapImage(workingLandscape->GetRenderingMode(), 
                                                       savedPath,
                                                       workingLandscape->GetBoundingBox());
    
    workingScene->RemoveNode(landscapeDebugNode);
    SafeRelease(landscapeDebugNode);
    
    SafeRelease(heightmap);
}

void LandscapeEditorHeightmap::ShowAction()
{
    prevToolSize = 0.f;
    
    workingScene->RemoveNode(workingLandscape);

    landscapeDebugNode = new LandscapeDebugNode(workingScene);
    landscapeDebugNode->SetName("Landscape");
    landscapeDebugNode->SetHeightmapPath(workingLandscape->GetHeightMapPathname());
    landscapeDebugNode->SetDebugFlags(workingLandscape->GetDebugFlags());

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
    
    landscapeSize = heightmap->Size();

	landscapeDebugNode->CursorEnable();
}

void LandscapeEditorHeightmap::UndoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetLastUNDOAction();
    if(UNDOAction::ACTION_HEIGHTMAP == type)
    {
        UNDOManager::Instance()->UndoHeightmap(heightmap);
        heightmapNode->UpdateHeightmapRect(Rect(0, 0, heightmap->Size()-1, heightmap->Size()-1));
    }
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
    LandscapeEditorPropertyControl *propsControl = 
    (LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingLandscape, rect, LandscapeEditorPropertyControl::HEIGHT_EDITOR_MODE);
    
    propsControl->SetDelegate(this);
    
    return propsControl;
}


#pragma mark -- LandscapeEditorPropertyControlDelegate
void LandscapeEditorHeightmap::LandscapeEditorSettingsChanged(LandscapeEditorSettings *newSettings)
{
}

void LandscapeEditorHeightmap::TextureWillChanged()
{
    if(savedPath.length())
    {
        SaveTextureAction(savedPath);
    }
}

void LandscapeEditorHeightmap::TextureDidChanged()
{
//TODO: Clear history
    
    SafeRelease(heightmap);
    heightmap = SafeRetain(workingLandscape->GetHeightmap());
    savedPath = workingLandscape->GetHeightMapPathname();
    landscapeDebugNode->SetDebugHeightmapImage(heightmap, workingLandscape->GetBoundingBox());
    
    landscapeSize = heightmap->Size();
}

#pragma mark  --LandscapeToolsPanelDelegate
void LandscapeEditorHeightmap::OnToolSelected(LandscapeTool *newTool)
{
    LandscapeEditorBase::OnToolSelected(newTool);
    SafeRelease(toolImage);
}

void LandscapeEditorHeightmap::OnShowGrid(bool show)
{
    LandscapeEditorBase::OnShowGrid(show);
    
    if(landscapeDebugNode)
    {
        landscapeDebugNode->SetDebugFlags(workingLandscape->GetDebugFlags());
    }
}
