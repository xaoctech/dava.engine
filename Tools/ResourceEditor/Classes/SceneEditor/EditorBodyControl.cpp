#include "EditorBodyControl.h"


EditorBodyControl::EditorBodyControl(const Rect & rect)
    :   UIControl(rect)
{
    selectedNode = NULL;
    
    fontLight = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    fontLight->SetSize(12);
    fontLight->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

    fontDark = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    fontDark->SetSize(12);
    fontDark->SetColor(Color(0.1f, 0.1f, 0.1f, 1.0f));

    
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
    
    sceneTree = new UIHierarchy(Rect(0, 0, LEFT_SIDE_WIDTH, rect.dy));
    sceneTree->SetCellHeight(CELL_HEIGHT);
    sceneTree->SetDelegate(this);
    sceneTree->SetClipContents(true);
    sceneTree->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    sceneTree->GetBackground()->SetColor(Color(0.92f, 0.92f, 0.92f, 1.0f));
    AddControl(sceneTree);

    scene3dView = new UI3DView(Rect(LEFT_SIDE_WIDTH + 10, 10, rect.dx - LEFT_SIDE_WIDTH - RIGHT_SIDE_WIDTH - 20, SCENE_HEIGHT));
    scene3dView->SetDebugDraw(true);
    scene3dView->SetInputEnabled(false);
    AddControl(scene3dView);
    
    CreateScene();
    
    CreatePropertyPanel();
}
    
EditorBodyControl::~EditorBodyControl()
{
    ReleasePropertyPanel();
    
    ReleaseScene();
    
    SafeRelease(sceneTree);
    
    SafeRelease(fontLight);
    SafeRelease(fontDark);
}

void EditorBodyControl::CreateScene()
{
    scene = new GameScene();
    // Camera setup
    cameraController = new WASDCameraController(40);
    Camera * cam = new Camera(scene);
    cam->SetName("editor-camera");
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
    
    Camera * cam2 = new Camera(scene);
    cam2->SetName("editor-top-camera");
    cam2->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
    cam2->SetUp(Vector3(1.0f, 0.0f, 0.0f));
    cam2->SetPosition(Vector3(0.0f, 0.0f, 200.0f));
    cam2->SetTarget(Vector3(0.0f, 250.0f, 0.0f));
    
    cam2->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
    
    scene->AddNode(cam2);
    scene->AddCamera(cam2);
    
    SafeRelease(cam2);
    
    
    LandscapeNode * node = new LandscapeNode(scene);
    //node->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
    AABBox3 box(Vector3(198, 201, 0), Vector3(-206, -203, 13.7f));
    
    node->SetDebugFlags(LandscapeNode::DEBUG_DRAW_ALL);
#if 1
    node->BuildLandscapeFromHeightmapImage(LandscapeNode::RENDERING_MODE_DETAIL_SHADER, "~res:/Landscape/hmp2_1.png", box);
    
    Texture::EnableMipmapGeneration();
    node->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, "~res:/Landscape/tex3.png");
    node->SetTexture(LandscapeNode::TEXTURE_DETAIL, "~res:/Landscape/detail_gravel.png");
    Texture::DisableMipmapGeneration();
#else  
    node->BuildLandscapeFromHeightmapImage(LandscapeNode::RENDERING_MODE_BLENDED_SHADER, "~res:/Landscape/hmp2_1.png", box);
    
    Texture::EnableMipmapGeneration();
    node->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, "~res:/Landscape/blend/d.png");
    node->SetTexture(LandscapeNode::TEXTURE_TEXTURE1, "~res:/Landscape/blend/s.png");
    node->SetTexture(LandscapeNode::TEXTURE_TEXTUREMASK, "~res:/Landscape/blend/mask.png");
    Texture::DisableMipmapGeneration();
#endif
    
    node->SetName("landscapeNode");
    scene->AddNode(node);
    
    scene3dView->SetScene(scene);
}

