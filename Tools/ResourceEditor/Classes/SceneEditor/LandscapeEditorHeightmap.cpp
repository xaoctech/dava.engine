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
    
    editingIsEnabled = false;
    
    copyFromCenter = Vector2(-1.0f, -1.0f);
    copyToCenter = Vector2(-1.0f, -1.0f);
    
    tilemaskWasChanged = false;
    tilemaskImage = NULL;
    tilemaskPathname = "";
    tilemaskTexture = NULL;
    toolImageTile = NULL;
}


LandscapeEditorHeightmap::~LandscapeEditorHeightmap()
{
    SafeRelease(tilemaskImage);
    SafeRelease(toolImageTile);
    SafeRelease(tilemaskTexture);

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
            if((LandscapeTool::TOOL_BRUSH == currentTool->type) || (LandscapeTool::TOOL_COPYPASTE == currentTool->type))
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
            if(LandscapeTool::TOOL_COPYPASTE == currentTool->type)
            {
                SafeRelease(toolImageTile);
            }
        }
    }

    if(currentTool && !toolImage)
    {
        prevToolSize = currentTool->size;

        int32 sideSize = currentTool->size;
        toolImage = CreateToolImage(sideSize);
        
        if(LandscapeTool::TOOL_COPYPASTE == currentTool->type && tilemaskImage)
        {
            float32 multiplier = (float32)tilemaskImage->GetWidth() / (float32)(landscapeSize);
            int32 sideSize = currentTool->size * multiplier;
            toolImageTile = CreateToolImage(sideSize);
        }
    }
}

Image *LandscapeEditorHeightmap::CreateToolImage(int32 sideSize)
{
    RenderManager::Instance()->LockNonMain();
    
    Image *image = currentTool->image;
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
    
    Image *retImage = dstSprite->GetTexture()->CreateImageFromMemory();
    
    SafeRelease(srcSprite);
    SafeRelease(srcTex);
    SafeRelease(dstSprite);
    
    RenderManager::Instance()->UnlockNonMain();
    
    return retImage;
}


void LandscapeEditorHeightmap::UpdateTileMaskTool(float32 timeElapsed)
{
    if(toolImage)
    {
        if(LandscapeTool::TOOL_BRUSH == currentTool->type)
        {
            UpdateBrushTool(timeElapsed);
        }
        else if(LandscapeTool::TOOL_COPYPASTE == currentTool->type)
        {
            UpdateCopypasteTool(timeElapsed);
        }
    }
}

void LandscapeEditorHeightmap::UpdateBrushTool(float32 timeElapsed)
{
    int32 scaleSize = toolImage->GetWidth();
    Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2;
    {
        if(currentTool->averageDrawing)
        {
            float32 koef = (currentTool->averageStrength * timeElapsed) * 2.0f;
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
            
            float32 koef = (currentTool->averageStrength * timeElapsed) * 2.0f;
            ImageRasterizer::DrawAbsoluteRGBA(heightmap, toolImage, pos.x, pos.y, scaleSize, scaleSize, koef, height);
        }
        
        heightmapNode->UpdateHeightmapRect(Rect(pos.x, pos.y, scaleSize, scaleSize));
    }
}

void LandscapeEditorHeightmap::UpdateCopypasteTool(float32 timeElapsed)
{
    if(     (Vector2(-1.0f, -1.0f) != copyFromCenter) 
       &&   (Vector2(-1.0f, -1.0f) != copyToCenter))
    {
        
        if(currentTool->copyHeightmap)
        {
            int32 scaleSize = toolImage->GetWidth();
            Vector2 posTo = landscapePoint - Vector2(scaleSize, scaleSize)/2;
            
            Vector2 deltaPos = landscapePoint - copyToCenter;
            Vector2 posFrom = copyFromCenter + deltaPos - Vector2(scaleSize, scaleSize)/2;
            
            float32 koef = (currentTool->averageStrength * timeElapsed) * 2.0f;
            ImageRasterizer::DrawCopypasteRGBA(heightmap, toolImage, posFrom, posTo, scaleSize, scaleSize, koef);
            
            heightmapNode->UpdateHeightmapRect(Rect(posTo.x, posTo.y, scaleSize, scaleSize));
        }
        
        if(currentTool->copyTilemask)
        {
            if(tilemaskImage && toolImageTile)
            {
                tilemaskWasChanged = true;
                
                float32 multiplier = (float32)tilemaskImage->GetWidth() / (float32)(landscapeSize);
                
                int32 scaleSize = toolImageTile->GetWidth();
                Vector2 posTo = landscapePoint * multiplier - Vector2(scaleSize, scaleSize)/2;
                
                Vector2 deltaPos = landscapePoint - copyToCenter;
                Vector2 posFrom = (copyFromCenter + deltaPos) * multiplier - Vector2(scaleSize, scaleSize)/2;
                
                ImageRasterizer::DrawCopypasteRGBA(tilemaskImage, tilemaskImage, toolImageTile, posFrom, posTo, scaleSize, scaleSize);
                
                Texture *tex = tilemaskTexture;//landscapeDebugNode->GetTexture(LandscapeNode::TEXTURE_TILE_MASK);
                if(tex)
                {
                    tex->TexImage(0, tilemaskImage->GetWidth(), tilemaskImage->GetHeight(), tilemaskImage->GetData());
                    tex->GenerateMipmaps();
                    tex->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
                }
            }
        }
    }
}


