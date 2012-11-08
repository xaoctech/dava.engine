#include "Test.h"
#include "SettingsManager.h"
#include "AppScreens.h"

#define DEF_CAMERA_DIRECTION Vector3(1.f, 0.f, 0.f)

enum eCameraAnimationFrames
{
    CAMERA_ANIMATION_MOVE = 0,
    CAMERA_ANIMATION_ROTATE,
    
    CAMERA_ANIMATION_COUNT
};

int32 Test::globalScreenId=PREDEFINED_SCREENS_COUNT;

Test::Test()
{
}

Test::Test(const String& fullName)
{
	this->fullName = fullName;
	
	screenId = globalScreenId++;
}

void Test::LoadResources()
{
	SettingsManager *pSettings = SettingsManager::Instance();
	
	time = 0.f;
	isFinished = false;

	Scene *scene = new Scene();
	scene->AddNode(scene->GetRootNode(fullName));
	
	Camera* cam = new Camera();
	scene->AddCamera(cam);

	Core* core = DAVA::Core::Instance();
	float32 aspect = core->GetVirtualScreenHeight() / core->GetVirtualScreenWidth();

	cam->Setup(70.f, aspect, 1.f, 5000.f);
	cam->SetLeft(Vector3(1, 0, 0));
	cam->SetUp(Vector3(0, 0, 1));
    
    scene->SetCurrentCamera(cam);
	SafeRelease(cam);
	
    LandscapeNode* land = (LandscapeNode *)scene->FindByName(pSettings->GetLandscapeNodeName());
    if(land)
    {
		land->SetTiledShaderMode(LandscapeNode::TILED_MODE_TEXTURE);
		UI3DView *sceneView = new UI3DView(Rect(0, 0, GetSize().x, GetSize().y));
		sceneView->SetScene(scene);
		AddControl(sceneView);
		SafeRelease(sceneView);
    }

	PreparePath();
    PrepareFpsStat();
    PrepareCameraAnimation();
    ZeroCurFpsStat();
    MoveToNextPoint();

    SafeRelease(scene);
}

void Test::UnloadResources()
{
	RemoveAllControls();
}

void Test::WillAppear()
{
}

void Test::WillDisappear()
{
}

void Test::Input(UIEvent * event)
{
}

void Test::Update(float32 timeElapsed)
{
	time += timeElapsed;

    if(isFinished)
    {
        SaveFpsStat();
    }
    else
    {
        Rect curFpsRect = rectSequence[curFpsRectNum];
        Vector2 point(curCameraPosition.x, curCameraPosition.y);
        if(!curFpsRect.PointInside(point))
        {
            SaveFpsStat();
            
            ZeroCurFpsStat();
            ++curFpsRectNum;
        }

        float32 fps = 1.f / timeElapsed;
		if(fps > fpsMax)
			fpsMax = fps;
		if(fps < fpsMin)
			fpsMin = fps;
		fpsMid += fps;
		++frameCount;

		Camera* cam = GetCamera();
        cam->SetPosition(curCameraPosition);

		Vector3 newCamTarget = DEF_CAMERA_DIRECTION * Matrix4::MakeRotation(Vector3(0.f, 0.f, 1.f), DegToRad(curCameraAngle));
		newCamTarget += curCameraPosition;
		
		cam->SetTarget(newCamTarget);
	}

	UIScreen::Update(timeElapsed);
}

void Test::SaveFpsStat()
{
    fpsStat[STAT_MIN][curFpsRectNum] = fpsMin;
    fpsStat[STAT_MID][curFpsRectNum] = fpsMid / frameCount;
    fpsStat[STAT_MAX][curFpsRectNum] = fpsMax;
}

