#include "EditorBodyControl.h"
#include "ControlsFactory.h"
#include "OutputManager.h"
#include "OutputPanelControl.h"
#include "../BeastProxy.h"
#include "../SceneNodeUserData.h"
#include "PropertyControlCreator.h"
#include "EditorSettings.h"
#include "../config.h"

#include "SceneInfoControl.h"
#include "SceneValidator.h"
#include "../LightmapsPacker.h"

#include "ErrorNotifier.h"

#include "LandscapeEditorColor.h"
#include "LandscapeEditorHeightmap.h"
#include "SceneEditorScreenMain.h"
#include "LandscapeToolsSelection.h"


#include "SceneGraph.h"
#include "DataGraph.h"
#include "EntitiesGraph.h"

#if defined (DAVA_QT)        
#include "../Qt/SceneDataManager.h"
#include "../Qt/SceneData.h"
#endif //#if defined (DAVA_QT)

EditorBodyControl::EditorBodyControl(const Rect & rect)
    :   UIControl(rect)
	, beastManager(0)
{
    currentViewportType = ResourceEditor::VIEWPORT_DEFAULT;
    
    scene = NULL;
	
    ControlsFactory::CusomizeBottomLevelControl(this);

    sceneGraph = new SceneGraph(this, rect);
#if defined (DAVA_QT)
    currentGraph = sceneGraph;
#else //#if defined (DAVA_QT)
    dataGraph = new DataGraph(this, rect);
	entitiesGraph = new EntitiesGraph(this, rect);
    currentGraph = NULL;
#endif //#if defined (DAVA_QT)

    InitControls();

    scene3dView->SetDebugDraw(true);
    scene3dView->SetInputEnabled(false);
    AddControl(scene3dView);

    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    sceneInfoControl = new SceneInfoControl(Rect(rect.dx - rightSideWidth * 2 , 0, rightSideWidth, rightSideWidth));
    AddControl(sceneInfoControl);

#if defined (DAVA_QT)
    scene = NULL;
    cameraController = NULL;
#else //#if defined (DAVA_QT)
    CreateScene(true);
#endif //#if defined (DAVA_QT)

    
	CreateModificationPanel();
    CreateLandscapeEditor();

	mainCam = 0;
	debugCam = 0;
    
    propertyShowState = EPSS_ONSCREEN;

#if defined (DAVA_QT)
    AddControl(currentGraph->GetPropertyPanel());
#else //#if defined (DAVA_QT)
    ToggleSceneGraph();
#endif //#if defined (DAVA_QT)
}

#if defined (DAVA_QT)
void EditorBodyControl::InitControls()
{
    Rect rect = GetRect();

    scene3dView = new UI3DView(Rect(SCENE_OFFSET, SCENE_OFFSET,
                                    rect.dx - 2 * SCENE_OFFSET,
                                    rect.dy - 2 * SCENE_OFFSET));
}
#else //#if defined (DAVA_QT)
void EditorBodyControl::InitControls()
{
    Rect rect = GetRect();
    
    bool showOutput = EditorSettings::Instance()->GetShowOutput();
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    if(showOutput)
    {
        scene3dView = new UI3DView(Rect(leftSideWidth + SCENE_OFFSET, SCENE_OFFSET,
                                        rect.dx - leftSideWidth - 2 * SCENE_OFFSET,
                                        rect.dy - 2 * SCENE_OFFSET - ControlsFactory::OUTPUT_PANEL_HEIGHT));
        
        outputPanel = new OutputPanelControl(scene, Rect(leftSideWidth, rect.dy - ControlsFactory::OUTPUT_PANEL_HEIGHT,
                                                         rect.dx - leftSideWidth,
                                                         ControlsFactory::OUTPUT_PANEL_HEIGHT));
        ControlsFactory::CustomizePanelControl(outputPanel, false);
        AddControl(outputPanel);
        
    }
    else
    {
        scene3dView = new UI3DView(Rect(leftSideWidth + SCENE_OFFSET, SCENE_OFFSET,
                                        rect.dx - leftSideWidth - 2 * SCENE_OFFSET,
                                        rect.dy - 2 * SCENE_OFFSET));
        
        outputPanel = NULL;
        
    }
}
#endif //#if defined (DAVA_QT)




EditorBodyControl::~EditorBodyControl()
{
    SafeRelease(sceneGraph);
    currentGraph = NULL;
    
    SafeRelease(sceneInfoControl);
    
    ReleaseModificationPanel();
    
    
    ReleaseLandscapeEditor();
    
#if defined (DAVA_QT)
    SafeRelease(scene);
    SafeRelease(cameraController);
#else //#if defined (DAVA_QT)
    ReleaseScene();
    SafeRelease(dataGraph);
	SafeRelease(entitiesGraph);
    SafeRelease(outputPanel);
#endif //#if defined (DAVA_QT)
  
    SafeRelease(scene3dView);
}

