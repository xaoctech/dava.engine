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

    controlHeight = 100;
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
            zoomStartPt = zoomStopPt;
            zoomStopPt = event->point;
            UpdateCamera();
        }
        else if(UIEvent::PHASE_ENDED == event->phase)
        {
            zoomStartPt = zoomStopPt;
            zoomStopPt = event->point;
            UpdateCamera();
        }
    }
}

void PreviewCameraController::SetControlHeight(int32 height)
{
    controlHeight = height;
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
            zoomLevel *= (1 + zoom.y/controlHeight);   
        }
        else
        {
            zoomLevel /= (1 - zoom.y/controlHeight);   
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
    
    editorScene = new Scene();
    SetScene(editorScene);

    // Camera setup
    cameraController = new PreviewCameraController();
    cameraController->SetRadius(10.f);
    cameraController->SetControlHeight(rect.dy);
}
    
ScenePreviewControl::~ScenePreviewControl()
{
    SafeRelease(cameraController);
    if (rootNode && editorScene)
    {
        editorScene->RemoveNode(rootNode);
        rootNode = NULL;
    }
    SafeRelease(editorScene);
}


void ScenePreviewControl::Input(DAVA::UIEvent *event)
{
    cameraController->Input(event);
    
    UIControl::Input(event);
}

void ScenePreviewControl::RecreateScene()
{
    if(editorScene)
    {
        SetScene(NULL);
        SafeRelease(editorScene);
    }
    
    editorScene = new Scene();
    SetScene(editorScene);
}

bool ScenePreviewControl::OpenScene(const String &pathToFile)
{
    if(currentScenePath.length())
    {
        editorScene->ReleaseRootNode(currentScenePath);
        editorScene->RemoveNode(rootNode);
        rootNode = NULL;
    }
    
    RecreateScene();
    
    currentScenePath = pathToFile;
    rootNode = editorScene->GetRootNode(pathToFile);
    if(rootNode)
    {
        editorScene->AddNode(rootNode);
        
        needSetCamera = true;
        Camera *cam = editorScene->GetCamera(0);
        if(!cam)
        {
            Camera * cam = new Camera(editorScene);
            cam->SetName("preview-camera");
            cam->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
            cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
            
            cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
            
            editorScene->AddNode(cam);
            editorScene->AddCamera(cam);
            editorScene->SetCurrentCamera(cam);
            cameraController->SetCamera(cam);
            
            SafeRelease(cam);
            
            sceCamera = false;
        }
        else
        {
            sceCamera = true;
        }
    }
    else
    {
        currentScenePath = "";
    }
    
    return (NULL != rootNode);
}

void ScenePreviewControl::Update(float32 timeElapsed)
{
    UI3DView::Update(timeElapsed);
    
    if(needSetCamera)
    {
        needSetCamera = false;
        SetupCamera();
    }
    
    if(cameraController)
    {
        cameraController->Update(timeElapsed);
    }
}

void ScenePreviewControl::SetupCamera()
{
    Camera *camera = editorScene->GetCamera(0);
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
        editorScene->SetCurrentCamera(camera);
        editorScene->SetClipCamera(camera);
        
        cameraController->SetCamera(camera);
        cameraController->SetRadius(radius);
        cameraController->UpdateCamera();
    }
}