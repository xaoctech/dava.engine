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
}

LandscapeEditorVisibilityCheckTool::~LandscapeEditorVisibilityCheckTool()
{
    SafeRelease(editedHeightmap);
    SafeRelease(savedHeightmap);
	SafeRelease(texSurf);
	SafeRelease(pointCursorTexture);
	SafeRelease(visibilityAreaSprite);
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
	
	if(visibilityAreaSprite != 0)
	{
		visibilityAreaSprite->Draw();
	}

	if(visibilityPointSprite != 0)
	{
		float32 scale = VISIBILITY_POINT_CURSOR_SCALE * 2;
		Vector2 scaledSize = visibilityPointSprite->GetSize() * scale;
		visibilityPointSprite->SetPosition(visibilityPoint - scaledSize / 2);
		visibilityPointSprite->SetScale(scale, scale);
		visibilityPointSprite->Draw();
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
	if(state == newState)
		state = VCT_STATE_NORMAL;
	else
		state = newState;

	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	switch(state)
	{
		case VCT_STATE_SET_POINT:
			handler->SetPointButtonStateVisibilityTool(true);
			handler->SetAreaButtonStateVisibilityTool(false);
			break;
			
		case VCT_STATE_SET_AREA:
			handler->SetPointButtonStateVisibilityTool(false);
			handler->SetAreaButtonStateVisibilityTool(true);
			break;
			
		default:
			handler->SetPointButtonStateVisibilityTool(false);
			handler->SetAreaButtonStateVisibilityTool(false);
			break;
	}
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

		Rect landRect(0, 0, visibilityAreaSprite->GetWidth(), visibilityAreaSprite->GetHeight());
		if(landRect.PointInside(visibilityPoint))
		{
			isVisibilityPointSet = true;
			SetState(VCT_STATE_NORMAL);

			Image* undoImage = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
			RecreateVisibilityAreaSprite();

			UNDOManager::Instance()->SaveVisibilityPoint(undoImage, isVisibilityPointSet, visibilityPoint);

			res = true;
		}
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
			Rect landRect(0, 0, visibilityAreaSprite->GetWidth(), visibilityAreaSprite->GetHeight());
			Vector2 visibilityAreaCenter(landscapePoint * 2);
			
			if(landRect.PointInside(visibilityAreaCenter))
			{
				Vector3 point(visibilityPoint);
				point.z = GetLandscapeHeightFromCursorPos(visibilityPoint);
				point.z += visibilityPointHeight;

				Vector<Vector3> resP;
				PerformHightTest(point, visibilityAreaCenter, visibilityAreaSize, pointsDensity, areaPointHeights, &resP);

				Rect2i rect;
				rect.SetSize(Size2i(visibilityAreaSize * 2 + 10, visibilityAreaSize * 2 + 10));
				rect.SetCenter(Point2i((int32)visibilityAreaCenter.x, (int32)visibilityAreaCenter.y));

				Image* undoTextureImage = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
				rect = FitRectToImage(undoTextureImage, rect);

				Image* undoImage = Image::Create(rect.dx, rect.dy, undoTextureImage->GetPixelFormat());
				CopyImageRectToImage(undoTextureImage, rect, undoImage, Point2i(0, 0));

				DrawVisibilityAreaPoints(resP);

				Image* redoTextureImage = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
				Image* redoImage = Image::Create(rect.dx, rect.dy, redoTextureImage->GetPixelFormat());
				CopyImageRectToImage(redoTextureImage, rect, redoImage, Point2i(0, 0));

				UNDOManager::Instance()->SaveVisibilityArea(undoImage, redoImage, rect.GetPosition(), isVisibilityPointSet, visibilityPoint);

				SafeRelease(undoTextureImage);
				SafeRelease(redoTextureImage);

				res = true;
			}
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
	if(hightValues.size() == 0)
	{
		return;
	}
	Vector2 startOfCounting(circleCentr.x - circleRadius, circleCentr.y - circleRadius);
	Vector2 SpectatorCoords2D(spectatorCoords.x, spectatorCoords.y);

	
	// get soource point in propper coords system
	Vector3 sourcePoint(ConvertToLanscapeSystem(SpectatorCoords2D)) ;
	
	sourcePoint.z = spectatorCoords.z;
	
	uint32	hight = hightValues.size();
	uint32	sideLength = (circleRadius * 2) / density;
	
	Vector< Vector< Vector< Vector3 > > > points;
	
	for(uint32 layerIndex = 0; layerIndex < hight; ++layerIndex)
	{
		Vector<Vector<Vector3> > xLine;
		for(uint32 x = 0; x < sideLength; ++x)
		{
			float xOfPoint = startOfCounting.x + density * x;
			Vector<Vector3> yLine;
			for(uint32 y = 0; y < sideLength; ++y)
			{
				float yOfPoint = startOfCounting.y + density * y;
				float32 zOfPoint = GetLandscapeHeightFromCursorPos(Vector2(xOfPoint, yOfPoint)) + hightValues[layerIndex];
				Vector3 pointToInsert(xOfPoint, yOfPoint, zOfPoint);
				yLine.push_back(pointToInsert);
			}
			xLine.push_back(yLine);
		}
		points.push_back(xLine);
	}
	
	colorizedPoints->clear();
	parent->GetScene()->JuncCollWorldToLandscapeCollWorld();
	for(uint32 x = 0; x < sideLength; ++x)
	{
		for(uint32 y = 0; y < sideLength; ++y)
		{
			Vector3 target(ConvertToLanscapeSystem( Vector2(points[0][x][y].x,points[0][x][y].y) )) ;

			bool prevRes = false;
			for(int32 layerIndex = hight - 1; layerIndex >= 0; --layerIndex)
			{
				Vector3 targetTmp = points[layerIndex][x][y];
				if(!CheckIsInCircle(circleCentr, circleRadius, Vector2(targetTmp.x, targetTmp.y)))
				{
					continue;
				}
				
				target.z = targetTmp.z;
				bool res = parent->GetScene()->TryIsTargetAccesible(sourcePoint, target);
				float colorIndex =0;
				if(prevRes)
				{
					if(!res)
					{
						colorIndex = (float)layerIndex+2; // +1 - because need layer from prev loop, and +1 - 'cause the first color reserved for  "death zone" 
						Vector3 exportData(targetTmp.x, targetTmp.y, colorIndex);
						colorizedPoints->push_back(exportData);
						break;
					}
					else
					{
						if(layerIndex == 0)
						{
							colorIndex = 1;
							Vector3 exportData(targetTmp.x, targetTmp.y, colorIndex);
							colorizedPoints->push_back(exportData);
						}
					}
				}
				else
				{
					if(layerIndex == hight - 1 && !res)
					{
						colorIndex = 0;
						Vector3 exportData(targetTmp.x, targetTmp.y, colorIndex );
						colorizedPoints->push_back(exportData);
						break;
					}
				}
				prevRes = res;
			}
		}
	}
	parent->GetScene()->SeparateCollWorldFromLandscapeCollWorld();
}

Vector2 LandscapeEditorVisibilityCheckTool::TranslatePoint(const Vector2& point, const Rect& fromRect, const Rect& toRect)
{
	DVASSERT(fromRect.dx != 0 && fromRect.dy != 0);
	
	Vector2 origRectSize = fromRect.GetSize();
	Vector2 destRectSize = toRect.GetSize();
	
	Vector2 scale(destRectSize.x / origRectSize.x,
	     destRectSize.y / origRectSize.y);
	
	Vector2 relPos = point - fromRect.GetPosition();
	Vector2 newRelPos(relPos.x * scale.x,
	      relPos.y * scale.y);
	
	Vector2 newPos = newRelPos + toRect.GetPosition();
	
	return newPos;
}

float32 LandscapeEditorVisibilityCheckTool::GetLandscapeHeightFromCursorPos(const Vector2& point)
{
	Vector3 landscapePoint(ConvertToLanscapeSystem(point));
	
	workingLandscape->PlacePoint(landscapePoint, landscapePoint);

	return landscapePoint.z;
}

Vector2 LandscapeEditorVisibilityCheckTool::ConvertToLanscapeSystem(const Vector2& point)
{
	AABBox3 boundingBox = workingLandscape->GetBoundingBox();
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,
	     (boundingBox.max - boundingBox.min).y);
	
	Rect landRect(landPos, landSize);
	Rect textRect(Vector2(0, 0), visibilityAreaSprite->GetSize());
	
	return TranslatePoint(point, textRect, landRect);
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
	return (targetCoord - circleCentre).Length() < radius;
}

