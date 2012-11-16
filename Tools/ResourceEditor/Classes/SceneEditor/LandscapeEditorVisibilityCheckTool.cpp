#include "LandscapeEditorVisibilityCheckTool.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelCustomColors.h"
#include "PropertyControlCreator.h"
#include "ErrorNotifier.h"
#include "EditorScene.h"
#include "EditorConfig.h"

#include "UNDOManager.h"
#include "HeightmapNode.h"

#include "../LandscapeEditor/EditorHeightmap.h"
#include "../LandscapeEditor/EditorLandscapeNode.h"
#include "../Qt/QtMainWindowHandler.h"

#include "EditorBodyControl.h"
#include "cmath"

#define VISIBILITY_POINT_CURSOR_SCALE 0.2f

LandscapeEditorVisibilityCheckTool::LandscapeEditorVisibilityCheckTool(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl),
		state(VCT_STATE_NORMAL),
		isVisibilityPointSet(false),
		visibilityAreaSprite(0)
{
	wasTileMaskToolUpdate = false;

    settings = NULL;

    editedHeightmap = NULL;
    savedHeightmap = NULL;
    
    toolsPanel = new LandscapeToolsPanelCustomColors(this, toolsRect);

    editingIsEnabled = false;

	texSurf = NULL;
	
	isCursorTransparent = false;

	areaCursorTexture = cursorTexture;
	pointCursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/setPointCursor.png");
	pointCursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
	
	PrepareConfig();
}

LandscapeEditorVisibilityCheckTool::~LandscapeEditorVisibilityCheckTool()
{
    SafeRelease(editedHeightmap);
    SafeRelease(savedHeightmap);
	SafeRelease(texSurf);
	SafeRelease(pointCursorTexture);
	SafeRelease(areaCursorTexture);
}

void LandscapeEditorVisibilityCheckTool::Draw(const DAVA::UIGeometricData &geometricData)
{
    if(wasTileMaskToolUpdate)
	{
		PerformLandscapeDraw();
		wasTileMaskToolUpdate = false;
	}
}

void LandscapeEditorVisibilityCheckTool::RecreateVisibilityAreaSprite()
{
	SafeRelease(visibilityAreaSprite);
	visibilityAreaSprite = Sprite::CreateAsRenderTarget(texSurf->width, texSurf->height, FORMAT_RGBA8888);
}

void LandscapeEditorVisibilityCheckTool::PerformLandscapeDraw()
{
	Sprite* visibilityPointSprite = 0;
	if(isVisibilityPointSet)
	{
		visibilityPointSprite = Sprite::CreateFromTexture(pointCursorTexture,
														  0,
														  0,
														  pointCursorTexture->GetWidth(),
														  pointCursorTexture->GetHeight());
	}

	Sprite* sprLandscape = Sprite::CreateFromTexture(texSurf, 0, 0, texSurf->GetWidth(), texSurf->GetHeight());
	Sprite* sprTargetSurf = Sprite::CreateAsRenderTarget(texSurf->GetWidth(), texSurf->GetHeight(), FORMAT_RGBA8888);

    RenderManager::Instance()->SetRenderTarget(sprTargetSurf);
	sprLandscape->Draw();

	if(visibilityPointSprite != 0)
	{
		float32 scale = VISIBILITY_POINT_CURSOR_SCALE * 2;
		Vector2 scaledSize = visibilityPointSprite->GetSize() * scale;
		visibilityPointSprite->SetPosition(visibilityPoint - scaledSize / 2);
		visibilityPointSprite->SetScale(scale, scale);
		visibilityPointSprite->Draw();
	}
	
	if(visibilityAreaSprite != 0)
	{
		visibilityAreaSprite->Draw();
	}
	
    texSurf->GenerateMipmaps();
    RenderManager::Instance()->RestoreRenderTarget();

	workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_FULL, sprTargetSurf->GetTexture());
	
	SafeRelease(sprTargetSurf);
	SafeRelease(sprLandscape);
	
}

void LandscapeEditorVisibilityCheckTool::UpdateCursor()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		float32 scale;
		
		switch(state)
		{
			case VCT_STATE_SET_POINT:
				cursorTexture = pointCursorTexture;
				scale = VISIBILITY_POINT_CURSOR_SCALE;
				break;
				
			case VCT_STATE_SET_AREA:
				cursorTexture = areaCursorTexture;
				scale = (float32)visibilityAreaSize / areaCursorTexture->GetWidth();
				break;

			default:
				scale = 0.f;
				break;
		}

		float32 scaledSize = cursorTexture->GetWidth() * scale;
		Vector2 pos = landscapePoint - Vector2(scaledSize, scaledSize) / 2;

		workingLandscape->SetCursorTexture(cursorTexture);
		workingLandscape->SetBigTextureSize((float32)workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK)->GetWidth());
		workingLandscape->SetCursorPosition(pos);
		workingLandscape->SetCursorScale(scaledSize);
	}
}