void EditorBodyControl::ReleaseScene()
{
    SafeRelease(scene3dView);
    SafeRelease(scene);
    SafeRelease(cameraController);
}

void EditorBodyControl::CreatePropertyPanel()
{
    localMatrixControl = new EditMatrixControl(Rect(0, 0, RIGHT_SIDE_WIDTH, MATRIX_HEIGHT));
    localMatrixControl->OnMatrixChanged = Message(this, &EditorBodyControl::OnLocalTransformChanged);
    
    worldMatrixControl = new EditMatrixControl(Rect(0, 0, RIGHT_SIDE_WIDTH, MATRIX_HEIGHT), true);

    lookAtButton = CreateButton(Rect(0, 0, RIGHT_SIDE_WIDTH,BUTTON_HEIGHT), L"Look At Object");
    removeNodeButton = CreateButton(Rect(0, 0, RIGHT_SIDE_WIDTH,BUTTON_HEIGHT), L"Remove Object");
    enableDebugFlagsButton = CreateButton(Rect(0, 0, RIGHT_SIDE_WIDTH,BUTTON_HEIGHT), L"Debug Flags");
    
    lookAtButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnLookAtButtonPressed));
    removeNodeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRemoveNodeButtonPressed));
    enableDebugFlagsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnEnableDebugFlagsPressed));
    
    Rect fullRect = GetRect();
    activePropertyPanel = new PropertyPanel(Rect(fullRect.dx - RIGHT_SIDE_WIDTH, 0, RIGHT_SIDE_WIDTH, size.y));
    
    nodeName = SafeRetain(activePropertyPanel->AddHeader(L"Node name:"));
    
    activePropertyPanel->AddHeader(L"Local Matrix:");
    activePropertyPanel->AddPropertyControl(localMatrixControl);
    activePropertyPanel->AddHeader(L"World Matrix:");
    activePropertyPanel->AddPropertyControl(worldMatrixControl);
    nodeBoundingBoxMin = SafeRetain(activePropertyPanel->AddHeader(L"-"));
    nodeBoundingBoxMax = SafeRetain(activePropertyPanel->AddHeader(L"-"));
    activePropertyPanel->AddPropertyControl(lookAtButton);
    activePropertyPanel->AddPropertyControl(removeNodeButton);
    activePropertyPanel->AddPropertyControl(enableDebugFlagsButton);
    
    AddControl(activePropertyPanel);
}

void EditorBodyControl::ReleasePropertyPanel()
{
    SafeRelease(nodeName);
    SafeRelease(nodeBoundingBoxMin);
    SafeRelease(nodeBoundingBoxMax);
    SafeRelease(lookAtButton);
    SafeRelease(removeNodeButton);
    SafeRelease(enableDebugFlagsButton);
    SafeRelease(activePropertyPanel);
}

UIButton * EditorBodyControl::CreateButton(Rect r, const WideString &text)
{
    UIButton *btn = new UIButton(r);

    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    
    btn->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    btn->GetStateBackground(UIControl::STATE_SELECTED)->SetColor(Color(0.0f, 0.0f, 1.0f, 0.2f));
    
    
    btn->SetStateFont(UIControl::STATE_PRESSED_INSIDE, fontLight);
    btn->SetStateFont(UIControl::STATE_DISABLED, fontLight);
    btn->SetStateFont(UIControl::STATE_NORMAL, fontLight);
    btn->SetStateFont(UIControl::STATE_SELECTED, fontLight);
    
    btn->SetStateText(UIControl::STATE_PRESSED_INSIDE, text);
    btn->SetStateText(UIControl::STATE_DISABLED, text);
    btn->SetStateText(UIControl::STATE_NORMAL, text);
    btn->SetStateText(UIControl::STATE_SELECTED, text);
    return btn;
}


