#include "Test.h"
#include "SettingsManager.h"
#include "AppScreens.h"
#include "TextureHelper.h"

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
	time = 0.f;
	isFinished = false;
	skipFrames = 100;

	Scene *scene = new Scene();
	scene->AddNode(scene->GetRootNode(fullName));
	DVASSERT_MSG(scene, "Could not load the scene");

	Camera* cam = new Camera();
	scene->AddCamera(cam);

	Core* core = DAVA::Core::Instance();
	float32 aspect = core->GetVirtualScreenHeight() / core->GetVirtualScreenWidth();

	cam->Setup(70.f, aspect, 1.f, 5000.f);
	cam->SetLeft(Vector3(1, 0, 0));
	cam->SetUp(Vector3(0, 0, 1));
    
    scene->SetCurrentCamera(cam);
	SafeRelease(cam);

	UI3DView *sceneView = new UI3DView(Rect(0, 0, GetSize().x, GetSize().y));
	sceneView->SetScene(scene);
	AddControl(sceneView);
	SafeRelease(sceneView);

	Landscape* landscape = GetLandscape();
	DVASSERT_MSG(scene, "There is no landscape in a scene");
//	landscape->SetTiledShaderMode(Landscape::TILED_MODE_TEXTURE);

	uint32 textureMemory = TextureHelper::GetSceneTextureMemory(scene, GetFilePath());
	testData.SetTextureMemorySize(textureMemory);

	uint32 textureFilesSize = TextureHelper::GetSceneTextureFilesSize(scene, GetFilePath());
	testData.SetTexturesFilesSize(textureFilesSize);

	File* file = File::Create(fullName, File::OPEN | File::READ);
	DVASSERT_MSG(file, "Could not open file scene file");
	testData.SetSceneFileSize(file->GetSize());
	SafeRelease(file);

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

	if(skipFrames)
	{
		if(!--skipFrames)
		{
			camRotateAnimation->Start(CAMERA_ANIMATION_ROTATE);
			camMoveAnimation->Start(CAMERA_ANIMATION_MOVE);
		}
	}
	else if(!isFinished)
    {
		Camera* cam = GetCamera();

		float32 fps = 1.f / timeElapsed;
		if(fps < fpsStatItem.minFps)
		{
			fpsStatItem.minFps = fps;
			fpsStatItem.position = cam->GetPosition();
			fpsStatItem.viewTarget = cam->GetTarget();
		}

        Rect curFpsRect = rectSequence[testData.GetItemCount()];
        Vector2 point(curCameraPosition.x, curCameraPosition.y);
        if(!curFpsRect.PointInside(point))
        {
            SaveFpsStat();
            
            ZeroCurFpsStat();
			fpsStatItem.rect = rectSequence[testData.GetItemCount()];
        }
		
        cam->SetPosition(curCameraPosition);

		Vector3 newCamTarget = DEF_CAMERA_DIRECTION * Matrix4::MakeRotation(Vector3(0.f, 0.f, 1.f), DegToRad(curCameraAngle));
		newCamTarget += curCameraPosition;
		
		cam->SetTarget(newCamTarget);
	}

	UIScreen::Update(timeElapsed);
}

void Test::SaveFpsStat()
{
	testData.AddStatItem(fpsStatItem);
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
	
    Landscape *land = GetLandscape();
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
		if(!skipFrames)
        	camMoveAnimation->Start(CAMERA_ANIMATION_MOVE);
    }
    else
    {
		SaveFpsStat();
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
}

void Test::PrepareFpsStat()
{
	testData.Clear();
	fpsStatItem.rect = rectSequence[testData.GetItemCount()];

    Landscape *land = GetLandscape();
	AABBox3 boundingBox = land->GetBoundingBox();
	
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,
					 (boundingBox.max - boundingBox.min).x);

	testData.SetLandscapeRect(Rect(landPos, landSize));
}

Vector3 Test::GetRealPoint(const Vector2& point)
{
    Vector3 realPoint(point);

	Landscape *land = GetLandscape();
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

const String Test::GetFilePath() const
{
	String path;
	String filename;
	FileSystem::Instance()->SplitPath(fullName, path, filename);
	return path;
}

void Test::ZeroCurFpsStat()
{
	fpsStatItem.minFps = std::numeric_limits<float32>::infinity();
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

inline Landscape* Test::GetLandscape()
{
	SettingsManager* settings = SettingsManager::Instance();
	Entity* landscapeNode = GetScene()->FindByName(settings->GetLandscapeNodeName());
	Landscape* landscape = NULL;
	if (landscapeNode)
	{
		RenderComponent* renderComponent = cast_if_equal<RenderComponent*>(landscapeNode->GetComponent(Component::RENDER_COMPONENT));
		if (renderComponent)
		{
			landscape = dynamic_cast<Landscape*>(renderComponent->GetRenderObject());
		}
	}

	return landscape;
}

Texture* Test::GetLandscapeTexture()
{
	Landscape* landscape = GetLandscape();
	return landscape->CreateFullTiledTexture();
};

