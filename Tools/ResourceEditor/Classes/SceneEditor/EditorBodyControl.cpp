#include "EditorBodyControl.h"
#include "ControlsFactory.h"
#include "../BeastProxy.h"
#include "../SceneNodeUserData.h"
#include "PropertyControlCreator.h"
#include "EditorSettings.h"
#include "../config.h"

#include "SceneInfoControl.h"
#include "SceneValidator.h"
#include "../LightmapsPacker.h"

#include "LandscapeEditorColor.h"
#include "LandscapeEditorHeightmap.h"
#include "SceneEditorScreenMain.h"
#include "LandscapeToolsSelection.h"
#include "LandscapeEditorCustomColors.h"
#include "LandscapeEditorVisibilityCheckTool.h"


#include "SceneGraph.h"
#include "DataGraph.h"
#include "EntitiesGraph.h"

#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Main/QtUtils.h"
#include "../RulerTool/RulerTool.h"

#include "../SceneEditor/EditorConfig.h"

EditorBodyControl::EditorBodyControl(const Rect & rect)
    :   UIControl(rect)
	, beastManager(0)
{
    currentViewportType = ResourceEditor::VIEWPORT_DEFAULT;
    
    scene = NULL;
    
    landscapeRulerTool = NULL;
	
    ControlsFactory::CusomizeBottomLevelControl(this);

    sceneGraph = new SceneGraph(this, rect);
    currentGraph = sceneGraph;

    InitControls();

    scene3dView->SetDebugDraw(true);
    scene3dView->SetInputEnabled(false);
    AddControl(scene3dView);

    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    sceneInfoControl = new SceneInfoControl(Rect(rect.dx - rightSideWidth * 2 , 0, rightSideWidth, rightSideWidth));
    AddControl(sceneInfoControl);

	CreateModificationPanel();
    CreateLandscapeEditor();

    scene = NULL;
    cameraController = NULL;

	mainCam = 0;
	debugCam = 0;
    
    propertyShowState = EPSS_ONSCREEN;

    AddControl(currentGraph->GetPropertyPanel());
}

void EditorBodyControl::InitControls()
{
    Rect rect = GetRect();

    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    scene3dView = new UI3DView(Rect(SCENE_OFFSET, SCENE_OFFSET,
                                    rect.dx - 2 * SCENE_OFFSET - rightSideWidth,
                                    rect.dy - 2 * SCENE_OFFSET));
}

EditorBodyControl::~EditorBodyControl()
{
    SafeRelease(landscapeRulerTool);
    
    SafeRelease(sceneGraph);
    currentGraph = NULL;
    
    SafeRelease(sceneInfoControl);
    
    ReleaseModificationPanel();
    
    
    ReleaseLandscapeEditor();
    
    SafeRelease(scene);
    SafeRelease(cameraController);
  
    SafeRelease(scene3dView);
}

void EditorBodyControl::UpdateModificationPanel(void)
{
	modificationPanel->UpdateCollisionTypes();
}

void EditorBodyControl::SetBrushRadius(uint32 newSize)
{
	if(RulerToolIsActive())
        return;
    
	if(!currentLandscapeEditor || (currentLandscapeEditor != landscapeEditorCustomColors))
	{
		return;
	}

	landscapeEditorCustomColors->SetRadius(newSize);
}

void EditorBodyControl::SetColorIndex(uint32 indexInSet)
{
	if(RulerToolIsActive())
        return;
    
	if(!currentLandscapeEditor || (currentLandscapeEditor != landscapeEditorCustomColors))
	{
		return;
	}

	const Vector<Color> &colorVector = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	if(colorVector.size() == 0 || colorVector.size() <= indexInSet)
	{
		return;
	}
	landscapeEditorCustomColors->SetColor(colorVector[indexInSet]);
}

void EditorBodyControl::SaveTexture(const String &path)
{
	if(RulerToolIsActive())
        return;
    
	if(!currentLandscapeEditor)
	{
		return;
	}
	
	if(currentLandscapeEditor == landscapeEditorCustomColors)
		landscapeEditorCustomColors->SaveColorLayer(path);
	else if(currentLandscapeEditor == landscapeEditorVisibilityTool)
		landscapeEditorVisibilityTool->SaveColorLayer(path);
}

void EditorBodyControl::CustomColorsLoadTexture(const String &path)
{
	if(RulerToolIsActive())
		return;

	if(!currentLandscapeEditor || currentLandscapeEditor != landscapeEditorCustomColors)
		return;

	landscapeEditorCustomColors->LoadColorLayer(path);
}