void LandscapeEditorVisibilityCheckTool::HideAction()
{
	workingLandscape->CursorDisable();
	
    workingLandscape->SetHeightmap(savedHeightmap);
    SafeRelease(editedHeightmap);
    SafeRelease(savedHeightmap);

	SafeRelease(texSurf);
	
	SetState(VCT_STATE_NORMAL);
	QtMainWindowHandler::Instance()->SetWidgetsStateVisibilityTool(false);
}

void LandscapeEditorVisibilityCheckTool::ShowAction()
{
	SetState(VCT_STATE_NORMAL);

	PrepareConfig();

    landscapeSize = settings->maskSize;

	workingLandscape->CursorEnable();

	texSurf = SafeRetain( workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL));

	PerformLandscapeDraw();
	if(visibilityAreaSprite == 0)
		RecreateVisibilityAreaSprite();

	UNDOManager::Instance()->ClearHistory(UNDOAction::ACTION_VISIBILITY_AREA);
	UNDOManager::Instance()->ClearHistory(UNDOAction::ACTION_VISIBILITY_POINT);

	UNDOManager::Instance()->SaveVisibilityArea(0, 0, Point2i(0, 0), isVisibilityPointSet, visibilityPoint);

	SafeRelease(editedHeightmap);
	SafeRelease(savedHeightmap);

    savedHeightmap = SafeRetain(workingLandscape->GetHeightmap());
    editedHeightmap = new EditorHeightmap(savedHeightmap);
    workingLandscape->SetHeightmap(editedHeightmap);
	
	SafeRelease(editedHeightmap);

	QtMainWindowHandler::Instance()->SetWidgetsStateVisibilityTool(true);
}

