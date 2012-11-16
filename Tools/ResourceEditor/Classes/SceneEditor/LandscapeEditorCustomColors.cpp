#include "LandscapeEditorCustomColors.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelCustomColors.h"
#include "PropertyControlCreator.h"
#include "ErrorNotifier.h"
#include "EditorScene.h"

#include "UNDOManager.h"
#include "HeightmapNode.h"

#include "../LandscapeEditor/EditorHeightmap.h"
#include "../LandscapeEditor/EditorLandscapeNode.h"
#include "../Qt/QtMainWindowHandler.h"



LandscapeEditorCustomColors::LandscapeEditorCustomColors(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
	wasTileMaskToolUpdate = false;

    settings = NULL;

    editedHeightmap = NULL;
    savedHeightmap = NULL;
    
    toolsPanel = new LandscapeToolsPanelCustomColors(this, toolsRect);

    editingIsEnabled = false;

	paintColor = Color(1.f, 1.f, 1.f, 1.0f);

	colorSprite = NULL;
	texSurf = NULL;
	circleTexture = NULL;

	
	isCursorTransparent = false;

	radius = 64;
	UpdateCircleTexture(false);
	//
}

LandscapeEditorCustomColors::~LandscapeEditorCustomColors()
{
    SafeRelease(editedHeightmap);
    SafeRelease(savedHeightmap);
	SafeRelease(texSurf);
	SafeRelease(colorSprite);
	SafeRelease(circleTexture);
}


void LandscapeEditorCustomColors::Draw(const DAVA::UIGeometricData &geometricData)
{
    if(wasTileMaskToolUpdate)
	{
		PrepareRenderLayers();
		PerformLandscapeDraw();
		wasTileMaskToolUpdate = false;
	}
}

void LandscapeEditorCustomColors::PrepareRenderLayers()
{
	UpdateCircleTexture(false);

    Sprite* blankSprite = Sprite::CreateFromTexture(circleTexture, 0, 0, (float32)circleTexture->width, (float32)circleTexture->height);
    
	//fill color sprite to get opportunity to save its texture separately 
	RenderManager::Instance()->SetRenderTarget(colorSprite);
	
	Vector2 newPoint = workingLandscape->GetCursor()->GetCursorPosition(); 
    newPoint *= 2; 
	RenderManager::Instance()->SetColor(paintColor);
	
    blankSprite->SetPosition(newPoint); 
	blankSprite->Draw();
	RenderManager::Instance()->RestoreRenderTarget();
    RenderManager::Instance()->ResetColor();
	SafeRelease(blankSprite);
	
}

void LandscapeEditorCustomColors::PerformLandscapeDraw()
{
	Sprite* sprLandscape = Sprite::CreateFromTexture(texSurf, 0, 0, texSurf->GetWidth(), texSurf->GetHeight());
	Sprite* sprTargetSurf = Sprite::CreateAsRenderTarget(texSurf->width, texSurf->height, FORMAT_RGBA8888);
	//render original and color layer to final container 
    RenderManager::Instance()->SetRenderTarget(sprTargetSurf);
	sprLandscape->Draw();
    colorSprite->Draw();
    texSurf->GenerateMipmaps();
    RenderManager::Instance()->RestoreRenderTarget();

	workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_FULL, sprTargetSurf->GetTexture());
	
	SafeRelease(sprTargetSurf);
	SafeRelease(sprLandscape);
	
}