void Test::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void Test::PreparePath()
{
	SettingsManager *settings = SettingsManager::Instance();

	int32 partX = settings->GetLandscapePartitioning().x;
	int32 partY = settings->GetLandscapePartitioning().y;
	
    LandscapeNode *land = GetLandscape();
	AABBox3 boundingBox = land->GetBoundingBox();
	Vector3 min = boundingBox.min;
	Vector3 max = boundingBox.max;
	
	float32 landWidth = max.x - min.x;
	float32 landLength = max.y - min.y;
	
	Vector2 rectSize(landWidth / partX, landLength / partY);
	
	int32 x = 0;
	int32 xDir = 1;
	for(int32 y = 0; y < partY; ++y)
    {
		Rect curRect;
		for(int32 i = 0; i < partX; ++i, x += xDir)
        {
			Vector2 v;
			v.Set(min.x + x * rectSize.x, min.y + y * rectSize.y);
			curRect.SetPosition(v);
			curRect.SetSize(rectSize);
            rectSequence.push_back(curRect);
		}
		x -= xDir;
		xDir = -xDir;
	}
}

void Test::MoveToNextPoint()
{
    ++nextRectNum;

    if(nextRectNum < rectSequence.size())
    {
        Rect nextRect = rectSequence[nextRectNum];

        Vector3 endPoint = GetRealPoint(nextRect.GetCenter());

        float32 pathSegmentLength = (endPoint - curCameraPosition).Length();
        float32 timeToMove = pathSegmentLength / SettingsManager::Instance()->GetCameraMovementSpeed();
        
        camMoveAnimation = new LinearAnimation<Vector3>(this, &curCameraPosition, endPoint, timeToMove, Interpolation::LINEAR);
        camMoveAnimation->AddEvent(Animation::EVENT_ANIMATION_END, Message(this, &Test::AnimationFinished));
        camMoveAnimation->Start(CAMERA_ANIMATION_MOVE);
    }
    else
    {
        isFinished = true;
    }
}

void Test::PrepareCameraAnimation()
{
    Rect rect = rectSequence.front();
    curCameraPosition = GetRealPoint(rect.GetCenter());

    Camera* cam = GetCamera();
    cam->SetPosition(curCameraPosition);
    cam->SetDirection(DEF_CAMERA_DIRECTION);

    nextRectNum = 0;

    curCameraAngle = 0.f;
    float32 maxRotateAngle = 360.f;
    float32 timeToRotate = maxRotateAngle / SettingsManager::Instance()->GetCameraRotationSpeed();
    camRotateAnimation = new LinearAnimation<float32>(this, &curCameraAngle, maxRotateAngle, timeToRotate, Interpolation::LINEAR);
    camRotateAnimation->SetRepeatCount(-1);
    camRotateAnimation->Start(CAMERA_ANIMATION_ROTATE);
}

void Test::PrepareFpsStat()
{
    int32 rectCount = rectSequence.size();
    fpsStat[STAT_MIN].resize(rectCount);
    fpsStat[STAT_MID].resize(rectCount);
    fpsStat[STAT_MAX].resize(rectCount);
    
    curFpsRectNum = 0;
}

Vector3 Test::GetRealPoint(const Vector2& point)
{
    Vector3 realPoint(point);

	LandscapeNode *land = GetLandscape();
    land->PlacePoint(realPoint, realPoint);

    realPoint.z += SettingsManager::Instance()->GetCameraElevation();
    
    return realPoint;
}

void Test::AnimationFinished(DAVA::BaseObject *, void *, void *)
{
    MoveToNextPoint();
    camMoveAnimation = NULL;
}

const String Test::GetFileName() const
{
	String path;
	String filename;
	FileSystem::Instance()->SplitPath(fullName, path, filename);
	return filename;
}

void Test::ZeroCurFpsStat()
{
	fpsMin = std::numeric_limits<float32>::infinity();
	fpsMid = 0.f;
	fpsMax = 0.f;
	frameCount = 0;
}

inline UI3DView* Test::GetSceneView()
{
	return (UI3DView*)GetChildren().front();
}

inline Scene* Test::GetScene()
{
	return GetSceneView()->GetScene();
}

inline Camera* Test::GetCamera()
{
	return GetScene()->GetCurrentCamera();
}

inline LandscapeNode* Test::GetLandscape()
{
	SettingsManager* settings = SettingsManager::Instance();
	return (LandscapeNode *)GetScene()->FindByName(settings->GetLandscapeNodeName());
}