bool EditorBodyControl::IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode)
{
    if(forHierarchy == sceneTree)
    {
        if (forNode) 
        {
            return ((SceneNode*)forNode)->GetChildrenCount() > 0;
        }
        
        return scene->GetChildrenCount() > 0;
    }
    
    return false;
}

int32 EditorBodyControl::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
    if(forHierarchy == sceneTree)
    {
        if (forParent) 
        {
            return ((SceneNode*)forParent)->GetChildrenCount();
        }
        
        return scene->GetChildrenCount();
    }

    return 0;
}

void * EditorBodyControl::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
    if(forHierarchy == sceneTree)
    {
        if (forParent) 
        {
            return ((SceneNode*)forParent)->GetChild(index);
        }
        
        return scene->GetChild(index);
    }
    
    return NULL;
}

UIHierarchyCell * EditorBodyControl::CellForNode(UIHierarchy *forHierarchy, void *node)
{
    UIHierarchyCell *c = NULL;
    if(forHierarchy == sceneTree)
    {
        c = forHierarchy->GetReusableCell("SceneNode cell"); //try to get cell from the reusable cells store
        if(!c)
        { //if cell of requested type isn't find in the store create new cell
            c = new UIHierarchyCell(Rect(0, 0, LEFT_SIDE_WIDTH, CELL_HEIGHT), "SceneNode cell");
        }
        
        //fill cell whith data
        SceneNode *n = (SceneNode *)node;
        
        c->text->SetText(StringToWString(n->GetName()));
    }
//    else
//    {
//        c = forHierarchy->GetReusableCell("Library cell"); //try to get cell from the reusable cells store
//        if(!c)
//        { //if cell of requested type isn't find in the store create new cell
//            c = new UIHierarchyCell(Rect(0, 0, RIGHT_SIDE_WIDTH, CELL_HEIGHT), "Library cell");
//        }
//        
//        c->text->SetText(L"Cell");
//    }
    
    c->text->SetFont(fontDark);
    c->text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);

    Color color(0.1f, 0.5f, 0.05f, 1.0f);
    c->openButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->openButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    c->openButton->SetStateDrawType(UIControl::STATE_HOVER, UIControlBackground::DRAW_FILL);
    c->openButton->GetStateBackground(UIControl::STATE_NORMAL)->color = color;
    c->openButton->GetStateBackground(UIControl::STATE_HOVER)->color = color + 0.1f;
    c->openButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->color = color + 0.3f;

    c->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    c->GetStateBackground(UIControl::STATE_NORMAL)->color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(1.0f, 0.8f, 0.8f, 1.0f);
    
    return c;//returns cell
}

void EditorBodyControl::OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell)
{
    if(forHierarchy == sceneTree)
    {
        UIHierarchyNode * hNode = selectedCell->GetNode();
        SceneNode * node = dynamic_cast<SceneNode*>((BaseObject*)hNode->GetUserNode());
        if (node)
        {
            MeshInstanceNode * prevSelMesh = dynamic_cast<MeshInstanceNode*>(selectedNode);
            if (prevSelMesh)
            {
                prevSelMesh->SetDebugFlags(0);
            }   
            selectedNode = node;
            MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode*>(node);
            if (mesh)
            {
                AABBox3 bbox = mesh->GetBoundingBox();
                AABBox3 transformedBox;
                bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);
                
                mesh->SetDebugFlags(SceneNode::DEBUG_DRAW_AABBOX | SceneNode::DEBUG_DRAW_LOCAL_AXIS);
                nodeBoundingBoxMin->SetText(Format(L"Min: (%0.2f, %0.2f, %0.2f)", 
                                                   transformedBox.min.x, transformedBox.min.y, transformedBox.min.z));
                nodeBoundingBoxMax->SetText(Format(L"Max: (%0.2f, %0.2f, %0.2f)", 
                                                   transformedBox.max.x, transformedBox.max.y, transformedBox.max.z));
            }else
            {
                nodeBoundingBoxMin->SetText(L"Bounding Box:");
                nodeBoundingBoxMax->SetText(L"Not available for this node");
            }
            
            Camera * camera = dynamic_cast<Camera*> (node);
            if (camera)
            {
                scene->SetCurrentCamera(camera);
                Camera * cam2 = scene->GetCamera(0);
                scene->SetClipCamera(cam2);
                //cameraController->SetCamera(camera);
                
                nodeBoundingBoxMin->SetText(Format(L"fov: %f, aspect: %f", camera->GetFOV(), camera->GetAspect()));
                nodeBoundingBoxMax->SetText(Format(L"znear: %f, zfar: %f", camera->GetZNear(), camera->GetZFar()));
            }
            
            localMatrixControl->SetMatrix(selectedNode->GetLocalTransform());
            worldMatrixControl->SetMatrix(selectedNode->GetWorldTransform());
            
            nodeName->SetText(StringToWString(selectedNode->GetFullName()));
        }
        
        List<UIControl*> children = sceneTree->GetVisibleCells();
        for(List<UIControl*>::iterator it = children.begin(); it != children.end(); ++it)
        {
            UIControl *ctrl = (*it);
            ctrl->SetSelected(false, false);
        }
        
        selectedCell->SetSelected(true, false);
    }
}