float32 LandscapeEditorHeightmap::GetDropperHeight()
{
    Vector3 landSize;
    AABBox3 transformedBox;
    workingLandscape->GetBoundingBox().GetTransformedBox(workingLandscape->GetWorldTransform(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;

    int32 index = landscapePoint.x + landscapePoint.y * heightmap->Size();
    float32 height = heightmap->Data()[index];
    float32 maxHeight = landSize.z;
    return (height / Heightmap::MAX_VALUE * maxHeight);
}

void LandscapeEditorHeightmap::UpdateCursor()
{
	if(currentTool)
	{
		int32 scaleSize = currentTool->size;
		Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2;

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
                if(currentTool->absoluteDropperDrawing)
                {
                    currentTool->height = GetDropperHeight();
                }
                
                if(LandscapeTool::TOOL_COPYPASTE == currentTool->type)
                {
                    CopyPasteBegin();
                }
                
                editingIsEnabled = true;
                UpdateToolImage();
                
                break;
            }
                
            case UIEvent::PHASE_DRAG:
            {
                if(editingIsEnabled && !intersects)
                {
                    editingIsEnabled = false;
                    UNDOManager::Instance()->SaveHightmap(heightmap);
                }
                else if(!editingIsEnabled && intersects)
                {
                    editingIsEnabled = true;
                    UpdateToolImage();
                }
                break;
            }
                
            case UIEvent::PHASE_ENDED:
            {
                editingIsEnabled = false;
                UNDOManager::Instance()->SaveHightmap(heightmap);
 
                break;
            }
                
            default:
                break;
        }
    }
}

void LandscapeEditorHeightmap::CopyPasteBegin()
{
    bool start = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
    if(start)
    {
        copyFromCenter = landscapePoint;
        copyToCenter = Vector2(-1.0f, -1.0f);
    }
    else if(Vector2(-1.0f, -1.0f) == copyToCenter)
    {
        copyToCenter = landscapePoint;
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
    
    if(tilemaskImage && tilemaskWasChanged)
    {
        tilemaskWasChanged = false;
        tilemaskImage->Save(tilemaskPathname);
    }

    SafeRelease(tilemaskImage);
    SafeRelease(toolImageTile);

    workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, tilemaskPathname);

    
    UNDOManager::Instance()->ClearHistory(UNDOAction::ACTION_HEIGHTMAP);
}

void LandscapeEditorHeightmap::ShowAction()
{
    prevToolSize = 0.f;
    
    workingScene->RemoveNode(workingLandscape);

    landscapeDebugNode = new LandscapeDebugNode(workingScene);
    landscapeDebugNode->SetName("Landscape");
    landscapeDebugNode->SetHeightmapPath(workingLandscape->GetHeightmapPathname());
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
    savedPath = workingLandscape->GetHeightmapPathname();
    landscapeDebugNode->SetDebugHeightmapImage(heightmap, workingLandscape->GetBoundingBox());
    
    landscapeSize = heightmap->Size();

	landscapeDebugNode->CursorEnable();
    
    CreateTilemaskImage();
    
    UNDOManager::Instance()->SaveHightmap(heightmap);
}

void LandscapeEditorHeightmap::CreateTilemaskImage()
{
    SafeRelease(tilemaskImage);
    SafeRelease(toolImageTile);
    SafeRelease(tilemaskTexture);

    tilemaskWasChanged = false;
    prevToolSize = -1;
    
    copyFromCenter = Vector2(-1.0f, -1.0f);
    copyToCenter = Vector2(-1.0f, -1.0f);
    
    Texture *mask = workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK);
    if(mask)
    {
        Image::EnableAlphaPremultiplication(false);
        
        tilemaskPathname = mask->GetPathname();
        tilemaskImage = Image::CreateFromFile(tilemaskPathname);
        
        tilemaskTexture = Texture::CreateFromData(tilemaskImage->format, tilemaskImage->GetData(), tilemaskImage->GetWidth(), tilemaskImage->GetHeight());
        
        Image::EnableAlphaPremultiplication(true);
    }
    
    landscapeDebugNode->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, tilemaskTexture);
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

void LandscapeEditorHeightmap::RedoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetFirstREDOAction();
    if(UNDOAction::ACTION_HEIGHTMAP == type)
    {
        UNDOManager::Instance()->RedoHeightmap(heightmap);
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

void LandscapeEditorHeightmap::TextureWillChanged(const String &forKey)
{
    if("property.landscape.texture.heightmap" == forKey)
    {
        if(savedPath.length())
        {
            SaveTextureAction(savedPath);
        }
    }
    else if("property.landscape.texture.tilemask" == forKey) 
    {
        if(tilemaskImage && tilemaskWasChanged)
        {
            tilemaskWasChanged = false;
            tilemaskImage->Save(tilemaskPathname);
        }
    }
}

void LandscapeEditorHeightmap::TextureDidChanged(const String &forKey)
{
    if("property.landscape.texture.heightmap" == forKey)
    {
        SafeRelease(heightmap);
        heightmap = SafeRetain(workingLandscape->GetHeightmap());
        savedPath = workingLandscape->GetHeightmapPathname();
        landscapeDebugNode->SetDebugHeightmapImage(heightmap, workingLandscape->GetBoundingBox());
        
        landscapeSize = heightmap->Size();
        
        UNDOManager::Instance()->ClearHistory(UNDOAction::ACTION_HEIGHTMAP);
        UNDOManager::Instance()->SaveHightmap(heightmap);
    }
    else if("property.landscape.texture.tilemask" == forKey) 
    {
        CreateTilemaskImage();
    }
}

#pragma mark  --LandscapeToolsPanelDelegate
void LandscapeEditorHeightmap::OnToolSelected(LandscapeTool *newTool)
{
    LandscapeEditorBase::OnToolSelected(newTool);
    prevToolSize = -1.0f;
    
    copyFromCenter = Vector2(-1.0f, -1.0f);
    copyToCenter = Vector2(-1.0f, -1.0f);
    
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
