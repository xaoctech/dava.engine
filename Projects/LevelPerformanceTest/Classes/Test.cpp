#include "Test.h"
#include "SettingsManager.h"

#define DEF_CAMERA_DIRECTION Vector3(1.f, 0.f, 0.f)

int Test::globalScreenId=2;

Test::Test() {
}

Test::Test(const String& fullName, int skipFrames) {
	m_FullName = fullName;
	m_nSkipFrames = skipFrames;
	
	m_nScreenId = globalScreenId++;
}

void Test::LoadResources()
{
	SettingsManager *pSettings = SettingsManager::Instance();
	
	m_fTime = 0.f;
	m_fVelocity = pSettings->GetCameraMovementSpeed();
	m_bIsFinished = false;

	Scene *scene = new Scene();
	scene->AddNode(scene->GetRootNode(m_FullName));
	
	Camera* pCam = new Camera();
	scene->AddCamera(pCam);

	Core* core = DAVA::Core::Instance();
	float aspect = core->GetVirtualScreenHeight() / core->GetVirtualScreenWidth();

	pCam->Setup(70.f, aspect, 1.f, 5000.f);
	pCam->SetLeft(Vector3(1, 0, 0));
	pCam->SetUp(Vector3(0, 0, 1));
    
    scene->SetCurrentCamera(pCam);
	SafeRelease(pCam);
	
    LandscapeNode* pLand = (LandscapeNode *)scene->FindByName(pSettings->GetLandscapeNodeName());
    if(pLand)
    {
		pLand->SetTiledShaderMode(LandscapeNode::TILED_MODE_TEXTURE);
		UI3DView *sceneView = new UI3DView(Rect(0, 0, GetSize().x, GetSize().y));
		sceneView->SetScene(scene);
		AddControl(sceneView);
		SafeRelease(sceneView);
    }

	PreparePath();

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
	m_fTime += timeElapsed;
	
	SettingsManager *pSettings = SettingsManager::Instance();
	
	if(m_nSkipFrames)
		--m_nSkipFrames;

	if(!m_bIsFinished && !m_nSkipFrames) {
		float fps = 1.f / timeElapsed;
		if(fps > m_fFpsMax)
			m_fFpsMax = fps;
		if(fps < m_fFpsMin)
			m_fFpsMin = fps;
		m_fFpsMid += fps;
		++m_nFrameCount;
		
		Camera* pCam = GetCamera();
		
		Vector3 camOffset(m_VelocityDir * m_fVelocity);
		Vector3 camPos = pCam->GetPosition();
		camPos += camOffset;

		Vector3 diffFpsCheck = m_NextFpsCheckPoint - camPos;
		if(fabsf(diffFpsCheck.x) < (m_fVelocity / 2) && fabsf(diffFpsCheck.y) < (m_fVelocity / 2)) {
			++m_nNextFpsCheckPointNum;
			UpdateFpsSegment();
		}
		
		Vector3 diffPath = m_NextPathPoint - camPos;
		if(fabsf(diffPath.x) < (m_fVelocity / 2) && fabsf(diffPath.y) < (m_fVelocity / 2)) {
			camPos = m_NextPathPoint;
			
			if(m_nCurPathPointNum < (m_CenterPoints.size() - 2)) {
				++m_nCurPathPointNum;
				UpdatePathSegment();
			} else {
				m_bIsFinished = true;
			}
		}
		
		static float angle = 0.f;
		angle += pSettings->GetCameraRotationAngleStep();
		if(angle >= 360.f)
			angle -= 360.f;
		
		Vector3 newCamTarget = DEF_CAMERA_DIRECTION * Matrix4::MakeRotation(Vector3(0.f, 0.f, 1.f), DegToRad(angle));
		newCamTarget += camPos;
		
		pCam->SetTarget(newCamTarget);
		pCam->SetPosition(camPos);
	}
	
	UIScreen::Update(timeElapsed);
}