void LandscapeEditorVisibilityCheckTool::UndoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetLastUNDOAction();
	if(UNDOAction::ACTION_VISIBILITY_AREA == type || UNDOAction::ACTION_VISIBILITY_POINT == type)
    {
		Image* undoImage;
		Point2i imagePos;
		bool pointSet;
		Vector2 point;

		UNDOManager::Instance()->UndoVisibility(&undoImage, &imagePos, &pointSet, &point);
		
		if(undoImage)
		{
			Rect2i rect(0, 0, undoImage->GetWidth(), undoImage->GetHeight());
			Image* image = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
			CopyImageRectToImage(undoImage, rect, image, imagePos);

			Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight());
			Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());

			RecreateVisibilityAreaSprite();
			RenderManager::Instance()->SetRenderTarget(visibilityAreaSprite);
			sprite->Draw();
			RenderManager::Instance()->RestoreRenderTarget();
			
			SafeRelease(sprite);
			SafeRelease(texture);
			SafeRelease(image);
		}

		isVisibilityPointSet = pointSet;
		visibilityPoint = point;

		PerformLandscapeDraw();
	}
}

void LandscapeEditorVisibilityCheckTool::RedoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetFirstREDOAction();
	if(UNDOAction::ACTION_VISIBILITY_AREA == type)
    {
		Image* redoImage;
		Point2i imagePos;
		bool pointSet;
		Vector2 point;

		UNDOManager::Instance()->RedoVisibility(&redoImage, &imagePos, &pointSet, &point);
		
		if(redoImage)
		{
			Rect2i rect(0, 0, redoImage->GetWidth(), redoImage->GetHeight());
			Image* image = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
			CopyImageRectToImage(redoImage, rect, image, imagePos);

			Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight());
			Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());

			RecreateVisibilityAreaSprite();
			RenderManager::Instance()->SetRenderTarget(visibilityAreaSprite);
			sprite->Draw();
			RenderManager::Instance()->RestoreRenderTarget();
			
			SafeRelease(sprite);
			SafeRelease(texture);
			SafeRelease(image);
		}

		isVisibilityPointSet = pointSet;
		visibilityPoint = point;

		PerformLandscapeDraw();
	}
	else if(UNDOAction::ACTION_VISIBILITY_POINT == type)
	{
		Image* redoImage;
		Point2i imagePos;
		bool pointSet;
		Vector2 point;

		UNDOManager::Instance()->RedoVisibility(&redoImage, &imagePos, &pointSet, &point);

		isVisibilityPointSet = pointSet;
		visibilityPoint = point;
		RecreateVisibilityAreaSprite();
		PerformLandscapeDraw();
	}
}