//Bresenham's algorithm
void LandscapeEditorCustomColors::DrawCircle(Vector<Vector<bool> >& matrixForCircle)
{
	const int matrSize = matrixForCircle.size();
	int x0 = matrSize/2 - 1;
	int y0 = matrSize/2 - 1 ;
	int radius = matrSize/2 - 2 ;

	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	matrixForCircle[x0][ y0 + radius] = true;
	matrixForCircle[x0][ y0 - radius] = true;
	matrixForCircle[x0 + radius][ y0] = true;
	matrixForCircle[x0 - radius][ y0] =	true;
	
	while(x < y)
	{
		if(f >= 0) 
		{
		  y--;
		  ddF_y += 2;
		  f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;    
		matrixForCircle[x0 + x][ y0 + y] = true;
		matrixForCircle[x0 - x][ y0 + y] = true;
		matrixForCircle[x0 + x][ y0 - y] = true;
		matrixForCircle[x0 - x][ y0 - y] = true;
		matrixForCircle[x0 + y][ y0 + x] = true;
		matrixForCircle[x0 - y][ y0 + x] = true;
		matrixForCircle[x0 + y][ y0 - x] = true;
		matrixForCircle[x0 - y][ y0 - x] = true;
	}
	for (int i = 0; i< matrSize; ++i)
	{
		int startOfBlackBlock = 0;
		int endOfBlackBlock = matrSize - 1;
		for (int j = startOfBlackBlock; j < matrSize; ++j)
		{
			if(matrixForCircle[i][j])
			{
				startOfBlackBlock = j;
				break;
			}
			startOfBlackBlock = endOfBlackBlock;
		}

		for (int j = endOfBlackBlock; j > startOfBlackBlock; --j)
		{
			if(matrixForCircle[i][j])
			{
				endOfBlackBlock = j;
				break;
			}
		}

		for (int j = startOfBlackBlock; j < endOfBlackBlock; ++j)
		{
			matrixForCircle[i][j] = true;
		}
	}
}

uint8*	LandscapeEditorCustomColors::DrawFilledCircleWithFormat(uint32 radius, DAVA::PixelFormat format, bool setTransparent)
{
	if(FORMAT_RGBA8888 != format || radius == 0)
		return NULL;

    uint32 texFormatSize = 4;
    uint32 size = 4 * radius * radius * texFormatSize;
    uint8* texArr = new uint8[size];
	
    memset(texArr, 0, size);

	Vector<Vector<bool> > matrixForCircle;
	
	for (uint32 i = 0; i < radius*2; ++i)
	{
		//Vector<float> tmp(10) - get error, with std::vector ok
		Vector<bool> tmp;
		tmp.resize(radius*2);
		matrixForCircle.push_back(tmp);
	}
	

	DrawCircle(matrixForCircle);
	
	
	for(uint32 i = 0; i <matrixForCircle.size(); ++i)
	{
		for(uint32 j = 0; j < matrixForCircle.size(); ++j)
		{
			if(matrixForCircle[i][j])
			{
				uint32 lineNumber = i * matrixForCircle.size() + j;
				uint32 blockOffset = lineNumber*4;
				texArr[blockOffset]		= 0xff;
				texArr[blockOffset + 1] = 0xff;
				texArr[blockOffset + 2] = 0xff;
				if(setTransparent)
				{
					texArr[blockOffset + 3] = 0x00;
				}
				else
				{
					texArr[blockOffset + 3] = 0xff;
				}
				
				
			}
		}
	}

	isCursorTransparent = setTransparent;

	return texArr;
}
/*
void LandscapeEditorCustomColors::UpdateTool()
{
	wasTileMaskToolUpdate = true;
	return;
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		float32 scaleSize = currentTool->sprite->GetWidth() * (currentTool->size * currentTool->size);
        Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2;
        if(pos != prevDrawPos)
        {
            wasTileMaskToolUpdate = true;

			UpdateCircleTexture();
			
			Sprite* blankSprite = Sprite::CreateFromTexture(circleTexture, 0, 0, (float32)circleTexture->width, (float32)circleTexture->height);
			
			//fill color sprite to get opportunity to save its texture separately 
			RenderManager::Instance()->SetRenderTarget(currentTool->sprite);
			RenderManager::Instance()->SetColor(paintColor);
			
			blankSprite->Draw();//
			RenderManager::Instance()->RestoreRenderTarget();
			RenderManager::Instance()->ResetColor();
			SafeRelease(blankSprite);

			currentTool->sprite->SetPosition(pos);
			
        }
	}
}*/

void LandscapeEditorCustomColors::UpdateCursor()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		//isCursorTransparent = false;
		Vector2 pos = landscapePoint - Vector2(radius, radius)/2;
		UpdateCircleTexture(false);
		workingLandscape->SetCursorTexture(circleTexture);
		workingLandscape->SetBigTextureSize((float32)workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK)->GetWidth());
		workingLandscape->SetCursorPosition(pos);
		workingLandscape->SetCursorScale(radius);
	}
}