void Test::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void Test::PreparePath() {
	SettingsManager *pSettings = SettingsManager::Instance();
	
	int X = pSettings->GetLandscapePartitioning().x;
	int Y = pSettings->GetLandscapePartitioning().y;
	float H = pSettings->GetCameraElevation();
	
	LandscapeNode *pLand = GetLandscape();
	
	AABBox3 boundingBox = pLand->GetBoundingBox();
	Vector3 min = boundingBox.min;
	Vector3 max = boundingBox.max;
	
	float landWidth = max.x - min.x;
	float landLength = max.y - min.y;
	
	Vector2 rectSize(landWidth / X, landLength / Y);
	
	int x = 0;
	int xDir = 1;
	for(int y = 0; y < Y; ++y) {
		Rect curRect;
		for(int i = X; i; --i, x += xDir) {
			Vector2 v;
			v.Set(min.x + x * rectSize.x, min.y + y * rectSize.y);
			curRect.SetPosition(v);
			curRect.SetSize(rectSize);
			
			Vector3 p(curRect.GetCenter());
			pLand->PlacePoint(p, p);
			p.z += H;
			
			m_CenterPoints.push_back(p);
		}
		x -= xDir;
		xDir = -xDir;
	}
	
	for(Vector<Vector3>::const_iterator it = m_CenterPoints.begin(); it != m_CenterPoints.end(); ++it) {
		Logger::Debug("%.4f %.4f %.4f", (*it).x, (*it).y, (*it).z);
	}
	
	Vector3 camPos = m_CenterPoints.front();
	
	Camera* pCam = GetCamera();
	
	pCam->SetPosition(camPos);
	pCam->SetDirection(DEF_CAMERA_DIRECTION);
	
	PrepareFpsCheckPoints();
	
	m_nCurPathPointNum = 0;
	UpdatePathSegment();
}

void Test::PrepareFpsCheckPoints() {
	Vector<Vector3>::const_iterator it = m_CenterPoints.begin();
	
	Vector3 prevPoint, curPoint;
	curPoint = *it;
	while(++it != m_CenterPoints.end()) {
		prevPoint = curPoint;
		curPoint = *it;
		
		Vector3 fpsCheckPoint = (prevPoint + curPoint) / 2.f;
		m_FpsCheckPoints.push_back(fpsCheckPoint);
	}
	m_FpsCheckPoints.push_back(m_CenterPoints.back());
	
	for(Vector<Vector3>::const_iterator it = m_FpsCheckPoints.begin(); it != m_FpsCheckPoints.end(); ++it) {
		Logger::Debug("%.4f %.4f", (*it).x, (*it).y);
	}
	
	ZeroCurFpsStat();
	
	m_nNextFpsCheckPointNum = 0;
	m_NextFpsCheckPoint = m_FpsCheckPoints.front();
}

void Test::UpdatePathSegment() {
	Logger::Debug("%d -> %d", m_nCurPathPointNum, m_nCurPathPointNum + 1);
	
	m_CurPathPoint = m_CenterPoints[m_nCurPathPointNum];
	m_NextPathPoint = m_CenterPoints[m_nCurPathPointNum + 1];

	m_VelocityDir = m_NextPathPoint - m_CurPathPoint;
	m_VelocityDir.Normalize();
}

void Test::UpdateFpsSegment() {
	m_fFpsMid /= m_nFrameCount;
	m_FpsStat[0].push_back(m_fFpsMin);
	m_FpsStat[1].push_back(m_fFpsMid);
	m_FpsStat[2].push_back(m_fFpsMax);
	Logger::Debug("min: %.4f\tmid: %.4f\tmax: %.4f", m_fFpsMin, m_fFpsMid, m_fFpsMax);
	
	ZeroCurFpsStat();

	m_NextFpsCheckPoint = m_FpsCheckPoints[m_nNextFpsCheckPointNum];
}

const String Test::GetFileName() const {
	String path;
	String filename;
	FileSystem::Instance()->SplitPath(m_FullName, path, filename);
	return filename;
}

void Test::ZeroCurFpsStat() {
	m_fFpsMin = INFINITY;
	m_fFpsMid = 0.f;
	m_fFpsMax = 0.f;
	m_nFrameCount = 0;
}

inline UI3DView* Test::GetSceneView() {
	return (UI3DView*)GetChildren().front();
}

inline Scene* Test::GetScene() {
	return GetSceneView()->GetScene();
}

inline Camera* Test::GetCamera() {
	return GetScene()->GetCurrentCamera();
}

inline LandscapeNode* Test::GetLandscape() {
	SettingsManager* pSettings = SettingsManager::Instance();
	return (LandscapeNode *)GetScene()->FindByName(pSettings->GetLandscapeNodeName());
}
