/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "TestScreen.h"
#include "PngImageEx.h"

#define H 10.f
#define X 3
#define Y 4
#define ANGLE_STEP 1.f
#define VELOCITY .5f
#define DEF_RECT_STEP_X 50
#define DEF_RECT_STEP_Y 20
#define DEF_CAMERA_DIRECTION Vector3(1.f, 0.f, 0.f)

void TestScreen::LoadResources()
{
	time=0.f;
	velocity=VELOCITY;
	isFinished=false;

	
	land = new LandscapeNode();  
	SafeRelease(land);
	
	Scene *scene = new Scene();
	scene->AddNode(scene->GetRootNode(String("~res:/3d/LandscapeTest/landscapetest.sc2")));
	
	cam = (Camera *)scene->FindByName(String("TestCamera"));
    
	scene->AddCamera(cam); 
    scene->SetCurrentCamera(cam);

    land = (LandscapeNode *)scene->FindByName(String("Landscape")); 
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

void TestScreen::UnloadResources()
{
	RemoveAllControls();
}

void TestScreen::WillAppear()
{
}

void TestScreen::WillDisappear()
{
}

void TestScreen::Input(UIEvent * event)
{
}

void TestScreen::Update(float32 timeElapsed)
{
	time+=timeElapsed;

	if(!isFinished) {
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
				
				CreateAndSaveImage(&fpsStat[0], "min");
				CreateAndSaveImage(&fpsStat[1], "mid");
				CreateAndSaveImage(&fpsStat[2], "max");
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

void TestScreen::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void TestScreen::PreparePath() {
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

void TestScreen::PrepareFpsCheckPoints() {
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

void TestScreen::UpdatePathSegment() {
	Logger::Debug("%d -> %d", curPathPointNum, curPathPointNum+1);

	curPathPoint=centerPoints[curPathPointNum];
	nextPathPoint=centerPoints[curPathPointNum+1];

	velocityDir=nextPathPoint-curPathPoint;
	velocityDir.Normalize();
}

void TestScreen::UpdateFpsSegment() {
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

void TestScreen::CreateAndSaveImage(Vector<float> *v, const char *filename) {
	String file=FileSystem::Instance()->GetUserDocumentsPath();
	file+=filename;
	file+=".png";

	PngImageEx *img=new PngImageEx();
	
	int width=X*DEF_RECT_STEP_X+1;
	int height=Y*DEF_RECT_STEP_Y+1;
	img->Create(width, height);
	img->DrawFillRect(Rect2i(0, 0, width, height), 0xff000000);
	
	Vector2 rectSize(DEF_RECT_STEP_X-1, DEF_RECT_STEP_Y-1);

	int x=0;
	int xDir=1;
	int n=0;
	for(int y=0; y<Y; ++y) {
		Rect curRect;
		for(int i=X; i; --i, x+=xDir) {
			Vector2 pos;
			pos.Set(x*DEF_RECT_STEP_X+1, y*DEF_RECT_STEP_Y+1);
			img->DrawFillRect(Rect2i(pos.x, pos.y, rectSize.x, rectSize.y), PickColor((*v)[n]));
			++n;
		}
		x-=xDir;
		xDir=-xDir;
	}

	img->Save(file.c_str());
	delete img;
}

#define RGB2ABGR(r, g, b) ((uint32)(0xff000000 | ((uint8)(b) << 16) | ((uint8)(g) << 8) | (uint8)(r)))

uint32 TestScreen::PickColor(float fps) {
	const float fpsList[] = {
		0.f, 15.f, 20.f, 35.f, 45.f, INFINITY
	};
	const uint32 colors[sizeof(fpsList)-1] = { //ABGR
		RGB2ABGR(0xff, 0x00, 0x00),
		RGB2ABGR(0xff, 0xff, 0x3e),
		RGB2ABGR(0x92, 0xd0, 0x50),
		RGB2ABGR(0x00, 0xb0, 0x50),
		RGB2ABGR(0x00, 0x70, 0xc0)
	};
	
	for(int i=0; i<sizeof(fpsList)-1; ++i) {
		if(fps >= fpsList[i] && fps < fpsList[i+1]) {
			return colors[i];
		}
	}
	return 0x00000000;
}