void LandscapeEditorVisibilityCheckTool::SetVisibilityAreaSize(uint32 size)
{
	visibilityAreaSize = size;
}

void LandscapeEditorVisibilityCheckTool::SetState(eVisibilityCheckToolState newState)
{
	state = newState;
}

void LandscapeEditorVisibilityCheckTool::SaveColorLayer(const String &pathName)
{
	SaveTextureAction(pathName);
}

void LandscapeEditorVisibilityCheckTool::InputAction(int32 phase, bool intersects)
{
	switch(state)
	{
		case VCT_STATE_SET_POINT:
			wasTileMaskToolUpdate = SetPointInputAction(phase);
			break;
			
		case VCT_STATE_SET_AREA:
			wasTileMaskToolUpdate = SetAreaInputAction(phase);
			break;
			
		default:
			break;
	}
}

bool LandscapeEditorVisibilityCheckTool::SetPointInputAction(int32 phase)
{
	bool res = false;
	
	if(phase == UIEvent::PHASE_ENDED)
	{
		visibilityPoint = landscapePoint * 2;
		isVisibilityPointSet = true;
		state = VCT_STATE_NORMAL;

		RecreateVisibilityAreaSprite();

		res = true;
	}
	
	return res;
}

bool LandscapeEditorVisibilityCheckTool::SetAreaInputAction(int32 phase)
{
	bool res = false;
	
	if(phase == UIEvent::PHASE_ENDED)
	{
		if(isVisibilityPointSet)
		{
			Vector2 visibilityAreaCenter(landscapePoint * 2);
//			workingLandscape->PlacePoint(visibilityAreaCenter, visibilityAreaCenter);

			Vector3 point(visibilityPoint);
			workingLandscape->PlacePoint(point, point);
			point.z += visibilityPointHeight;

//			Vector<Vector3> resPoints = CalculateVisibility(pointsDensity,
//															point,
//															areaPointHeights,
//															visibilityAreaCenter,
//															visibilityAreaSize);

			Vector<Vector3> resP;
			
			PerformHightTest(point, visibilityAreaCenter, visibilityAreaSize, pointsDensity, areaPointHeights, &resP);
			
			DrawVisibilityAreaPoints(resP);
//			DrawVisibilityAreaPoints(resPoints);

			res = true;
		}
		else
		{
			ErrorNotifier::Instance()->ShowError("Cannot check visibility without visibility point is set. Please set visibility point");
		}
	}
	
	return res;
}

void LandscapeEditorVisibilityCheckTool::DrawVisibilityAreaPoints(const Vector<DAVA::Vector3> &points)
{
	DVASSERT(visibilityAreaSprite != 0);

	RenderManager* manager = RenderManager::Instance();
	RenderHelper* helper = RenderHelper::Instance();

	manager->SetRenderTarget(visibilityAreaSprite);

	for(uint32 i = 0; i < points.size(); ++i)
	{
		uint32 colorIndex = (uint32)points[i].z;
		Vector2 pos(points[i].x, points[i].y);

		Color color(1.f, 0.f, 0.f, 1.f);
		if(areaPointColors.size() > colorIndex)
			color = areaPointColors[colorIndex];

		manager->SetColor(areaPointColors[colorIndex]);
		helper->DrawPoint(pos);
	}

	manager->RestoreRenderTarget();
}

void LandscapeEditorVisibilityCheckTool::PrepareConfig()
{
	EditorConfig* config = EditorConfig::Instance();

	pointsDensity = 10.f;
	VariantType* value = config->GetPropertyDefaultValue("LevelVisibilityDensity");
	if(value && config->GetPropertyValueType("LevelVisibilityDensity") == VariantType::TYPE_FLOAT)
		pointsDensity = value->AsFloat();

	visibilityPointHeight = 2.f;
	areaPointHeights.clear();
	const Vector<String>& heights = config->GetComboPropertyValues("LevelVisibilityPointHeights");
	if(heights.size() != 0)
	{
		if(heights.front() != "none")
		{
			std::sscanf(heights[0].c_str(), "%f", &visibilityPointHeight);

			for(uint32 i = 1; i < heights.size(); ++i)
			{
				float32 val;
				std::sscanf(heights[i].c_str(), "%f", &val);
				areaPointHeights.push_back(val);
			}
		}
	}

	areaPointColors = config->GetColorPropertyValues("LevelVisibilityColors");
}