#if !defined (DAVA_QT)
void EditorBodyControl::CreateScene(bool withCameras)
{
    scene = new EditorScene();
    // Camera setup
    cameraController = new WASDCameraController(EditorSettings::Instance()->GetCameraSpeed());
    
    if(withCameras)
    {
        Camera * cam = new Camera();
        cam->SetName("editor.main-camera");
        cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
        
        cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
        
        scene->AddNode(cam);
        scene->AddCamera(cam);
        scene->SetCurrentCamera(cam);
        cameraController->SetScene(scene);
        
        SafeRelease(cam);
        
        Camera * cam2 = new Camera();
        cam2->SetName("editor.debug-camera");
        cam2->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam2->SetPosition(Vector3(0.0f, 0.0f, 200.0f));
        cam2->SetTarget(Vector3(0.0f, 250.0f, 0.0f));
        
        cam2->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
        
        scene->AddNode(cam2);
        scene->AddCamera(cam2);
        
        SafeRelease(cam2);
    }
    
    scene3dView->SetScene(scene);
    sceneInfoControl->SetWorkingScene(scene);
    
    sceneGraph->SetScene(scene);
    dataGraph->SetScene(scene);
	entitiesGraph->SetScene(scene);
}

void EditorBodyControl::ReleaseScene()
{
    //TODO: need to release root nodes?
    ResetSelection();
    
    SafeRelease(scene);
    SafeRelease(cameraController);
}

void EditorBodyControl::OpenScene(const String &pathToFile, bool editScene)
{
    if (FileSystem::Instance()->GetExtension(pathToFile) == ".sce")
    {
        if(editScene)
        {
            SceneNode *rootNode = scene->GetRootNode(pathToFile);
            
            mainFilePath = pathToFile;
            scene->AddNode(rootNode);
        }
        else
        {
            SceneNode *rootNode = scene->GetRootNode(pathToFile)->Clone();
            
            KeyedArchive * customProperties = rootNode->GetCustomProperties();
            customProperties->SetString("editor.referenceToOwner", pathToFile);
            
            rootNode->SetSolid(true);
            scene->AddNode(rootNode);
            
            SafeRelease(rootNode);
        }
        
        if (scene->GetCamera(0))
        {
            scene->SetCurrentCamera(scene->GetCamera(0));
            cameraController->SetScene(scene);
        }
    }
    else if(FileSystem::Instance()->GetExtension(pathToFile) == ".sc2")
    {
        if(editScene)
        {
            SceneNode * rootNode = scene->GetRootNode(pathToFile);
            if(rootNode)
            {
                mainFilePath = pathToFile;
                for (int ci = 0; ci < rootNode->GetChildrenCount(); ++ci)
                {
                    //рут нода это сама сцена в данном случае
                    scene->AddNode(rootNode->GetChild(ci));
                }
            }
        }
        else
        {
            SceneNode * rootNode = scene->GetRootNode(pathToFile)->Clone();
            
            KeyedArchive * customProperties = rootNode->GetCustomProperties();
            customProperties->SetString("editor.referenceToOwner", pathToFile);
            
            rootNode->SetSolid(true);
            scene->AddNode(rootNode);
            
            Camera *currCamera = scene->GetCurrentCamera();
            if(currCamera)
            {
                Vector3 pos = currCamera->GetPosition();
                Vector3 direction  = currCamera->GetDirection();
                
                Vector3 nodePos = pos + 10 * direction;
                nodePos.z = 0;
                
                LandscapeNode * ls = scene->GetLandScape(scene);
                if(ls)
                {
                    Vector3 result;
                    bool res = ls->PlacePoint(nodePos, result);
                    if(res)
                    {
                        nodePos = result;
                    }
                }
                
                Matrix4 mod;
                mod.CreateTranslation(nodePos);
                rootNode->SetLocalTransform(rootNode->GetLocalTransform() * mod);
            }
            
            SafeRelease(rootNode);
        }
        
        Refresh();
    }
    
    SelectNodeAtTree(scene->GetSelection());
    SceneValidator::Instance()->ValidateScene(scene);
    SceneValidator::Instance()->EnumerateSceneTextures();
}


const String &EditorBodyControl::GetFilePath()
{
    return mainFilePath;
}

void EditorBodyControl::SetFilePath(const String &newFilePath)
{
    mainFilePath = newFilePath;
}


void EditorBodyControl::ShowProperties(bool show)
{
    if(currentGraph)
    {
        int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
        if(show && !currentGraph->GetPropertyPanel()->GetParent())
        {
            if(!ControlsAreLocked())
            {
                AddControl(currentGraph->GetPropertyPanel());
                
                ChangeControlWidthRight(scene3dView, -rightSideWidth);
                if(outputPanel)
                {
                    ChangeControlWidthRight(outputPanel, -rightSideWidth);
                }
            }
        }
        else if(!show && currentGraph->GetPropertyPanel()->GetParent())
        {
            RemoveControl(currentGraph->GetPropertyPanel());
            
            ChangeControlWidthRight(scene3dView, rightSideWidth);
            if(outputPanel)
            {
                ChangeControlWidthRight(outputPanel, rightSideWidth);
            }
        }
    }
    
    propertyShowState = (show) ? EPSS_ONSCREEN : EPSS_HIDDEN;
}

bool EditorBodyControl::PropertiesAreShown()
{
    if(currentGraph)
    {
        return currentGraph->PropertiesOnScreen();
    }
    
    return false;
}