void LandscapeEditorVisibilityCheckTool::SaveTextureAction(const String &pathToFile)
{
    if(visibilityAreaSprite)
    {
		Sprite* saveSprite = Sprite::CreateAsRenderTarget(visibilityAreaSprite->GetWidth(),
														  visibilityAreaSprite->GetHeight(),
														  FORMAT_RGBA8888);

		if(!saveSprite)
			return;

		Sprite* cursorSprite = Sprite::CreateFromTexture(pointCursorTexture,
														 0,
														 0,
														 pointCursorTexture->GetWidth(),
														 pointCursorTexture->GetHeight());

		RenderManager::Instance()->SetRenderTarget(saveSprite);
		visibilityAreaSprite->Draw();
		if(cursorSprite != 0)
		{
			float32 scale = VISIBILITY_POINT_CURSOR_SCALE * 2;
			Vector2 scaledSize = cursorSprite->GetSize() * scale;
			cursorSprite->SetPosition(visibilityPoint - scaledSize / 2);
			cursorSprite->SetScale(scale, scale);
			cursorSprite->Draw();
		}
		RenderManager::Instance()->RestoreRenderTarget();

        Image *img = saveSprite->GetTexture()->CreateImageFromMemory();
        if(img)
        {
            img->Save(pathToFile);
            SafeRelease(img);
        }
		
		SafeRelease(cursorSprite);
		SafeRelease(saveSprite);
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

Rect2i LandscapeEditorVisibilityCheckTool::FitRectToImage(DAVA::Image *image, const Rect2i& rect)
{
	Point2i rectPos(Max(rect.x, 0),
					Max(rect.y, 0));
	Point2i rectBottomRight(Min(rect.x + rect.dx, image->GetWidth()),
							Min(rect.y + rect.dy, image->GetHeight()));
	Point2i rectSize = rectBottomRight - rectPos;

	Rect2i newRect;
	newRect.SetSize(Size2i(rectSize.x, rectSize.y));
	newRect.SetPosition(rectPos);
	
	return newRect;
}

void LandscapeEditorVisibilityCheckTool::CopyImageRectToImage(Image* imageFrom, const Rect2i& rectFrom, Image* imageTo, const Point2i& pos)
{
	DVASSERT(imageFrom && imageTo && imageTo->GetPixelFormat() == imageFrom->GetPixelFormat());

	uint32 fromWidth = rectFrom.dx;
	uint32 fromHeight = rectFrom.dy;
	uint32 fromX = rectFrom.x;
	uint32 fromY = rectFrom.y;
	uint32 toX = pos.x;
	uint32 toY = pos.y;
	uint32 pixelSize = Texture::GetPixelFormatSizeInBytes(imageFrom->GetPixelFormat());

	uint8* imageFromData = imageFrom->GetData();
	uint8* imageToData = imageTo->GetData();

	for(uint32 i = 0; i < fromHeight; ++i)
	{
		uint8* imageFromRow = &(imageFromData[imageFrom->GetWidth() * pixelSize * (fromY + i) + fromX * pixelSize]);
		uint8* imageToRow = &(imageToData[imageTo->GetWidth() * pixelSize * (toY + i) + toX * pixelSize]);

		memcpy(imageToRow, imageFromRow, fromWidth * pixelSize);
	}
}