const Vector<Vector3> LandscapeEditorVisibilityCheckTool::CalculateVisibility(float32 density, const Vector3& point, const Vector<float32>& pointsHeight, const Vector3& areaCenter, float32 areaSize)
{
	Vector<Vector3> points;

	Vector2 center(areaCenter.x, areaCenter.y);
	Vector2 a = center - Vector2(areaSize, areaSize);

	for(float32 x = a.x; x < (a.x + areaSize * 2); x += density)
	{
		for(float32 y = a.y; y < (a.y + areaSize * 2); y += density)
		{
			Vector2 b(x, y);
			Vector2 c = b - center;
			if(c.Length() < areaSize)
			{
				Vector3 d(b);
				d.z = rand() % areaPointColors.size();
				points.push_back(d);
			}
		}
	}

	return points;
}

void LandscapeEditorVisibilityCheckTool::PerformHightTest(Vector3 spectatorCoords, Vector2 circleCentr, float circleRadius,
												   float density, const Vector<float>& hightValues, Vector<Vector3>* colorizedPoints)
{
	Vector2 startOfCounting(circleCentr.x - circleRadius, circleCentr.y - circleRadius);
	Vector2 SpectatorCoords2D(spectatorCoords.x, spectatorCoords.y);
	
	//Vector3 spectatorPointTmp;
	//spectatorPointTmp.x = spectatorCoords.x ;
	//spectatorPointTmp.y = spectatorCoords.y ;
	
	// get soource point in propper coords system
	Vector3 sourcePoint;
	bool isIntersect = GetIntersectionPoint(SpectatorCoords2D, sourcePoint);
	
	sourcePoint.z = spectatorCoords.z;
	
	uint32	hight = hightValues.size();
	uint32	sideLength = (circleRadius * 2) / density;
	
	Vector< Vector< Vector< Vector3 > > > points;
	
	for(uint32 layerIndex = 0; layerIndex < hight; ++layerIndex)
	{
		float zOfPoint = hightValues[layerIndex];
		Vector<Vector<Vector3> > xLine;
		for(uint32 x = 0; x < sideLength; ++x)
		{
			float xOfPoint = startOfCounting.x + density * x;
			Vector<Vector3> yLine;
			for(uint32 y = 0; y < sideLength; ++y)
			{
				float yOfPoint = startOfCounting.y + density * y;
				Vector3 pointToInsert(xOfPoint, yOfPoint, zOfPoint);
				yLine.push_back(pointToInsert);
			}
			xLine.push_back(yLine);
		}
		points.push_back(xLine);
	}
	
	colorizedPoints->clear();
	
	for(uint32 x = 0; x < sideLength; ++x)
	{
		for(uint32 y = 0; y < sideLength; ++y)
		{
			for(uint32 layerIndex = 0; layerIndex < hight; ++layerIndex)
			{
				Vector3 targetTmp = points[layerIndex][x][y];
				if(!CheckIsInCircle(circleCentr, circleRadius, Vector2(targetTmp.x, targetTmp.y)))
				{
					continue;
				}
				
				Vector3 target;
				isIntersect = GetIntersectionPoint(Vector2(targetTmp.x,targetTmp.y), target);
				
				target.z = targetTmp.z;
				
				bool res = parent->GetScene()->TryIsTargetAccesible(sourcePoint, target);
				//DAVA::Logger::Debug("Trace test: from source [%f, %f, %f] to point [%f, %f, %f] - %s",
				//	sourcePointTmp.x,sourcePointTmp.y,sourcePoint.z,
				//	targetTmp.x, targetTmp.y, targetTmp.z,
				//	(res)?"true":"false");
				if(res)
				{
					Vector3 exportData(targetTmp.x, targetTmp.y,(float)layerIndex);
					//tmp
//					DAVA::Logger::Debug("OutputContent: %f, %f, %f",
//										exportData.x,exportData.y,exportData.z);
					//
					colorizedPoints->push_back(exportData);
					
					break;
				}
			}
		}
	}
}

bool LandscapeEditorVisibilityCheckTool::GetIntersectionPoint(const DAVA::Vector2 &touchPoint, DAVA::Vector3 &pointOnLandscape)
{
    DVASSERT(parent);
	
    Vector3 from, dir;
    parent->GetCursorVectors(&from, &dir, touchPoint);
    Vector3 to = from + dir * 1000;
	
    bool isIntersect = workingScene->LandscapeIntersection(from, to, pointOnLandscape);
    return isIntersect;
}

bool LandscapeEditorVisibilityCheckTool::CheckIsInCircle(Vector2 circleCentre, float radius, Vector2 targetCoord)
{
	float arg1 = targetCoord.x - circleCentre.x;
	float arg2 = targetCoord.y - circleCentre.y;
	return  ( pow(arg1, 2) + pow( arg2, 2) ) < ( pow(radius,2) ) ;
}