void EditorBodyControl::ToggleGraph(GraphBase *graph)
{
    bool needToResizeControls = false;
    
    if(currentGraph == graph)
    {
        if(graph->GetGraphPanel()->GetParent())
        {
            RemoveControl(graph->GetGraphPanel());
            needToResizeControls = true;
        }
        else
        {
            if(!ControlsAreLocked())
            {
                AddControl(graph->GetGraphPanel());
                needToResizeControls = true;
                
                graph->RefreshGraph();
                graph->UpdatePropertyPanel();
                
            }
        }
    }
    else  //if(currentGraph == graph)
    {
        if(!ControlsAreLocked())
        {
            ePropertyShowState oldState = propertyShowState;
            ShowProperties(false);
            
            if(currentGraph && currentGraph->GetGraphPanel()->GetParent())
            {
                RemoveControl(currentGraph->GetGraphPanel());
            }
            else if(currentGraph && !currentGraph->GetGraphPanel()->GetParent())
            {
                needToResizeControls = true;
            }
            
            AddControl(graph->GetGraphPanel());
            
            currentGraph = graph;
            if(EPSS_ONSCREEN == oldState)
            {
                ShowProperties(true);
            }
            
            graph->RefreshGraph();
            graph->UpdatePropertyPanel();
        }
    }
    
    if(needToResizeControls)
    {
        int32 leftSideWidth = graph->GetGraphPanel()->GetSize().x;
        if(graph->GetGraphPanel()->GetParent())
        {
            ChangeControlWidthLeft(scene3dView, leftSideWidth);
            if(outputPanel)
            {
                ChangeControlWidthLeft(outputPanel, leftSideWidth);
            }
        }
        else
        {
            ChangeControlWidthLeft(scene3dView, -leftSideWidth);
            if(outputPanel)
            {
                ChangeControlWidthLeft(outputPanel, -leftSideWidth);
            }
        }
    }
}


void EditorBodyControl::ToggleSceneGraph()
{
    ToggleGraph(sceneGraph);
}

void EditorBodyControl::ToggleDataGraph()
{
    ToggleGraph(dataGraph);
}

void EditorBodyControl::ToggleEntities()
{
	ToggleGraph(entitiesGraph);
}


void EditorBodyControl::UpdateLibraryState(bool isShown, int32 width)
{
    if(isShown)
    {
        ShowProperties(false);
        
        ChangeControlWidthRight(scene3dView, -width);
        if(outputPanel)
        {
            ChangeControlWidthRight(outputPanel, -width);
        }
    }
    else
    {
        int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
        ChangeControlWidthRight(scene3dView, rightSideWidth);
        if(outputPanel)
        {
            ChangeControlWidthRight(outputPanel, rightSideWidth);
        }
    }
}


void EditorBodyControl::BakeNode(SceneNode *node)
{
    if(node->GetSolid())
    {
        node->BakeTransforms();
        return;
    }
    
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        BakeNode(node->GetChild(i));
    }
}

void EditorBodyControl::RemoveIdentityNodes(DAVA::SceneNode *node)
{
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        SceneNode *removedChild = node->GetChild(i);
        
        if(
           (removedChild->GetFlags() & SceneNode::NODE_LOCAL_MATRIX_IDENTITY)
           &&   (typeid(SceneNode) == typeid(*removedChild))
           &&   (typeid(LodNode) != typeid(*node))
           &&   (removedChild->GetChildrenCount() == 1))
        {
            SceneNode *child = SafeRetain(removedChild->GetChild(0));
            removedChild->RemoveNode(child);
            node->AddNode(child);
            SafeRelease(child);
            
            node->RemoveNode(removedChild);
            
            i = -1;
        }
        else
        {
            RemoveIdentityNodes(removedChild);
        }
    }
}

void EditorBodyControl::FindIdentityNodes(SceneNode *node)
{
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        SceneNode *child = node->GetChild(i);
        
        if(child->GetSolid())
        {
            RemoveIdentityNodes(child);
        }
        else
        {
            FindIdentityNodes(child);
        }
    }
}


void EditorBodyControl::BakeScene()
{
    if(scene)
    {
#if !defined(DAVA_QT)
        ResetSelection();
#endif //#if !defined(DAVA_QT)
        
        BakeNode(scene);
        FindIdentityNodes(scene);
        
        Refresh();
    }
}


void EditorBodyControl::ChangeControlWidthRight(UIControl *c, float32 width)
{
    Rect r = c->GetRect();
    r.dx += width;
    c->SetRect(r);
}

void EditorBodyControl::ChangeControlWidthLeft(UIControl *c, float32 width)
{
    Rect r = c->GetRect();
    r.dx -= width;
    r.x += width;
    c->SetRect(r);
}


#endif //#if !defined (DAVA_QT)

void RemoveDeepCamera(SceneNode * curr)
{
	SceneNode * cam;
	
	cam = curr->FindByName("editor.main-camera");
	while (cam)
	{
		cam->GetParent()->RemoveNode(cam);
		cam = curr->FindByName("editor.main-camera");
	}
	
	cam = curr->FindByName("editor.debug-camera");
	while (cam)
	{
		cam->GetParent()->RemoveNode(cam);
		cam = curr->FindByName("editor.debug-camera");
	}	
}

void EditorBodyControl::PushDebugCamera()
{
	mainCam = scene->FindByName("editor.main-camera");
	if (mainCam)
	{
		SafeRetain(mainCam);
		scene->RemoveNode(mainCam);
	}
	
	debugCam = scene->FindByName("editor.debug-camera");
	if (debugCam)
	{
		SafeRetain(debugCam);
		scene->RemoveNode(debugCam);
	}
	
	RemoveDeepCamera(scene);
}