String EditorBodyControl::CustomColorsGetCurrentSaveFileName()
{
	if(RulerToolIsActive())
		return "";

	if(!currentLandscapeEditor || currentLandscapeEditor != landscapeEditorCustomColors)
		return "";

	return landscapeEditorCustomColors->GetCurrentSaveFileName();
}

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
        PlaceOnLandscape(selection);
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
    bool inputDone = LandscapeEditorInput(event);
    if(!inputDone)
    {
        inputDone = RulerToolInput(event);
    }
    
    if(!inputDone)
    {
        ProcessKeyboard(event);
        ProcessMouse(event);
    }
    
    UIControl::Input(event);
}

bool EditorBodyControl::LandscapeEditorInput(UIEvent *event)
{
    if(LandscapeEditorActive())
    {
        bool processed = currentLandscapeEditor->Input(event);
        if(!processed)
        {
            cameraController->Input(event);
        }
        return true;
    }
    
    return false;
}

bool EditorBodyControl::RulerToolInput(UIEvent *event)
{
    if(RulerToolIsActive())
    {
        bool processed = landscapeRulerTool->Input(event);
        if(!processed)
        {
            cameraController->Input(event);
        }
        return true;
    }

    return false;
}

bool EditorBodyControl::ProcessKeyboard(UIEvent *event)
{
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
        if(!tf)
        {
            modificationPanel->Input(event);
            
			if(!IsKeyModificatorsPressed())
			{
				Camera * newCamera = 0;
				switch(event->tid)
				{
				case DVKEY_ESCAPE:
					{
						UIControl *c = UIControlSystem::Instance()->GetFocusedControl();
						if(c == this || c == scene3dView)
						{
                        SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
							activeScene->SelectNode(NULL);
						}

						break;
					}

				case DVKEY_BACKSPACE:
					{
						bool cmdIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL);
						if(cmdIsPressed)
						{
							sceneGraph->RemoveWorkingNode();

                        SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
							activeScene->SelectNode(NULL);
							activeScene->RebuildSceneGraph();
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

				case DVKEY_X:
					{
						bool Z = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_Z);
						bool C = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_C);
						if(!Z && !C)
						{
							ProcessIsSolidChanging();
						}

						break;
					}

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
	}
	
    return true;
}

bool EditorBodyControl::ProcessMouse(UIEvent *event)
{
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
                    LandscapeNode *landscape = dynamic_cast<LandscapeNode *>(selection);
                    if(!landscape)
                    {
                        PrepareModMatrix(event->point);
                        selection->SetLocalTransform(currTransform);
                        if(currentGraph)
                        {
                            currentGraph->UpdateMatricesForCurrentNode();
                        }
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
    return true;
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

	String path = EditorSettings::Instance()->GetProjectPath()+"DataSource/lightmaps_temp/";
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
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->AddSceneNode(node);
}

SceneNode * EditorBodyControl::GetSelectedSGNode()
{
    return scene->GetSelection();
}

void EditorBodyControl::RemoveSelectedSGNode()
{
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->RemoveSceneNode(GetSelectedSGNode());
}


void EditorBodyControl::Refresh()
{
    sceneGraph->RefreshGraph();
}


void EditorBodyControl::SelectNodeAtTree(DAVA::SceneNode *node)
{
    SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
    sceneData->SelectNode(node);
}

void EditorBodyControl::SetViewportSize(ResourceEditor::eViewportType viewportType)
{
    if(currentViewportType == viewportType)
        return;
    
    currentViewportType = viewportType;

    Rect fullRect = GetRect();
    Rect viewRect = Rect(SCENE_OFFSET, SCENE_OFFSET, fullRect.dx - 2 * SCENE_OFFSET, fullRect.dy - 2 * SCENE_OFFSET);
    
    viewRect.dx -= EditorSettings::Instance()->GetRightPanelWidth();
    
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
	SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
	String inputDir = EditorSettings::Instance()->GetProjectPath()+"DataSource/lightmaps_temp/";
	String outputDir = sceneData->GetScenePathname() + "_lightmaps/";
	FileSystem::Instance()->MoveFile(inputDir+"landscape.png", "test_landscape.png"); 

	LightmapsPacker packer;
	packer.SetInputDir(inputDir);

	packer.SetOutputDir(outputDir);
	packer.Pack();
	packer.Compress();
	packer.ParseSpriteDescriptors();

	BeastProxy::Instance()->UpdateAtlas(beastManager, packer.GetAtlasingData());

	FileSystem::Instance()->MoveFile("test_landscape.png", outputDir+"landscape.png");
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

void EditorBodyControl::CreateLandscapeEditor()
{
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    Rect toolsRect(leftSideWidth, 0, GetRect().dx - (leftSideWidth + rightSideWidth), ControlsFactory::TOOLS_HEIGHT);
    landscapeEditorColor = new LandscapeEditorColor(this, this, toolsRect);
    
    toolsRect.dy += ControlsFactory::TOOLS_HEIGHT;
    landscapeEditorHeightmap = new LandscapeEditorHeightmap(this, this, toolsRect);

    landscapeEditorCustomColors = new LandscapeEditorCustomColors(this, this, toolsRect);
	landscapeEditorVisibilityTool = new LandscapeEditorVisibilityCheckTool(this, this, toolsRect);
    
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
	SafeRelease(landscapeEditorCustomColors);
	SafeRelease(landscapeEditorVisibilityTool);
}

bool EditorBodyControl::ToggleLandscapeEditor(int32 landscapeEditorMode)
{
    if(RulerToolIsActive())
        return false;
    
    LandscapeEditorBase *requestedEditor = NULL;
    if(SceneEditorScreenMain::ELEMID_COLOR_MAP == landscapeEditorMode)
    {
        requestedEditor = landscapeEditorColor;
    }
    else if(SceneEditorScreenMain::ELEMID_HEIGHTMAP == landscapeEditorMode)
    {
        requestedEditor = landscapeEditorHeightmap;
    } else if(SceneEditorScreenMain::ELEMID_CUSTOM_COLORS == landscapeEditorMode)
    {
        requestedEditor = landscapeEditorCustomColors;
    } else if(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL == landscapeEditorMode)
	{
		requestedEditor = landscapeEditorVisibilityTool;
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
	SelectNodeAtTree(NULL);
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

bool EditorBodyControl::TileMaskEditorEnabled()
{
    return LandscapeEditorActive() && (currentLandscapeEditor == landscapeEditorColor);
}

NodesPropertyControl *EditorBodyControl::GetPropertyControl(const Rect &rect)
{
    return currentLandscapeEditor->GetPropertyControl(rect);
}

void EditorBodyControl::SetScene(EditorScene *newScene)
{
    SafeRelease(scene);
    scene = SafeRetain(newScene);
    
    scene3dView->SetScene(scene);
	sceneGraph->SetScene(scene);
    
    if(sceneInfoControl)
    {
        sceneInfoControl->SetWorkingScene(newScene);
    }
    
    modificationPanel->SetScene(scene);
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

void EditorBodyControl::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);

    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    scene3dView->SetSize(newSize - Vector2(2 * SCENE_OFFSET + rightSideWidth, 2 * SCENE_OFFSET));
    
    sceneGraph->SetSize(newSize);
    
    sceneInfoControl->SetPosition(Vector2(newSize.x - rightSideWidth * 2, 0));
}



void EditorBodyControl::ProcessIsSolidChanging()
{
    SceneNode *selectedNode = scene->GetSelection();
    if(selectedNode)
    {
        KeyedArchive *customProperties = selectedNode->GetCustomProperties();
        if(customProperties && customProperties->IsKeyExists(String("editor.isSolid")))
        {
            bool isSolid = selectedNode->GetSolid();
            selectedNode->SetSolid(!isSolid);
            
            SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
            activeScene->RebuildSceneGraph();
            
            KeyedArchive *properties = selectedNode->GetCustomProperties();
            if(properties && properties->IsKeyExists(String("editor.referenceToOwner")))
            {
                String filePathname = properties->GetString(String("editor.referenceToOwner"));
                activeScene->OpenLibraryForFile(filePathname);
            }
            
            sceneGraph->SelectNode(selectedNode);
        }
    }
}


bool EditorBodyControl::RulerToolTriggered()
{
    if(landscapeRulerTool)
    {
        landscapeRulerTool->DisableTool();
        SafeRelease(landscapeRulerTool);
    }
    else
    {
        if(!LandscapeEditorActive())
        {
            landscapeRulerTool = new RulerTool(this);
            bool enabled = landscapeRulerTool->EnableTool(scene);
            if(!enabled)
            {
                SafeRelease(landscapeRulerTool);
                return false;
            }
        }
    }
    
    return true;
}

bool EditorBodyControl::RulerToolIsActive()
{
    return (NULL != landscapeRulerTool);
}

void EditorBodyControl::VisibilityToolSetPoint()
{
	landscapeEditorVisibilityTool->SetState(VCT_STATE_SET_POINT);
}

void EditorBodyControl::VisibilityToolSetArea()
{
	landscapeEditorVisibilityTool->SetState(VCT_STATE_SET_AREA);
}

void EditorBodyControl::VisibilityToolSetAreaSize(uint32 size)
{
	landscapeEditorVisibilityTool->SetVisibilityAreaSize(size);
}
