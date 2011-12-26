#include "ScenePreviewControl.h"

#include "ControlsFactory.h"

// ***************** PreviewCameraController *************** //
PreviewCameraController::PreviewCameraController()
    :   CameraController()
{
    angleVertical = 0.f;
    angleHorizontal = 0.f;
    
    moveStartPt = Vector2(0, 0);
    moveStopPt = Vector2(0, 0);
    
    zoomStartPt = Vector2(0, 0);
    zoomStopPt = Vector2(0, 0);

    radius = 10.f;
    zoomLevel = 1.f;
}

void PreviewCameraController::SetCamera(DAVA::Camera *camera)
{
    CameraController::SetCamera(camera);

    angleVertical = 0.f;
    angleHorizontal = 0.f;
    
    moveStartPt = Vector2(0, 0);
    moveStopPt = Vector2(0, 0);
    
    zoomStartPt = Vector2(0, 0);
    zoomStopPt = Vector2(0, 0);
    
    radius = 10.f;
    zoomLevel = 1.f;
}

void PreviewCameraController::SetRadius(float32 _radius)
{
    radius = _radius;
}

void PreviewCameraController::Input(DAVA::UIEvent *event)
{
    if (!camera)return;

    CameraController::Input(event);
    
    if(UIEvent::BUTTON_1 == event->tid)
    {
        if(UIEvent::PHASE_BEGAN == event->phase)
        {
            moveStartPt = moveStopPt = event->point;
        }
        else if(UIEvent::PHASE_DRAG == event->phase)
        {
            moveStartPt = moveStopPt;
            moveStopPt = event->point;
            UpdateCamera();
        }
        else if(UIEvent::PHASE_ENDED == event->phase)
        {
            moveStartPt = moveStopPt;
            moveStopPt = event->point;
            UpdateCamera();
        }
    }
    else if(UIEvent::BUTTON_2 == event->tid)
    {
        if(UIEvent::PHASE_BEGAN == event->phase)
        {
            zoomStartPt = zoomStopPt = event->point;
        }
        else if(UIEvent::PHASE_DRAG == event->phase)
        {
        }
        else if(UIEvent::PHASE_ENDED == event->phase)
        {
            zoomStartPt = zoomStopPt;
            zoomStopPt = event->point;
            UpdateCamera();
        }
    }
}

void PreviewCameraController::UpdateCamera()
{
    Vector2 zoom = zoomStopPt - zoomStartPt;
    if(Vector2(0, 0) != zoom)
    {
        zoomStartPt = zoomStopPt;
        float32 delta = zoom.y;
        if(0 < delta)
        {
            zoomLevel /= 2;   
        }
        else
        {
            zoomLevel *= 2;   
        }
    }

    
    Vector3 target = camera->GetTarget();
    angleHorizontal += (moveStopPt.x - moveStartPt.x);
    angleVertical += (moveStopPt.y - moveStartPt.y);
    
    Vector3 position = target - radius * zoomLevel * Vector3(sinf(DegToRad(angleHorizontal)), 
                                                 cosf(DegToRad(angleHorizontal)), 
                                                 sinf(DegToRad(angleVertical)));
    
    
    camera->SetPosition(position);
}


// ***************** ScenePreviewControl *************** //
ScenePreviewControl::ScenePreviewControl(const Rect & rect)
    :   UI3DView(rect)
{
    needSetCamera = false;
    sceCamera = false;
    currentScenePath = "";
    rootNode = NULL;
    
    scene = new EditorScene();
    SetScene(scene);

    // Camera setup
    cameraController = new PreviewCameraController();
    cameraController->SetRadius(10.f);
}
    
ScenePreviewControl::~ScenePreviewControl()
{
    SafeRelease(cameraController);
    if (rootNode && scene)
    {
        scene->RemoveNode(rootNode);
        rootNode = NULL;
    }
    SafeRelease(scene);
}


void ScenePreviewControl::Input(DAVA::UIEvent *event)
{
    cameraController->Input(event);
    
    UIControl::Input(event);
}

void ScenePreviewControl::OpenScene(const String &pathToFile)
{
    if(currentScenePath.length())
    {
        scene->ReleaseRootNode(currentScenePath);
        scene->RemoveNode(rootNode);
        rootNode = NULL;
    }
    currentScenePath = pathToFile;
    
    SceneFile * file = new SceneFile();
    file->SetDebugLog(true);
    file->LoadScene(pathToFile.c_str(), scene);
    rootNode = scene->GetRootNode(pathToFile);
    scene->AddNode(rootNode);
    SafeRelease(file);
    
    needSetCamera = true;
    Camera *cam = scene->GetCamera(0);
    if(!cam)
    {
        Camera * cam = new Camera(scene);
        cam->SetName("preview-camera");
        cam->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
        cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
        
        cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
        
        scene->AddNode(cam);
        scene->AddCamera(cam);
        scene->SetCurrentCamera(cam);
        cameraController->SetCamera(cam);
        
        SafeRelease(cam);
        
        sceCamera = false;
    }
    else
    {
        sceCamera = true;
    }
}

void ScenePreviewControl::Update(float32 timeElapsed)
{
    UI3DView::Update(timeElapsed);
    
    if(needSetCamera)
    {
        needSetCamera = false;
        SetupCamera();
    }
}

void ScenePreviewControl::SetupCamera()
{
    Camera *camera = scene->GetCamera(0);
    if (camera)
    {
        AABBox3 sceneBox = rootNode->GetWTMaximumBoundingBox();
        Vector3 target = sceneBox.GetCenter();
        camera->SetTarget(target);
        Vector3 dir = (sceneBox.max - sceneBox.min); 
        float32 radius = dir.Length();
        if(sceCamera)
        {
            radius = 5.f;
        }
        
        camera->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
        scene->SetCurrentCamera(camera);
        scene->SetClipCamera(camera);
        
        cameraController->SetCamera(camera);
        cameraController->SetRadius(radius);
        cameraController->UpdateCamera();
    }
}