void EditorBodyControl::PopDebugCamera()
{
	if (mainCam)
	{
		scene->AddNode(mainCam);
		SafeRelease(mainCam);
	}
	
	if (debugCam)
	{
		scene->AddNode(debugCam);
		SafeRelease(debugCam);
	}
	
	mainCam = 0;
	debugCam = 0;
}

void EditorBodyControl::CreateModificationPanel(void)
{
    modificationPanel = new ModificationsPanel(this, scene3dView->GetRect());
    modificationPanel->SetScene(scene);
    
    AddControl(modificationPanel);
}

void EditorBodyControl::ReleaseModificationPanel()
{
    SafeRelease(modificationPanel);
}


void EditorBodyControl::PlaceOnLandscape()
{
	SceneNode * selection = scene->GetProxy();
	if (selection && modificationPanel->IsModificationMode())
	{
		Vector3 result;
		LandscapeNode * ls = scene->GetLandScape(scene);
		if (ls)
		{
			const Matrix4 & itemWT = selection->GetWorldTransform();
			Vector3 p = Vector3(0,0,0) * itemWT;
			bool res = ls->PlacePoint(p, result);
			if (res)
			{
				Vector3 offs = result - p;
				Matrix4 invItem;
				Matrix4 mod;
				mod.CreateTranslation(offs);
				selection->SetLocalTransform(selection->GetLocalTransform() * mod);
			}						
		}
	}
}

void EditorBodyControl::PlaceOnLandscape(SceneNode *node)
{
	if(node)
	{
		Vector3 result;
		LandscapeNode * ls = scene->GetLandScape(scene);
		if (ls)
		{
			const Matrix4 & itemWT = node->GetWorldTransform();
			Vector3 p = Vector3(0,0,0) * itemWT;
			bool res = ls->PlacePoint(p, result);
			if (res)
			{
				Vector3 offs = result - p;
				Matrix4 invItem;
				Matrix4 mod;
				mod.CreateTranslation(offs);
				node->SetLocalTransform(node->GetLocalTransform() * mod);
			}						
		}
	}
}


void EditorBodyControl::Input(DAVA::UIEvent *event)
{    
    if(LandscapeEditorActive())
    {
        bool processed = currentLandscapeEditor->Input(event);
        if(!processed)
        {
            cameraController->Input(event);
        }
        UIControl::Input(event);
        return;
    }
    
    
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
        if(!tf)
        {
            modificationPanel->Input(event);
            
            Camera * newCamera = 0;
            switch(event->tid)
            {
				case DVKEY_ESCAPE:
                {
                    UIControl *c = UIControlSystem::Instance()->GetFocusedControl();
                    if(c == this || c == scene3dView)
                    {
#if defined (DAVA_QT)
                        SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
                        activeScene->SelectNode(NULL);
#else //#if defined (DAVA_QT)                        
                        ResetSelection();
#endif //#if defined (DAVA_QT)
                    }
                    
                    break;
                }
					
				case DVKEY_BACKSPACE:
                {
                    bool cmdIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL);
                    if(cmdIsPressed)
                    {
                        sceneGraph->RemoveWorkingNode();
                    }
                    break;
                }
					

                case DVKEY_C:
                    newCamera = scene->GetCamera(2);
                    break;

                case DVKEY_V:
                    newCamera = scene->GetCamera(3);
                    break;

                case DVKEY_B:
                    newCamera = scene->GetCamera(4);
                    break;

                default:
                    break;
            }
            
            if (newCamera)
            {
                scene->SetCurrentCamera(newCamera);
                scene->SetClipCamera(scene->GetCamera(0));
            }
        }
	}
	
	SceneNode * selection = scene->GetProxy();
	//selection with second mouse button 

	if (event->tid == UIEvent::BUTTON_1)
	{
		if (event->phase == UIEvent::PHASE_BEGAN)
		{
			isDrag = false;
			inTouch = true;
			touchStart = event->point;
		}
		else if (event->phase == UIEvent::PHASE_DRAG)
		{
			if (!isDrag)
			{
				Vector2 d = event->point - touchStart;
				if (d.Length() > 5 && modificationPanel->IsModificationMode())
				{
					isDrag = true;
					if (selection && InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
					{//copy object
						SceneNode * clone = 0;
						clone = selection->Clone(clone);
						selection->GetParent()->AddNode(clone);
						scene->SetSelection(clone);
						selection = scene->GetProxy();
                        SelectNodeAtTree(selection);
					}
					
					if (selection)
					{
						scene->SetBulletUpdate(selection, false);
						
						inTouch = true;	
						touchStart = event->point;
						
						startTransform = selection->GetLocalTransform();			
						
						InitMoving(event->point);
						
						translate1.CreateTranslation(-rotationCenter);
						translate2.CreateTranslation(rotationCenter);
						
						//calculate koefficient for moving
						Camera * cam = scene->GetCurrentCamera();
						const Vector3 & camPos = cam->GetPosition();
						const Matrix4 & wt = selection->GetWorldTransform();
						Vector3 objPos = Vector3(0,0,0) * wt;
						Vector3 dir = objPos - camPos;
						moveKf = (dir.Length() - cam->GetZNear()) * 0.003;					
					}
				}
			}
			else 
			{
				if (selection && modificationPanel->IsModificationMode())
				{
					PrepareModMatrix(event->point);
					selection->SetLocalTransform(currTransform);
                    if(currentGraph)
                    {
                        currentGraph->UpdatePropertiesForCurrentNode();
                    }
				}
			}
		}
		else if (event->phase == UIEvent::PHASE_ENDED)
		{
			inTouch = false;
			if (isDrag)
			{
                if(modificationPanel->IsLandscapeRelative())
                {
                    PlaceOnLandscape();
                }
                
				if (selection)
					scene->SetBulletUpdate(selection, true);				
			}
			else
			{
				Vector3 from, dir;
				GetCursorVectors(&from, &dir, event->point);
				Vector3 to = from + dir * 1000.0f;
				scene->TrySelection(from, to);				
				selection = scene->GetProxy();
				SelectNodeAtTree(selection);
			}
		}
	}
	else
	{
		cameraController->SetSelection(selection);
        
        if (event->phase == UIEvent::PHASE_KEYCHAR)
        {
            UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
            if(!tf)
            {
                cameraController->Input(event);
            }
        }
        else
        {
            cameraController->Input(event);
        }
        
	}
	UIControl::Input(event);
}