void EditorBodyControl::Input(DAVA::UIEvent *event)
{
    cameraController->Input(event);
    
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        if (event->keyChar == '1')
            cameraController->SetSpeed(40);
        if (event->keyChar == '2')
            cameraController->SetSpeed(80);
        if (event->keyChar == '3')
            cameraController->SetSpeed(160);
        if (event->keyChar == '4')
            cameraController->SetSpeed(320);
        
        Camera * newCamera = 0;
        if (event->keyChar == 'z')newCamera = scene->GetCamera(0);
        if (event->keyChar == 'x')newCamera = scene->GetCamera(1);
        if (event->keyChar == 'c')newCamera = scene->GetCamera(2);
        if (event->keyChar == 'v')newCamera = scene->GetCamera(3);
        if (event->keyChar == 'b')newCamera = scene->GetCamera(4);
        if (newCamera)
        {
            scene->SetCurrentCamera(newCamera);
            scene->SetClipCamera(scene->GetCamera(0));
        }
    }   
    
	if (event->phase == UIEvent::PHASE_BEGAN)
	{
		inTouch = true;	
		touchStart = event->point;
		touchTankAngle = currentTankAngle;
	}
	
	if (event->phase == UIEvent::PHASE_DRAG)
	{
		touchCurrent = event->point;
		
		float32 dist = (touchCurrent.x - touchStart.x);
		//Logger::Debug("%f, %f", currentTankAngle, dist);
		currentTankAngle = touchTankAngle + dist;
	}
	
	if (event->phase == UIEvent::PHASE_ENDED)
	{
		touchCurrent = event->point;
		rotationSpeed = (touchCurrent.x - touchStart.x);
		inTouch = false;
		startRotationInSec = 0.0f;
	}
    
    UIControl::Input(event);
}