void LandscapeEditorVisibilityCheckTool::HideAction()
{
	workingLandscape->CursorDisable();
	
    workingLandscape->SetHeightmap(savedHeightmap);
    SafeRelease(editedHeightmap);
    SafeRelease(savedHeightmap);

	SafeRelease(texSurf);
	
	QtMainWindowHandler::Instance()->SetWidgetsStateVisibilityTool(false);
}

void LandscapeEditorVisibilityCheckTool::ShowAction()
{
    landscapeSize = settings->maskSize;

	workingLandscape->CursorEnable();

	texSurf = SafeRetain( workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL)); 

	Texture* texSpr = currentTool->sprite->GetTexture();

	PerformLandscapeDraw();

	UNDOManager::Instance()->ClearHistory(UNDOAction::ACTION_COLORIZE);
//	UNDOManager::Instance()->SaveColorize(colorSprite->GetTexture());

    savedHeightmap = SafeRetain(workingLandscape->GetHeightmap());
    editedHeightmap = new EditorHeightmap(savedHeightmap);
    workingLandscape->SetHeightmap(editedHeightmap);

	QtMainWindowHandler::Instance()->SetWidgetsStateVisibilityTool(true);
}

void LandscapeEditorVisibilityCheckTool::UndoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetLastUNDOAction();
    if(UNDOAction::ACTION_COLORIZE == type)
    {
        Image::EnableAlphaPremultiplication(false);
        
        Texture *tex = UNDOManager::Instance()->UndoColorize();
        
        Image::EnableAlphaPremultiplication(true);
//		SafeRelease(colorSprite);
//		colorSprite = Sprite::CreateAsRenderTarget(texSurf->width, texSurf->height, FORMAT_RGBA8888);
		Sprite* restSprite = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);

//		RenderManager::Instance()->SetRenderTarget(colorSprite);
	
		restSprite->Draw();
		
		SafeRelease(restSprite);

		RenderManager::Instance()->RestoreRenderTarget();

		PerformLandscapeDraw();
    }
}

void LandscapeEditorVisibilityCheckTool::RedoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetFirstREDOAction();
    if(UNDOAction::ACTION_COLORIZE == type)
    {
        Image::EnableAlphaPremultiplication(false);
        
        Texture *tex = UNDOManager::Instance()->RedoColorize();
        
        Image::EnableAlphaPremultiplication(true);

//		SafeRelease(colorSprite);
//		colorSprite = Sprite::CreateAsRenderTarget(texSurf->width, texSurf->height, FORMAT_RGBA8888);
		Sprite* restSprite = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);

//		RenderManager::Instance()->SetRenderTarget(colorSprite);
	
		restSprite->Draw();
		
		SafeRelease(restSprite);

		RenderManager::Instance()->RestoreRenderTarget();

		PerformLandscapeDraw();
    }
}

void LandscapeEditorVisibilityCheckTool::SaveTextureAction(const String &pathToFile)
{
    if(visibilityAreaSprite)
    {
        Image *img = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
        if(img)
        {
            img->Save(pathToFile);
            SafeRelease(img);
        }
    }
}

NodesPropertyControl *LandscapeEditorVisibilityCheckTool::GetPropertyControl(const Rect &rect)
{
    LandscapeEditorPropertyControl *propsControl = 
		(LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingLandscape, rect, LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE);

	workingLandscape->SetTiledShaderMode(LandscapeNode::TILED_MODE_TEXTURE);

    propsControl->SetDelegate(this);

    LandscapeEditorSettingsChanged(propsControl->Settings());


    return propsControl;
}

void LandscapeEditorVisibilityCheckTool::LandscapeEditorSettingsChanged(LandscapeEditorSettings *newSettings)
{
    settings = newSettings;
}

void LandscapeEditorVisibilityCheckTool::TextureWillChanged(const String &forKey)
{}

void LandscapeEditorVisibilityCheckTool::TextureDidChanged(const String &forKey)
{}

void LandscapeEditorVisibilityCheckTool::RecreateHeightmapNode()
{
    if(workingScene && heightmapNode)
    {
        workingScene->RemoveNode(heightmapNode);
    }
        
    SafeRelease(heightmapNode);
    heightmapNode = new HeightmapNode(workingScene, workingLandscape);
    workingScene->AddNode(heightmapNode);
}

bool LandscapeEditorVisibilityCheckTool::SetScene(EditorScene *newScene)
{
    EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(newScene->GetLandScape(newScene));
    if(editorLandscape)
    {
        ErrorNotifier::Instance()->ShowError("Cannot start Visibility Check Tool. Remove EditorLandscapeNode from scene");
        return false;
    }
    
    return LandscapeEditorBase::SetScene(newScene);
}