void EditorBodyControl::InitMoving(const Vector2 & point)
{
	//init planeNormal
	switch (modificationPanel->GetModAxis()) 
	{
		case ModificationsPanel::AXIS_X:
		case ModificationsPanel::AXIS_Y:
		case ModificationsPanel::AXIS_XY:
			planeNormal = Vector3(0,0,1);
			break;
		case ModificationsPanel::AXIS_Z:
		case ModificationsPanel::AXIS_YZ:
			planeNormal = Vector3(1,0,0);
			break;
		case ModificationsPanel::AXIS_XZ:
			planeNormal = Vector3(0,1,0);
			break;
		default:
			break;
	}

	Vector3 from, dir;
	GetCursorVectors(&from, &dir, point);

//	bool result = 
    GetIntersectionVectorWithPlane(from, dir, planeNormal, rotationCenter, startDragPoint);
}	

void EditorBodyControl::GetCursorVectors(Vector3 * from, Vector3 * dir, const Vector2 &point)
{
	Camera * cam = scene->GetCurrentCamera();
	if (cam)
	{
		const Rect & rect = scene3dView->GetLastViewportRect();
		*from = cam->GetPosition();
		Vector3 to = cam->UnProject(point.x, point.y, 0, rect);
		to -= *from;
		*dir = to;
	}
}

static Vector3 vect[3] = {Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};