void LandscapeEditorCustomColors::SetRadius(int _radius)
{
	//isCursorTransparent = true;
	radius = _radius;
	UpdateCircleTexture(true);
	
}

void LandscapeEditorCustomColors::SetColor(const Color &newColor)
{
	paintColor = newColor;
}

void LandscapeEditorCustomColors::SaveColorLayer(const String &pathName)
{
	SaveTextureAction(pathName);
}

void LandscapeEditorCustomColors::UpdateCircleTexture(bool setTransparent)
{
	if(isCursorTransparent == setTransparent)
	{
		if(NULL != circleTexture  )
		{
			return;
		}
	}
	uint8* texArr = DrawFilledCircleWithFormat(radius, FORMAT_RGBA8888, setTransparent);
	if(!texArr)
	{
		return;
	}
	SafeRelease(circleTexture);
	circleTexture = Texture::CreateFromData(FORMAT_RGBA8888, texArr, radius*2, radius*2);
	//check addref
	delete[] texArr;
}

void LandscapeEditorCustomColors::InputAction(int32 phase, bool intersects)
{
    switch(phase)
    {
        case UIEvent::PHASE_BEGAN:
        {
            editingIsEnabled = true;
            break;
        }
            
        case UIEvent::PHASE_DRAG:
        {
            if(editingIsEnabled && !intersects)
            {
                editingIsEnabled = false;
				UNDOManager::Instance()->SaveColorize(colorSprite->GetTexture());
            }
            else if(!editingIsEnabled && intersects)
            {
                editingIsEnabled = true;
            }
            break;
        }
            
        case UIEvent::PHASE_ENDED:
        {
            editingIsEnabled = false;
			UNDOManager::Instance()->SaveColorize(colorSprite->GetTexture());
            break;
        }
            
        default:
            break;
    }
    
    UpdateCircleTexture(false);
	wasTileMaskToolUpdate = true;
}

void LandscapeEditorCustomColors::HideAction()
{
	workingLandscape->CursorDisable();
	
    workingLandscape->SetHeightmap(savedHeightmap);
    SafeRelease(editedHeightmap);
    SafeRelease(savedHeightmap);

	//restore tool
	//RenderManager::Instance()->SetRenderTarget(currentTool->sprite);
	//RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	//currentToolSprite->Draw();//
	//RenderManager::Instance()->RestoreRenderTarget();
	
	//SafeRelease(currentToolSprite);
	//currentToolSprite = NULL;

	SafeRelease(texSurf);
	SafeRelease(circleTexture);
	
	QtMainWindowHandler::Instance()->SetCustomColorsWidgetsState(false);
}

void LandscapeEditorCustomColors::ShowAction()
{
	//
    landscapeSize = settings->maskSize;

	workingLandscape->CursorEnable();

	texSurf = SafeRetain( workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL)); 

	
	Texture* texSpr = currentTool->sprite->GetTexture();
	//currentToolSprite =  Sprite::CreateAsRenderTarget(texSpr->width, texSpr->height, FORMAT_RGBA8888);
	//RenderManager::Instance()->SetRenderTarget(currentToolSprite);
	//currentTool->sprite->Draw();//
	//RenderManager::Instance()->RestoreRenderTarget();

	if(NULL == colorSprite)
	{
		Texture* tex =  workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL); 
		colorSprite = Sprite::CreateAsRenderTarget(tex->width, tex->height, FORMAT_RGBA8888);
	}
	else
	{
		PerformLandscapeDraw();
	}
	
	//
	UNDOManager::Instance()->ClearHistory(UNDOAction::ACTION_COLORIZE);
	UNDOManager::Instance()->SaveColorize(colorSprite->GetTexture());
    
    savedHeightmap = SafeRetain(workingLandscape->GetHeightmap());
    editedHeightmap = new EditorHeightmap(savedHeightmap);
    workingLandscape->SetHeightmap(editedHeightmap);

	QtMainWindowHandler::Instance()->SetCustomColorsWidgetsState(true);
}

