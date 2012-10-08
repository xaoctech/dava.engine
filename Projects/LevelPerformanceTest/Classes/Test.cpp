#include "Test.h"
#include "Constants.h"

#define H 10.f
#define ANGLE_STEP 1.f
#define VELOCITY 4.5f
#define DEF_CAMERA_DIRECTION Vector3(1.f, 0.f, 0.f)
#define LANDSCAPE_NODE_NAME "Landscape"

int Test::globalScreenId=2;

Test::Test() {
}

Test::Test(const String& fullName, int skipFrames) {
	this->fullName=fullName;
	this->skipFrames=skipFrames;
	
	screenId=globalScreenId++;
}

void Test::LoadResources()
{
	time=0.f;
	velocity=VELOCITY;
	isFinished=false;
	
	
	land = new LandscapeNode();  
	SafeRelease(land);
	
	Scene *scene = new Scene();
	scene->AddNode(scene->GetRootNode(fullName));
	
	cam = new Camera();
	scene->AddCamera(cam);

	Core* core=DAVA::Core::Instance();
	float aspect=core->GetVirtualScreenHeight() / core->GetVirtualScreenWidth();

	cam->Setup(70.f, aspect, 1.f, 5000.f);
	cam->SetLeft(Vector3(1, 0, 0));
	cam->SetUp(Vector3(0, 0, 1));
	//SafeRelease(cam);	
    
    scene->SetCurrentCamera(cam);
	
    land = (LandscapeNode *)scene->FindByName(LANDSCAPE_NODE_NAME); 
    if(land)
    {
		land->SetTiledShaderMode(LandscapeNode::TILED_MODE_TEXTURE);
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
	time+=timeElapsed;
	
	if(skipFrames)
		--skipFrames;
	
	if(!isFinished && !skipFrames) {
		float fps=1.f/timeElapsed;
		if(fps > fpsMax)
			fpsMax=fps;
		if(fps < fpsMin)
			fpsMin=fps;
		fpsMid+=fps;
		++frameCount;
		
		Vector3 camOffset(velocityDir*velocity);
		Vector3 camPos=cam->GetPosition();
		camPos+=camOffset;
		
		Vector3 diffFpsCheck=nextFpsCheckPoint-camPos;
		if(fabsf(diffFpsCheck.x) < (velocity/2) && fabsf(diffFpsCheck.y) < (velocity/2)) {
			++nextFpsCheckPointNum;
			UpdateFpsSegment();
		}
		
		Vector3 diffPath=nextPathPoint-camPos;
		if(fabsf(diffPath.x) < (velocity/2) && fabsf(diffPath.y) < (velocity/2)) {
			camPos=nextPathPoint;
			
			if(curPathPointNum < (centerPoints.size()-2)) {
				++curPathPointNum;
				UpdatePathSegment();
			} else {
				isFinished=true;
			}
		}
		
		static float angle=0.f;
		angle+=ANGLE_STEP;
		if(angle >= 360.f)
			angle-=360.f;
		
		Vector3 newCamTarget=DEF_CAMERA_DIRECTION*Matrix4::MakeRotation(Vector3(0.f, 0.f, 1.f), DegToRad(angle));
		newCamTarget+=camPos;
		
		cam->SetTarget(newCamTarget);
		cam->SetPosition(camPos);
	}
	
	UIScreen::Update(timeElapsed);
}

void Test::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void Test::PreparePath() {
	AABBox3 boundingBox=land->GetBoundingBox();
	Vector3 min=boundingBox.min;
	Vector3 max=boundingBox.max;
	
	float landWidth=max.x-min.x;
	float landLength=max.y-min.y;
	
	Vector2 rectSize(landWidth/X, landLength/Y);
	
	int x=0;
	int xDir=1;
	for(int y=0; y<Y; ++y) {
		Rect curRect;
		for(int i=X; i; --i, x+=xDir) {
			Vector2 v;
			v.Set(min.x+x*rectSize.x, min.y+y*rectSize.y);
			curRect.SetPosition(v);
			curRect.SetSize(rectSize);
			
			Vector3 p(curRect.GetCenter());
			land->PlacePoint(p, p);
			p.z+=H;
			
			centerPoints.push_back(p);
		}
		x-=xDir;
		xDir=-xDir;
	}
	
	for(Vector<Vector3>::const_iterator it=centerPoints.begin(); it!=centerPoints.end(); ++it) {
		Logger::Debug("%.4f %.4f %.4f", (*it).x, (*it).y, (*it).z);
	}
	
	Vector3 camPos=centerPoints.front();
	
	cam->SetPosition(camPos);
	cam->SetDirection(DEF_CAMERA_DIRECTION);
	
	PrepareFpsCheckPoints();
	
	curPathPointNum=0;
	UpdatePathSegment();
}

void Test::PrepareFpsCheckPoints() {
	Vector<Vector3>::const_iterator it=centerPoints.begin();
	
	Vector3 prevPoint, curPoint;
	curPoint=*it;
	while(++it != centerPoints.end()) {
		prevPoint=curPoint;
		curPoint=*it;
		
		Vector3 fpsCheckPoint=(prevPoint+curPoint)/2.f;
		fpsCheckPoints.push_back(fpsCheckPoint);
	}
	fpsCheckPoints.push_back(centerPoints.back());
	
	for(Vector<Vector3>::const_iterator it=fpsCheckPoints.begin(); it!=fpsCheckPoints.end(); ++it) {
		Logger::Debug("%.4f %.4f", (*it).x, (*it).y);
	}
	
	fpsMin=INFINITY;
	fpsMid=0.f;
	fpsMax=0.f;
	frameCount=0.f;
	
	nextFpsCheckPointNum=0;
	nextFpsCheckPoint=fpsCheckPoints.front();
}

void Test::UpdatePathSegment() {
	Logger::Debug("%d -> %d", curPathPointNum, curPathPointNum+1);
	
	curPathPoint=centerPoints[curPathPointNum];
	nextPathPoint=centerPoints[curPathPointNum+1];
	
	velocityDir=nextPathPoint-curPathPoint;
	velocityDir.Normalize();
}

void Test::UpdateFpsSegment() {
	fpsMid/=frameCount;
	fpsStat[0].push_back(fpsMin);
	fpsStat[1].push_back(fpsMid);
	fpsStat[2].push_back(fpsMax);
	Logger::Debug("min: %.4f\tmid: %.4f\tmax: %.4f", fpsMin, fpsMid, fpsMax);
	
	fpsMin=INFINITY;
	fpsMid=0.f;
	fpsMax=0.f;
	frameCount=0;
	
	nextFpsCheckPoint=fpsCheckPoints[nextFpsCheckPointNum];
}

const String Test::GetFileName() const {
	String path;
	String filename;
	FileSystem::Instance()->SplitPath(fullName, path, filename);
	return filename;
}