void EditorBodyControl::PrepareModMatrix(const Vector2 & point)
{
	float32 winx = point.x - touchStart.x;
	float32 winy = point.y - touchStart.y;
	
	Matrix4 modification;
	modification.Identity();
	
	if (modificationPanel->GetModState() == ModificationsPanel::MOD_MOVE)
	{
		Vector3 from, dir;
		GetCursorVectors(&from, &dir, point);
		
		Vector3 currPoint;
		bool result = GetIntersectionVectorWithPlane(from, dir, planeNormal, rotationCenter, currPoint);
		
		if (result)
		{
			switch (modificationPanel->GetModAxis()) 
			{
				case ModificationsPanel::AXIS_X:
					currPoint.y = startDragPoint.y;
					currPoint.z = startDragPoint.z;
					break;
				case ModificationsPanel::AXIS_Y:
					currPoint.x = startDragPoint.x;
					currPoint.z = startDragPoint.z;
					break;
				case ModificationsPanel::AXIS_Z:
					currPoint.x = startDragPoint.x;
					currPoint.y = startDragPoint.y;
					break;
                    
                default:
                    break;
			}
			modification.CreateTranslation(currPoint - startDragPoint);
		}
	}
	else if (modificationPanel->GetModState() == ModificationsPanel::MOD_ROTATE)
	{
		Matrix4 d;
		switch (modificationPanel->GetModAxis()) 
		{
			case ModificationsPanel::AXIS_X:
			case ModificationsPanel::AXIS_Y:
				modification.CreateRotation(vect[modificationPanel->GetModAxis()], winy / 100.0f);
				break;
			case ModificationsPanel::AXIS_Z:
				modification.CreateRotation(vect[modificationPanel->GetModAxis()], winx / 100.0f);
				break;
			case ModificationsPanel::AXIS_XY:
				modification.CreateRotation(vect[ModificationsPanel::AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[ModificationsPanel::AXIS_Y], winy / 100.0f);
				modification *= d;
				break;
			case ModificationsPanel::AXIS_YZ:
				modification.CreateRotation(vect[ModificationsPanel::AXIS_Y], winx / 100.0f);
				d.CreateRotation(vect[ModificationsPanel::AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			case ModificationsPanel::AXIS_XZ:
				modification.CreateRotation(vect[ModificationsPanel::AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[ModificationsPanel::AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			default:
				break;
		}
		modification = (translate1 * modification) * translate2;
		
	}
	else if (modificationPanel->GetModState() == ModificationsPanel::MOD_SCALE)
	{
//		modification.CreateScale(Vector3(1,1,1) + vect[modAxis] * dist/100);
		float kf = winx / 100.0f;
		if (kf < -1.0)
			kf = - kf - 2.0;
		modification.CreateScale(Vector3(1,1,1) + Vector3(1,1,1) * kf);
		modification = (translate1 * modification) * translate2;
	}
	currTransform = startTransform * modification;
}


void EditorBodyControl::DrawAfterChilds(const UIGeometricData &geometricData)
{
	UIControl::DrawAfterChilds(geometricData);
	SceneNode * selection = scene->GetProxy();
	if (selection && modificationPanel->IsModificationMode())
	{
		const Rect & rect = scene3dView->GetLastViewportRect();
		Camera * cam = scene->GetCurrentCamera(); 
		Vector2 start = cam->GetOnScreenPosition(rotationCenter, rect);
		Vector2 end;
	
		const Vector3 & vc = cam->GetPosition();
		float32 kf = ((vc - rotationCenter).Length() - cam->GetZNear()) * 0.2f;
		
		for(int32 i = 0; i < 3; ++i)
		{
			if (modificationPanel->GetModAxis() == i
				|| (i == ModificationsPanel::AXIS_X && (modificationPanel->GetModAxis() == ModificationsPanel::AXIS_XY || modificationPanel->GetModAxis() == ModificationsPanel::AXIS_XZ)) 
				|| (i == ModificationsPanel::AXIS_Y && (modificationPanel->GetModAxis() == ModificationsPanel::AXIS_XY || modificationPanel->GetModAxis() == ModificationsPanel::AXIS_YZ)) 
				|| (i == ModificationsPanel::AXIS_Z && (modificationPanel->GetModAxis() == ModificationsPanel::AXIS_XZ || modificationPanel->GetModAxis() == ModificationsPanel::AXIS_YZ)))
			{
				RenderManager::Instance()->SetColor(0, 1.0f, 0, 1.0f);					
			}
			else 
			{
				RenderManager::Instance()->SetColor(1.0f, 0, 0, 1.0f);	
			}

			Vector3 v = rotationCenter + vect[i] * kf;
			end = cam->GetOnScreenPosition(v, rect);
			RenderHelper::Instance()->DrawLine(start, end);

		
			if (i == ModificationsPanel::AXIS_X 
				|| (i == ModificationsPanel::AXIS_Y && modificationPanel->GetModAxis() == ModificationsPanel::AXIS_Y)
				|| (i == ModificationsPanel::AXIS_Y && modificationPanel->GetModAxis() == ModificationsPanel::AXIS_YZ)
				)
			{
				axisSign[i] = (start.x > end.x) ? -1.0f: 1.0f;
			}
			else if (i == ModificationsPanel::AXIS_Y && modificationPanel->GetModAxis() == ModificationsPanel::AXIS_XY)
			{
				axisSign[i] = (start.y > end.y) ? -1.0f: 1.0f;				
			}
			else if (i == ModificationsPanel::AXIS_Z)
			{
				axisSign[i] = (start.y > end.y) ? -1.0f: 1.0f;
			}
		}
		RenderManager::Instance()->ResetColor();
	}
}

void EditorBodyControl::Update(float32 timeElapsed)
{
	SceneNode * selection = scene->GetProxy();
	if (selection)
	{
		rotationCenter = selection->GetWorldTransform().GetTranslationVector();
	}
	
    if(cameraController)
    {
        cameraController->Update(timeElapsed);
    }
    
    if(currentLandscapeEditor)
    {
        currentLandscapeEditor->Update(timeElapsed);
    }
    
    
    UIControl::Update(timeElapsed);

	BeastProxy::Instance()->Update(beastManager);
	if(BeastProxy::Instance()->IsJobDone(beastManager))
	{
		PackLightmaps();
		BeastProxy::Instance()->SafeDeleteManager(&beastManager);
	}
}

void EditorBodyControl::ReloadRootScene(const String &pathToFile)
{
    scene->ReleaseRootNode(pathToFile);
    
    ReloadNode(scene, pathToFile);
    
    scene->SetSelection(0);
    for (int32 i = 0; i < (int32)nodesToAdd.size(); i++) 
    {
        scene->ReleaseUserData(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->RemoveNode(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->AddNode(nodesToAdd[i].nodeToAdd);
        SafeRelease(nodesToAdd[i].nodeToAdd);
    }
    nodesToAdd.clear();

	modificationPanel->OnReloadScene();
    Refresh();
}

void EditorBodyControl::ReloadNode(SceneNode *node, const String &pathToFile)
{//если в рут ноды сложить такие же рут ноды то на релоаде все накроет пиздой
    KeyedArchive *customProperties = node->GetCustomProperties();
    if (customProperties->GetString("editor.referenceToOwner", "") == pathToFile) 
    {
        SceneNode *newNode = scene->GetRootNode(pathToFile)->Clone();
        newNode->SetLocalTransform(node->GetLocalTransform());
        newNode->GetCustomProperties()->SetString("editor.referenceToOwner", pathToFile);
        newNode->SetSolid(true);
        
        SceneNode *parent = node->GetParent();
        AddedNode addN;
        addN.nodeToAdd = newNode;
        addN.nodeToRemove = node;
        addN.parent = parent;

        nodesToAdd.push_back(addN);
        return;
    }
    
    int32 csz = node->GetChildrenCount();
    for (int ci = 0; ci < csz; ++ci)
    {
        SceneNode * child = node->GetChild(ci);
        ReloadNode(child, pathToFile);
    }
}


void EditorBodyControl::WillAppear()
{
    cameraController->SetSpeed(EditorSettings::Instance()->GetCameraSpeed());
    
    sceneGraph->SelectNode(NULL);
#if !defined (DAVA_QT)
    dataGraph->SelectNode(NULL);
	entitiesGraph->SelectNode(NULL);
#endif //#if !defied (DAVA_QT)
}


void EditorBodyControl::RefreshProperties()
{
    if(currentGraph)
    {
        currentGraph->RefreshProperties();
    }
}


void EditorBodyControl::BeastProcessScene()
{
	BeastProxy::Instance()->SafeDeleteManager(&beastManager);
	beastManager = BeastProxy::Instance()->CreateManager();

	//String path = GetFilePath();
	//if(String::npos == path.find("DataSource"))
	//{
	//	return;
	//}

	String path = "lightmaps_temp/";
	FileSystem::Instance()->CreateDirectory(path, false);

	BeastProxy::Instance()->SetLightmapsDirectory(beastManager, path);
	BeastProxy::Instance()->Run(beastManager, scene);
}

EditorScene * EditorBodyControl::GetScene()
{
    return scene;
}

void EditorBodyControl::AddNode(SceneNode *node)
{
#if defined (DAVA_QT)
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->AddSceneNode(node);
#else //#if defined (DAVA_QT)
    scene->AddNode(node);
    Refresh();
#endif // #if defined (DAVA_QT)

}

SceneNode * EditorBodyControl::GetSelectedSGNode()
{
    return scene->GetSelection();
}

void EditorBodyControl::RemoveSelectedSGNode()
{
#if defined (DAVA_QT)
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->RemoveSceneNode(GetSelectedSGNode());
#else //#if defined (DAVA_QT)
    sceneGraph->RemoveWorkingNode();
#endif // #if defined (DAVA_QT)
}




void EditorBodyControl::Refresh()
{
    sceneGraph->RefreshGraph();
#if !defined (DAVA_QT)
    dataGraph->RefreshGraph();
#endif //#if !defined(DAVA_QT)
}


void EditorBodyControl::SelectNodeAtTree(DAVA::SceneNode *node)
{
#if defined(DAVA_QT)
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    sceneData->SelectNode(node);
#else //#if defined(DAVA_QT)
    if(sceneGraph)
        sceneGraph->SelectNode(node);
    
    if(dataGraph)
        dataGraph->RefreshGraph();
#endif //#if defined(DAVA_QT)
    
}

#if !defined (DAVA_QT)
void EditorBodyControl::ResetSelection()
{
    scene->SetSelection(0);
    SelectNodeAtTree(0);
}
#endif //#if defined (DAVA_QT)

void EditorBodyControl::SetViewportSize(ResourceEditor::eViewportType viewportType)
{
    if(currentViewportType == viewportType)
        return;
    
    currentViewportType = viewportType;

    Rect fullRect = GetRect();
    Rect viewRect = Rect(SCENE_OFFSET, SCENE_OFFSET, fullRect.dx - 2 * SCENE_OFFSET, fullRect.dy - 2 * SCENE_OFFSET);
    
#if defined (DAVA_QT)
    viewRect.dx -= EditorSettings::Instance()->GetRightPanelWidth();

#else //#if defined (DAVA_QT)
    if(currentGraph->GetGraphPanel()->GetParent())
    {
        ToggleGraph(currentGraph);
    }
    ShowProperties(false);
    
    if(outputPanel)
    {
        viewRect.dy -= outputPanel->GetRect().dy;
    }
    
#endif //#if defined (DAVA_QT)
    
    Rect newRect = viewRect;
    switch (viewportType)
    {
        case ResourceEditor::VIEWPORT_IPHONE:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 480, 320);
            break;

        case ResourceEditor::VIEWPORT_RETINA:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 960, 640);
            break;

        case ResourceEditor::VIEWPORT_IPAD:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 1024, 768);
            break;

        default:
            break;
    }
    
    if((newRect.dx <= viewRect.dx) && (newRect.dy <= viewRect.dy))
    {
        viewRect = newRect;
    }
    else
    {
        currentViewportType = ResourceEditor::VIEWPORT_DEFAULT;
    }
    
    scene3dView->SetRect(viewRect);
}

bool EditorBodyControl::ControlsAreLocked()
{
    return (ResourceEditor::VIEWPORT_DEFAULT != currentViewportType);
}

void EditorBodyControl::ToggleSceneInfo()
{
    if(sceneInfoControl->GetParent())
    {
        RemoveControl(sceneInfoControl);
    }
    else
    {
        AddControl(sceneInfoControl);
    }
}


void EditorBodyControl::PackLightmaps()
{
	LightmapsPacker packer;
	packer.SetInputDir("lightmaps_temp/");
#if defined (DAVA_QT)	
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
	packer.SetOutputDir(sceneData->GetScenePathname() + "_lightmaps/");
#else //#if defined (DAVA_QT)
	packer.SetOutputDir(GetFilePath()+"_lightmaps/");
#endif //#if defined (DAVA_QT)	
	packer.Pack();
	packer.Compress();
	packer.ParseSpriteDescriptors();

	BeastProxy::Instance()->UpdateAtlas(beastManager, packer.GetAtlasingData());
}

void EditorBodyControl::Draw(const UIGeometricData &geometricData)
{
    if(LandscapeEditorActive())
    {
        currentLandscapeEditor->Draw(geometricData);
    }
    
    UIControl::Draw(geometricData);
}

void EditorBodyControl::RecreteFullTilingTexture()
{
    Vector<LandscapeNode *>landscapes;
    scene->GetChildNodes(landscapes);
    
    for(int32 i = 0; i < (int32)landscapes.size(); ++i)
    {
        landscapes[i]->UpdateFullTiledTexture();
    }
}



#pragma mark --Landscape Editor
void EditorBodyControl::CreateLandscapeEditor()
{
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    Rect toolsRect(leftSideWidth, 0, GetRect().dx - (leftSideWidth + rightSideWidth), ControlsFactory::TOOLS_HEIGHT);
    landscapeEditorColor = new LandscapeEditorColor(this, this, toolsRect);
    
    toolsRect.dy += ControlsFactory::TOOLS_HEIGHT;
    landscapeEditorHeightmap = new LandscapeEditorHeightmap(this, this, toolsRect);

    
    Rect rect = GetRect();
    landscapeToolsSelection = new LandscapeToolsSelection(NULL, 
                                                          Rect(leftSideWidth, rect.dy - ControlsFactory::OUTPUT_PANEL_HEIGHT, 
                                                               rect.dx - leftSideWidth - rightSideWidth, 
                                                               ControlsFactory::OUTPUT_PANEL_HEIGHT));
    landscapeToolsSelection->SetBodyControl(this);
    
    currentLandscapeEditor = NULL;
}

void EditorBodyControl::ReleaseLandscapeEditor()
{
    currentLandscapeEditor = NULL;
    SafeRelease(landscapeEditorColor);
    SafeRelease(landscapeEditorHeightmap);
    SafeRelease(landscapeToolsSelection);
}

bool EditorBodyControl::ToggleLandscapeEditor(int32 landscapeEditorMode)
{
    LandscapeEditorBase *requestedEditor = NULL;
    if(SceneEditorScreenMain::ELEMID_COLOR_MAP == landscapeEditorMode)
    {
        requestedEditor = landscapeEditorColor;
    }
    else if(SceneEditorScreenMain::ELEMID_HEIGHTMAP == landscapeEditorMode)
    {
        requestedEditor = landscapeEditorHeightmap;
    }
    
    if(currentLandscapeEditor && (currentLandscapeEditor != requestedEditor))
        return false;
    
    if(currentLandscapeEditor == requestedEditor)
    {
        currentLandscapeEditor->Toggle();
    }
    else
    {
        currentLandscapeEditor = requestedEditor;

        bool ret = currentLandscapeEditor->SetScene(scene);
        if(ret)
        {
            currentLandscapeEditor->GetToolPanel()->SetSelectionPanel(landscapeToolsSelection);
            currentLandscapeEditor->Toggle();
        }
        else
        {
            currentLandscapeEditor = NULL;
            return false;
        }
    }
    return true;
}

#pragma mark --LandscapeEditorDelegate
void EditorBodyControl::LandscapeEditorStarted()
{
    RemoveControl(sceneInfoControl);

    RemoveControl(modificationPanel);
    savedModificatioMode = modificationPanel->IsModificationMode();
    
    UIControl *toolsPanel = currentLandscapeEditor->GetToolPanel();
    if(!toolsPanel->GetParent())
    {
        AddControl(toolsPanel);
    }
    
    LandscapeNode *landscape = currentLandscapeEditor->GetLandscape();
    scene->SetSelection(landscape);
    SelectNodeAtTree(landscape);
    
    landscapeToolsSelection->Show();
}

void EditorBodyControl::LandscapeEditorFinished()
{
    landscapeToolsSelection->Close();

    UIControl *toolsPanel = currentLandscapeEditor->GetToolPanel();
    RemoveControl(toolsPanel);

    modificationPanel->IsModificationMode(savedModificatioMode);
    AddControl(modificationPanel);
    
    scene->SetSelection(NULL);
    SelectNodeAtTree(NULL);
    
    currentLandscapeEditor = NULL;
}


void EditorBodyControl::OnPlaceOnLandscape()
{
    PlaceOnLandscape();
}

bool EditorBodyControl::LandscapeEditorActive()
{
    return (currentLandscapeEditor && currentLandscapeEditor->IsActive());
}

NodesPropertyControl *EditorBodyControl::GetPropertyControl(const Rect &rect)
{
    return currentLandscapeEditor->GetPropertyControl(rect);
}

#if defined (DAVA_QT)
void EditorBodyControl::SetScene(EditorScene *newScene)
{
    SafeRelease(scene);
    scene = SafeRetain(newScene);
    
    scene3dView->SetScene(scene);
}

void EditorBodyControl::SetCameraController(CameraController *newCameraController)
{
    SafeRelease(cameraController);
    cameraController = SafeRetain(newCameraController);
}

void EditorBodyControl::SelectNodeQt(DAVA::SceneNode *node)
{
    sceneGraph->SelectNode(node);
}

void EditorBodyControl::OnReloadRootNodesQt()
{
    modificationPanel->OnReloadScene();
}
#endif //#if defined (DAVA_QT)