void LandscapeEditorCustomColors::UndoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetLastUNDOAction();
    if(UNDOAction::ACTION_COLORIZE == type)
    {
        Image::EnableAlphaPremultiplication(false);
        
        Texture *tex = UNDOManager::Instance()->UndoColorize();
        
        Image::EnableAlphaPremultiplication(true);
		SafeRelease(colorSprite);
		colorSprite = Sprite::CreateAsRenderTarget(texSurf->width, texSurf->height, FORMAT_RGBA8888);
		Sprite* restSprite = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);

		RenderManager::Instance()->SetRenderTarget(colorSprite);
	
		restSprite->Draw();
		
		SafeRelease(restSprite);

		RenderManager::Instance()->RestoreRenderTarget();

		PerformLandscapeDraw();
    }
}

void LandscapeEditorCustomColors::RedoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetFirstREDOAction();
    if(UNDOAction::ACTION_COLORIZE == type)
    {
        Image::EnableAlphaPremultiplication(false);
        
        Texture *tex = UNDOManager::Instance()->RedoColorize();
        
        Image::EnableAlphaPremultiplication(true);

		SafeRelease(colorSprite);
		colorSprite = Sprite::CreateAsRenderTarget(texSurf->width, texSurf->height, FORMAT_RGBA8888);
		Sprite* restSprite = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);

		RenderManager::Instance()->SetRenderTarget(colorSprite);
	
		restSprite->Draw();
		
		SafeRelease(restSprite);

		RenderManager::Instance()->RestoreRenderTarget();

		PerformLandscapeDraw();
    }
}

void LandscapeEditorCustomColors::SaveTextureAction(const String &pathToFile)
{
    if(colorSprite)
    {
        Image *img = colorSprite->GetTexture()->CreateImageFromMemory();   
        if(img)
        {
            img->Save(pathToFile);
            SafeRelease(img);
        }
    }
}

NodesPropertyControl *LandscapeEditorCustomColors::GetPropertyControl(const Rect &rect)
{
    LandscapeEditorPropertyControl *propsControl = 
		(LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingLandscape, rect, LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE);

	workingLandscape->SetTiledShaderMode(LandscapeNode::TILED_MODE_TEXTURE);

    propsControl->SetDelegate(this);

    LandscapeEditorSettingsChanged(propsControl->Settings());


    return propsControl;
}

void LandscapeEditorCustomColors::LandscapeEditorSettingsChanged(LandscapeEditorSettings *newSettings)
{
    settings = newSettings;
}

void LandscapeEditorCustomColors::TextureWillChanged(const String &forKey)
{}

void LandscapeEditorCustomColors::TextureDidChanged(const String &forKey)
{}

void LandscapeEditorCustomColors::RecreateHeightmapNode()
{
    if(workingScene && heightmapNode)
    {
        workingScene->RemoveNode(heightmapNode);
    }
        
    SafeRelease(heightmapNode);
    heightmapNode = new HeightmapNode(workingScene, workingLandscape);
    workingScene->AddNode(heightmapNode);
}

bool LandscapeEditorCustomColors::SetScene(EditorScene *newScene)
{
    EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(newScene->GetLandScape(newScene));
    if(editorLandscape)
    {
        ErrorNotifier::Instance()->ShowError("Cannot start color editor. Remove EditorLandscapeNode from scene");
        return false;
    }
    
    return LandscapeEditorBase::SetScene(newScene);
}

