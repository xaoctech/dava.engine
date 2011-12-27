#include "CameraOutputControl.h"

#include "../EditorScene.h"
#include "ControlsFactory.h"

CameraOutputControl::CameraOutputControl(EditorScene *_scene, const Rect & rect)
    :   OutputControl(rect)
{
    scene = _scene;
    timeAfterUpdate = 1.f;

    Log(L"Current camera:");
    Log(L"Fov");
    Log(L"Position");
    Log(L"Target");
    Log(L"Left");
    Log(L"Up");
    Log(L"Clip camera:");
    Log(L"Fov");
    Log(L"Position");
    Log(L"Target");
    Log(L"Left");
    Log(L"Up");
}


CameraOutputControl::~CameraOutputControl()
{

}

void CameraOutputControl::WillAppear()
{
    ReadCamera(false);

    OutputControl::WillAppear();
    timeAfterUpdate = 0.f;
}


void CameraOutputControl::Update(float32 timeElapsed)
{
    if(GetParent())
    {
        timeAfterUpdate += timeElapsed;
        if(1.f < timeAfterUpdate)
        {
            timeAfterUpdate = 0.f;
            ReadCamera(true);
        }
    }
    
    OutputControl::Update(timeElapsed);
}

void CameraOutputControl::ReadCamera(bool refreshList)
{
    if(scene)
    {
        Camera *camera = scene->GetCurrentCamera();
        if(camera)
        {
            messages[1]->text = Format(L"FOV: %0.2f. Aspect: %0.2f. zNear: %0.2f. zFar: %0.2f.", 
                                       camera->GetFOV(), camera->GetAspect(), camera->GetZNear(), camera->GetZFar());
            
            Vector3 pos = camera->GetPosition();
            messages[2]->text = Format(L"Position: %0.2f, %0.2f, %0.2f", pos.x, pos.y, pos.z);
            Vector3 target = camera->GetTarget();
            messages[3]->text = Format(L"Target: %0.2f, %0.2f, %0.2f", target.x, target.y, target.z);
            Vector3 left = camera->GetLeft();
            messages[4]->text = Format(L"Left: %0.2f, %0.2f, %0.2f", left.x, left.y, left.z);
            Vector3 up = camera->GetUp();
            messages[5]->text = Format(L"Up: %0.2f, %0.2f, %0.2f", up.x, up.y, up.z);
        }
        
        camera = scene->GetClipCamera();
        if(camera)
        {
            messages[7]->text = Format(L"FOV: %0.2f. Aspect: %0.2f. zNear: %0.2f. zFar: %0.2f.", 
                                       camera->GetFOV(), camera->GetAspect(), camera->GetZNear(), camera->GetZFar());
            
            Vector3 pos = camera->GetPosition();
            messages[8]->text = Format(L"Position: %0.2f, %0.2f, %0.2f", pos.x, pos.y, pos.z);
            Vector3 target = camera->GetTarget();
            messages[9]->text = Format(L"Target: %0.2f, %0.2f, %0.2f", target.x, target.y, target.z);
            Vector3 left = camera->GetLeft();
            messages[10]->text = Format(L"Left: %0.2f, %0.2f, %0.2f", left.x, left.y, left.z);
            Vector3 up = camera->GetUp();
            messages[11]->text = Format(L"Up: %0.2f, %0.2f, %0.2f", up.x, up.y, up.z);
        }
        if(refreshList)
        {
            messageList->Refresh();
        }
    }
}