void EditorBodyControl::Update(float32 timeElapsed)
{
//    Camera * cam = scene->GetCurrentCamera();
//    Camera * frustumCam = scene->GetClipCamera();
    
//    if (!cam)
//    {
//        cameraInfo->SetText(L"no active camera");
//    }else
//    {
//        WideString cameraInfoString = Format(L"cam: %s pos(%f, %f, %f) dir(%f, %f, %f) up(%f, %f, %f)", 
//                                             cam->GetName().c_str(), 
//                                             cam->GetPosition().x, cam->GetPosition().y, cam->GetPosition().z, 
//                                             cam->GetDirection().x, cam->GetDirection().y, cam->GetDirection().z, 
//                                             cam->GetUp().x, cam->GetUp().y, cam->GetUp().z);
//        cameraInfo->SetText(cameraInfoString);
//    }
    
//    if (!frustumCam)
//    {
//        clipCameraInfo->SetText(L"no clip camera");
//    }else if (frustumCam == cam)
//    {
//        clipCameraInfo->SetText(L"same camera");
//    }else
//    {
//        WideString cameraInfoString = Format(L"cam: %s pos(%f, %f, %f) dir(%f, %f, %f) up(%f, %f, %f)", 
//                                             frustumCam->GetName().c_str(), 
//                                             frustumCam->GetPosition().x, frustumCam->GetPosition().y, frustumCam->GetPosition().z, 
//                                             frustumCam->GetDirection().x, frustumCam->GetDirection().y, frustumCam->GetDirection().z, 
//                                             frustumCam->GetUp().x, frustumCam->GetUp().y, frustumCam->GetUp().z);
//        clipCameraInfo->SetText(cameraInfoString);
//    }
    
    
	startRotationInSec -= timeElapsed;
	if (startRotationInSec < 0.0f)
		startRotationInSec = 0.0f;
    
	if (startRotationInSec == 0.0f)
	{
		if (Abs(rotationSpeed) > 8.0)
		{
			rotationSpeed = rotationSpeed * 0.8f;
		}
		
		currentTankAngle += timeElapsed * rotationSpeed;
	}

    UIControl::Update(timeElapsed);
}

void EditorBodyControl::OnLocalTransformChanged(BaseObject * object, void * userData, void * callerData)
{
    if (selectedNode)
    {
        selectedNode->SetLocalTransform(localMatrixControl->GetMatrix());
    }
}

void EditorBodyControl::OnLookAtButtonPressed(BaseObject * obj, void *, void *)
{
    MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode*>(selectedNode);
    if (mesh)
    {
        AABBox3 bbox = mesh->GetBoundingBox();
        AABBox3 transformedBox;
        bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);
        Vector3 center = transformedBox.GetCenter();
        scene->GetCurrentCamera()->SetTarget(center);
    }
}

void EditorBodyControl::OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *)
{
    if (selectedNode)
    {
        SceneNode * parentNode = selectedNode->GetParent();
        if (parentNode)
        {
            parentNode->RemoveNode(selectedNode);
            selectedNode = 0;
            sceneTree->Refresh();
        }
    }
}

void EditorBodyControl::OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *)
{
    if (selectedNode)
    {
        if (selectedNode->GetDebugFlags() & SceneNode::DEBUG_DRAW_ALL)
        {
            selectedNode->SetDebugFlags(0, true);
        }else
        {
            selectedNode->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL, true);
        }
    }
}

void EditorBodyControl::OpenScene(const String &pathToFile)
{
    SceneFile * file = new SceneFile();
    file->SetDebugLog(true);
    file->LoadScene(pathToFile.c_str(), scene);
    scene->AddNode(scene->GetRootNode(pathToFile));
    SafeRelease(file);
    
    if (scene->GetCamera(0))
    {
        scene->SetCurrentCamera(scene->GetCamera(0));
        cameraController->SetCamera(scene->GetCamera(0));
    }
    sceneTree->Refresh();
}

void EditorBodyControl::WillAppear()
{
    sceneTree->Refresh();
}

void EditorBodyControl::ShowProperties(bool show)
{
    if(show && !activePropertyPanel->GetParent())
    {
        AddControl(activePropertyPanel);
        
        Rect r = scene3dView->GetRect();
        r.dx -= RIGHT_SIDE_WIDTH;
        scene3dView->SetRect(r);
    }
    else if(!show && activePropertyPanel->GetParent())
    {
        RemoveControl(activePropertyPanel);

        Rect r = scene3dView->GetRect();
        r.dx += RIGHT_SIDE_WIDTH;
        scene3dView->SetRect(r);
    }
}

bool EditorBodyControl::PropertiesAreShown()
{
    return (activePropertyPanel->GetParent() != NULL);
}
