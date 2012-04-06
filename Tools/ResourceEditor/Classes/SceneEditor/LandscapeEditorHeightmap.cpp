#include "LandscapeEditorHeightmap.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelHeightmap.h"
#include "PropertyControlCreator.h"

#include "../EditorScene.h"


#pragma mark --LandscapeEditorColor
LandscapeEditorHeightmap::LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, 
                                           EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
	wasTileMaskToolUpdate = false;

    landscapeDebugNode = NULL;
    heightImage = NULL;
    
    toolsPanel = new LandscapeToolsPanelHeightmap(this, toolsRect);
}

LandscapeEditorHeightmap::~LandscapeEditorHeightmap()
{
    SafeRelease(heightImage);
    SafeRelease(landscapeDebugNode);
}

void LandscapeEditorHeightmap::Draw(const UIGeometricData &geometricData)
{
    
}


void LandscapeEditorHeightmap::CreateMaskTexture()
{
    SafeRelease(heightImage);
    
    heightImage = Image::CreateFromFile(workingLandscape->GetHeightMapPathname());
    landscapeDebugNode->SetDebugHeightmapImage(heightImage);
}



void LandscapeEditorHeightmap::UpdateTileMaskTool()
{
//	if(currentTool && currentTool->sprite && currentTool->zoom)
//	{
//		float32 scaleSize = currentTool->sprite->GetWidth() * (currentTool->zoom * currentTool->zoom);
//		Vector2 deltaPos = endPoint - startPoint;
//		{
//			Vector2 pos = startPoint - Vector2(scaleSize, scaleSize)/2;
//			if(pos != prevDrawPos)
//			{
//				wasTileMaskToolUpdate = true;
//                
//				RenderManager::Instance()->SetRenderTarget(toolSprite);
//				currentTool->sprite->SetScaleSize(scaleSize, scaleSize);
//				currentTool->sprite->SetPosition(pos);
//				currentTool->sprite->Draw();
//				RenderManager::Instance()->RestoreRenderTarget();
//			}
//			startPoint = endPoint;
//		}
//	}
}

void LandscapeEditorHeightmap::InputAction()
{
//    UpdateTileMaskTool(); 
}

void LandscapeEditorHeightmap::HideAction()
{
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

}

NodesPropertyControl *LandscapeEditorHeightmap::GetPropertyControl(const Rect &rect)
{
    NodesPropertyControl *propsControl = 
    PropertyControlCreator::Instance()->CreateControlForNode(workingLandscape, rect, false);
    
    return propsControl;
}

