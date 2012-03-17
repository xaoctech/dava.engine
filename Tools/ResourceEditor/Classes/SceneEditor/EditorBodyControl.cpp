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

EditorBodyControl::EditorBodyControl(const Rect & rect)
    :   UIControl(rect)
	, beastManager(0)
{
    currentViewPortID = EVPID_DEFAULT;
    
    scene = NULL;
	
    isModeModification = false;
    selectedSceneGraphNode = NULL;
    selectedDataGraphNode = NULL;
	nodesPropertyPanel = 0;
	helpDialog = 0;
	btnPlaceOn = 0;


    ControlsFactory::CusomizeBottomLevelControl(this);


    CreateLeftPanel();
    
    bool showOutput = EditorSettings::Instance()->GetShowOutput();
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    if(showOutput)
    {
        scene3dView = new UI3DView(Rect(leftSideWidth + SCENE_OFFSET, SCENE_OFFSET, 
                                        rect.dx - leftSideWidth - rightSideWidth - 2 * SCENE_OFFSET, 
                                        rect.dy - 2 * SCENE_OFFSET - ControlsFactory::OUTPUT_PANEL_HEIGHT));
    }
    else
    {
        scene3dView = new UI3DView(Rect(leftSideWidth + SCENE_OFFSET, SCENE_OFFSET, 
                                        rect.dx - leftSideWidth - rightSideWidth - 2 * SCENE_OFFSET, 
                                        rect.dy - 2 * SCENE_OFFSET));
    }

    scene3dView->SetDebugDraw(true);
    scene3dView->SetInputEnabled(false);
    AddControl(scene3dView);
    
    

    sceneInfoControl = new SceneInfoControl(Rect(rect.dx - rightSideWidth * 2 , 0, rightSideWidth, rightSideWidth));
    AddControl(sceneInfoControl);
    
    CreateScene(true);

    if(showOutput)
    {
        outputPanel = new OutputPanelControl(scene, Rect(leftSideWidth, rect.dy - ControlsFactory::OUTPUT_PANEL_HEIGHT, 
                                                         rect.dx - leftSideWidth - rightSideWidth, 
                                                         ControlsFactory::OUTPUT_PANEL_HEIGHT));
        ControlsFactory::CustomizePanelControl(outputPanel, false);
        AddControl(outputPanel);
    }
    else
    {
        outputPanel = NULL;
    }
    
    
    CreatePropertyPanel();
	
	CreateModificationPanel();
	
	CreateHelpPanel();
	mainCam = 0;
	debugCam = 0;
}


EditorBodyControl::~EditorBodyControl()
{
    SafeRelease(sceneInfoControl);
    
    ReleaseModificationPanel();
    
    ReleasePropertyPanel();

    SafeRelease(outputPanel);
    
    ReleaseScene();
  
    SafeRelease(scene3dView);

    ReleaseLeftPanel();
	
	SafeRelease(helpDialog);
}


#define V_OFFSET 30
#define H_OFFSET 10
void EditorBodyControl::AddHelpText(const wchar_t * txt, float32 & y)
{
	UIStaticText *text;
	text = new UIStaticText(Rect(H_OFFSET, 0, 500, y++ * V_OFFSET));
	text->SetFont(ControlsFactory::GetFontLight());
	text->SetText(txt);
	text->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
	helpDialog->AddControl(text);	
}

void EditorBodyControl::CreateHelpPanel()
{
	float32 y = 1;

	helpDialog = new DraggableDialog(Rect(100, 100, 510, 500));
	ControlsFactory::CustomizeDialog(helpDialog);

	AddHelpText(L"F1/H - this help", y);
	AddHelpText(L"A W S D - fly camera", y);
	AddHelpText(L"1, 2, 3, 4 - set camera speed", y);
	AddHelpText(L"T - set camera to Top position", y);
	AddHelpText(L"Left mouse button - selection", y);
	AddHelpText(L"Right mouse button - camera angle", y);
	AddHelpText(L"Z - zoom to selection", y);	
	AddHelpText(L"BackSpace - remove selected object", y);
	AddHelpText(L"Esc - drop selection", y);	
	AddHelpText(L"Left mouse button (in selection) - object modification", y);
	AddHelpText(L"Drag with left mouse button + SHIFT (create copy of object)", y);
	AddHelpText(L"Middle mouse button (in selection) - move in camera plain", y);
	AddHelpText(L"Alt + Middle mouse button (in selection) rotate about selected objects", y);
	AddHelpText(L"Q, E, R (in selection) - change active modification mode (move, translate, scale)", y);
	AddHelpText(L"5, 6, 7 (in selection) - change active axis", y);
	AddHelpText(L"8 (in selection) - enumerate pairs of axis", y);
	AddHelpText(L"P (in selection) - place node on landscape", y);
    AddHelpText(L"Alt + 1...8: set SetForceLodLayer(0, 1, ... , 7)", y);
    AddHelpText(L"Alt + 0: set SetForceLodLayer(-1)", y);

    AddHelpText(L"Landscape Editor:", ++y);
	AddHelpText(L"Press & hold \"Spacebar\" to scroll area", y);
    
    AddHelpText(L"Scene Graph:", ++y);
    AddHelpText(L"Left mouse with Command/Ctrl key - change parent of node", y);
    AddHelpText(L"Right mouse with Shift key - change order of node", y);

	AddHelpText(L"version "EDITOR_VERSION, ++y);
}



void EditorBodyControl::CreateLeftPanel()
{
    Rect fullRect = GetRect();
    
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    Rect leftRect = Rect(0, 0, leftSideWidth, fullRect.dy);
    leftPanelSceneGraph = ControlsFactory::CreatePanelControl(leftRect);
    AddControl(leftPanelSceneGraph);
    
    Rect sceneGraphRect = leftRect;
    sceneGraphRect.dy -= (ControlsFactory::BUTTON_HEIGHT * 5);
    sceneGraphTree = new UIHierarchy(sceneGraphRect);
    ControlsFactory::CusomizeListControl(sceneGraphTree);
    ControlsFactory::SetScrollbar(sceneGraphTree);
    sceneGraphTree->SetCellHeight(CELL_HEIGHT);
    sceneGraphTree->SetDelegate(this);
    sceneGraphTree->SetClipContents(true);
    leftPanelSceneGraph->AddControl(sceneGraphTree);
    
    int32 y = sceneGraphRect.dy;
    UIButton * refreshSceneGraphButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                                 LocalizedString(L"panel.refresh"));
    y += ControlsFactory::BUTTON_HEIGHT;
    
    UIButton * lookAtButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"scenegraph.lookatobject"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * removeNodeButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                                LocalizedString(L"scenegraph.removeobject"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * enableDebugFlagsButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                                      LocalizedString(L"scenegraph.debugflags"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * bakeMatrices = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                                      LocalizedString(L"scenegraph.bakemetrics"));

    
    
    refreshSceneGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRefreshSceneGraph));
    lookAtButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnLookAtButtonPressed));
    removeNodeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRemoveNodeButtonPressed));
    enableDebugFlagsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnEnableDebugFlagsPressed));
    bakeMatrices->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnBakeMatricesPressed));
    
    leftPanelSceneGraph->AddControl(refreshSceneGraphButton);
    leftPanelSceneGraph->AddControl(lookAtButton);
    leftPanelSceneGraph->AddControl(removeNodeButton);
    leftPanelSceneGraph->AddControl(enableDebugFlagsButton);
    leftPanelSceneGraph->AddControl(bakeMatrices);
    
    SafeRelease(refreshSceneGraphButton);
    SafeRelease(lookAtButton);
    SafeRelease(removeNodeButton);
    SafeRelease(enableDebugFlagsButton);
    SafeRelease(bakeMatrices);
    
    
    
    Rect dataGraphRect = leftRect;
    dataGraphRect.dy -= (ControlsFactory::BUTTON_HEIGHT);
    leftPanelDataGraph = ControlsFactory::CreatePanelControl(leftRect);
    dataGraphTree = new UIHierarchy(dataGraphRect);
    ControlsFactory::CusomizeListControl(dataGraphTree);
    ControlsFactory::SetScrollbar(dataGraphTree);
    dataGraphTree->SetCellHeight(CELL_HEIGHT);
    dataGraphTree->SetDelegate(this);
    dataGraphTree->SetClipContents(true);
    leftPanelDataGraph->AddControl(dataGraphTree);
    UIButton * refreshDataGraphButton = ControlsFactory::CreateButton(Rect(0, dataGraphRect.dy, 
                                                                           leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                                 LocalizedString(L"panel.refresh"));
    refreshDataGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRefreshDataGraph));
    leftPanelDataGraph->AddControl(refreshDataGraphButton);
    SafeRelease(refreshDataGraphButton);
}

void EditorBodyControl::ReleaseLeftPanel()
{
    SafeRelease(sceneGraphTree);
    SafeRelease(leftPanelSceneGraph);
    
    SafeRelease(dataGraphTree);
    SafeRelease(leftPanelDataGraph);
}


void EditorBodyControl::CreateScene(bool withCameras)
{
    scene = new EditorScene();
    // Camera setup
    cameraController = new WASDCameraController(EditorSettings::Instance()->GetCameraSpeed());
    
    if(withCameras)
    {
        Camera * cam = new Camera(scene);
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
        
        Camera * cam2 = new Camera(scene);
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

void EditorBodyControl::ReleaseScene()
{
    //TODO: need to release root nodes?
    
    SafeRelease(scene);
    SafeRelease(cameraController);
}


static const wchar_t * mods[3] = { L"M", L"R", L"S"};
static const wchar_t * axises[3] = { L"X", L"Y", L"Z"};

#define BUTTON_W 20 
#define BUTTON_B 5 

void EditorBodyControl::CreateModificationPanel(void)
{
	float offx = scene3dView->GetRect(true).x;
	modState = MOD_MOVE;
	modAxis = AXIS_X;
	
	modificationPanel = ControlsFactory::CreatePanelControl(Rect(offx, 5, 160, 45));
    modificationPanel->GetBackground()->SetColor(Color(1.0, 1.0, 1.0, 0.2));
	int i;
	
	for (i = 0; i < 3; i++)
	{
		btnMod[i] = ControlsFactory::CreateButton(Rect((BUTTON_W + BUTTON_B) * i, 0, BUTTON_W, BUTTON_W), mods[i]);
		btnMod[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModificationPressed));
		modificationPanel->AddControl(btnMod[i]);

		btnAxis[i] = ControlsFactory::CreateButton(Rect((BUTTON_W + BUTTON_B) * i, BUTTON_W + BUTTON_B, BUTTON_W, BUTTON_W), axises[i]);
		btnAxis[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModificationPressed));
		modificationPanel->AddControl(btnAxis[i]);
	}
	UIStaticText * st = new UIStaticText(Rect(55, 0, 70, BUTTON_W));
    st->SetFont(ControlsFactory::GetFontLight());
	st->SetText(L"w, e, r");
    modificationPanel->AddControl(st);

	st = new UIStaticText(Rect(55, BUTTON_W + BUTTON_B, 80, BUTTON_W));
    st->SetFont(ControlsFactory::GetFontLight());
	st->SetText(L"5, 6, 7, 8");
    modificationPanel->AddControl(st);

	
	btnPlaceOn = ControlsFactory::CreateButton(Rect(115, 0, BUTTON_W, BUTTON_W), L"P");
	btnPlaceOn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModificationPressed));
	modificationPanel->AddControl(btnPlaceOn);
	
	
	UpdateModState();
	
	btnPopUp = ControlsFactory::CreateButton(Rect(140, 0, BUTTON_W, BUTTON_W), L"#");
	btnPopUp->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModificationPopUpPressed));
	modificationPanel->AddControl(btnPopUp);
	
	modificationPopUp = new ModificationPopUp();
	
	btnModeSelection = ControlsFactory::CreateButton(Rect(offx + 170, 5, BUTTON_W, BUTTON_W), L"S");
	btnModeSelection->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModePressed));
	AddControl(btnModeSelection);

	btnModeModification = ControlsFactory::CreateButton(Rect(offx + 195, 5, BUTTON_W, BUTTON_W), L"M");
	btnModeModification->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModePressed));
	AddControl(btnModeModification);
	OnModePressed(btnModeSelection, 0, 0);
}

void EditorBodyControl::ReleaseModificationPanel()
{
	for (int i = 0; i < 3; i++)
	{
		SafeRelease(btnMod[i]);
		SafeRelease(btnAxis[i]);
	}
	SafeRelease(modificationPanel);
}


void EditorBodyControl::OnModePressed(BaseObject * object, void * userData, void * callerData)
{
	isModeModification = (object == btnModeModification);
	
	if (isModeModification)
	{
		btnModeModification->SetState(UIControl::STATE_SELECTED);
		btnModeSelection->SetState(UIControl::STATE_NORMAL);
	}
	else
	{
		btnModeModification->SetState(UIControl::STATE_NORMAL);
		btnModeSelection->SetState(UIControl::STATE_SELECTED);
	}
}

void EditorBodyControl::OnModificationPopUpPressed(BaseObject * object, void * userData, void * callerData)
{
	UIScreen * scr = UIScreenManager::Instance()->GetScreen();
	if (modificationPopUp->GetParent() == 0)
	{
		modificationPopUp->SetSelection(scene->GetProxy());
		scr->AddControl(modificationPopUp);
	}
	else
	{
		scr->RemoveControl(modificationPopUp);
		modificationPopUp->SetSelection(0);
	}
}

void EditorBodyControl::OnModificationPressed(BaseObject * object, void * userData, void * callerData)
{
	if (object == btnPlaceOn)
	{
		PlaceOnLandscape();
		return;
	}
		
	for (int i = 0; i < 3; i++)
	{
		if (object == btnMod[i])
		{
			modState = (eModState)i;
		}
		if (object == btnAxis[i])
		{
			modAxis = (eModAxis)i;
		}
	}
	UpdateModState();
}


void EditorBodyControl::UpdateModState(void)
{
	for (int i = 0; i < 3; i++)
	{
		btnMod[i]->SetState(UIControl::STATE_NORMAL);
		btnAxis[i]->SetState(UIControl::STATE_NORMAL);
	}
	btnMod[modState]->SetState(UIControl::STATE_SELECTED);

	switch (modAxis) 
	{
	case AXIS_X:
	case AXIS_Y:
	case AXIS_Z:
		btnAxis[modAxis]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XY:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_YZ:
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XZ:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	default:
		break;
	}
}


void EditorBodyControl::CreatePropertyPanel()
{
    Rect fullRect = GetRect();

    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    propertyPanelRect = Rect(fullRect.dx - rightSideWidth, 0, rightSideWidth, size.y);
    rightPanel = ControlsFactory::CreatePanelControl(propertyPanelRect);
    AddControl(rightPanel);

    refreshButton = ControlsFactory::CreateButton(Rect(
                                            0, propertyPanelRect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                            propertyPanelRect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                            LocalizedString(L"panel.refresh"));
    refreshButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRefreshPressed));
    
    rightPanel->AddControl(refreshButton);
    
    
    propertyPanelRect.x = propertyPanelRect.y = 0;
    propertyPanelRect.dy -= ControlsFactory::BUTTON_HEIGHT;

    //nodesPropertyPanel = new NodesPropertyControl(propertyPanelRect, false);
    //nodesPropertyPanel->SetDelegate(this);
}

void EditorBodyControl::ReleasePropertyPanel()
{
    SafeRelease(refreshButton);

    SafeRelease(nodesPropertyPanel);
    
    SafeRelease(rightPanel);
}

bool EditorBodyControl::IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode)
{
    if(forHierarchy == sceneGraphTree)
    {
        if (forNode) 
        {
            SceneNode *node = (SceneNode*)forNode;
            if(node->GetSolid())
            {
                return false;
            }
            else
            {
                return node->GetChildrenCount() > 0;
            }
        }
        
        return scene->GetChildrenCount() > 0;
    }
    else if(forHierarchy == dataGraphTree)
    {
        if (forNode) 
        {
            return ((DataNode*)forNode)->GetChildrenCount() > 0;
        }
        
        return dataNodes.size() > 0;
    }
    
    return false;
}

int32 EditorBodyControl::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
    if(forHierarchy == sceneGraphTree)
    {
        if (forParent) 
        {
            SceneNode *node = (SceneNode*)forParent;
            if(node->GetSolid())
            {
                return 0;
            }
            else
            {
                return node->GetChildrenCount();
            }

        }
        
        return scene->GetChildrenCount();
    }
    else if(forHierarchy == dataGraphTree)
    {
        if (forParent) 
        {
            return ((DataNode*)forParent)->GetChildrenCount();
        }
        
        return dataNodes.size();
    }

    return 0;
}

void * EditorBodyControl::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
    if(forHierarchy == sceneGraphTree)
    {
        if (forParent) 
        {
            return ((SceneNode*)forParent)->GetChild(index);
        }
        
        return scene->GetChild(index);
    }
    else if(forHierarchy == dataGraphTree)
    {
        if (forParent) 
        {
            return ((DataNode*)forParent)->GetChild(index);
        }
        
        
        Set<DataNode *>::const_iterator it = dataNodes.begin();
        Set<DataNode *>::const_iterator endIt = dataNodes.end();
        for(int32 i = 0; it != endIt; ++it, ++i)
        {
            if(i == index)
            {
                return (*it);
            }
        }
    }
    
    return NULL;
}

UIHierarchyCell * EditorBodyControl::CellForNode(UIHierarchy *forHierarchy, void *node)
{
    UIHierarchyCell *c = NULL;
    if(forHierarchy == sceneGraphTree)
    {
        c = forHierarchy->GetReusableCell("SceneGraph cell"); //try to get cell from the reusable cells store
        if(!c)
        { //if cell of requested type isn't find in the store create new cell
            int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
            c = new UIHierarchyCell(Rect(0, 0, leftSideWidth, CELL_HEIGHT), "SceneGraph cell");
        }
        
        //fill cell whith data
        SceneNode *n = (SceneNode *)node;
        c->text->SetText(StringToWString(n->GetName()));
        if(n == selectedSceneGraphNode)
        {
            c->SetSelected(true, false);
        }
        else
        {
            c->SetSelected(false, false);
        }
    }
    else if(forHierarchy == dataGraphTree)
    {
        c = forHierarchy->GetReusableCell("DataGraph cell"); //try to get cell from the reusable cells store
        if(!c)
        { //if cell of requested type isn't find in the store create new cell
            int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
            c = new UIHierarchyCell(Rect(0, 0, leftSideWidth, CELL_HEIGHT), "DataGraph cell");
        }
        
        //fill cell whith data
        DataNode *n = (DataNode *)node;
        c->text->SetText(StringToWString(n->GetName()));
        if(n == selectedDataGraphNode)
        {
            c->SetSelected(true, false);
        }
        else
        {
            c->SetSelected(false, false);
        }
    }
    
    ControlsFactory::CustomizeExpandButton(c->openButton);
    ControlsFactory::CustomizeSceneGraphCell(c);

    return c;//returns cell
}

void EditorBodyControl::OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell)
{
    if(forHierarchy == sceneGraphTree)
    {
        UIHierarchyNode * hNode = selectedCell->GetNode();
        SceneNode * node = dynamic_cast<SceneNode*>((BaseObject*)hNode->GetUserNode());
        if (node)
        {
            selectedSceneGraphNode = node;

            scene->SetSelection(0);
			scene->SetSelection(node);
            
            UpdatePropertyPanel();
            DebugInfo();
			
			Camera * cam = dynamic_cast<Camera*>(node);
			if (cam)
			{
				if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT))
				{
					scene->SetClipCamera(cam);
				}
				else 
				{
					scene->SetCurrentCamera(cam);
					cameraController->SetScene(scene);
				}
			}
        }
    }
    else if(forHierarchy == dataGraphTree)
    {
        UIHierarchyNode * hNode = selectedCell->GetNode();
        DataNode * node = dynamic_cast<DataNode*>((BaseObject*)hNode->GetUserNode());
        if (node)
        {
            selectedDataGraphNode = node;

            UpdatePropertyPanel();
            DebugInfo();
        }
    }
    
    //select 
    List<UIControl*> children = forHierarchy->GetVisibleCells();
    for(List<UIControl*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        UIControl *ctrl = (*it);
        ctrl->SetSelected(false, false);
    }
    
    selectedCell->SetSelected(true, false);
}

void EditorBodyControl::DebugInfo()
{
}

void EditorBodyControl::UpdatePropertyPanel()
{
    if(selectedSceneGraphNode && (NULL != leftPanelSceneGraph->GetParent()))
    {
		RecreatePropertiesPanelForNode(selectedSceneGraphNode);
        if(!nodesPropertyPanel->GetParent())
        {
            rightPanel->AddControl(nodesPropertyPanel);
        }
        RefreshProperties();
    }
    else if(selectedDataGraphNode && (NULL != leftPanelDataGraph->GetParent()))
    {
		RecreatePropertiesPanelForNode(selectedDataGraphNode);
        if(!nodesPropertyPanel->GetParent())
        {
            rightPanel->AddControl(nodesPropertyPanel);
        }
        RefreshProperties();
    }
    else
    {
        if(nodesPropertyPanel && nodesPropertyPanel->GetParent())
        {
            rightPanel->RemoveControl(nodesPropertyPanel);
        }
    }
}

void EditorBodyControl::ToggleHelp(void)
{	
	UIScreen * scr = UIScreenManager::Instance()->GetScreen();
	if (helpDialog->GetParent() == 0)
	{
		scr->AddControl(helpDialog);
	}
	else
	{
		scr->RemoveControl(helpDialog);
	}
}

void EditorBodyControl::PlaceOnLandscape()
{
	SceneNode * selection = scene->GetProxy();

	if (selection && isModeModification)
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

void EditorBodyControl::Input(DAVA::UIEvent *event)
{    
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
        if(!tf)
        {
            Camera * newCamera = 0;
            switch(event->tid)
            {
                case DVKEY_F1:
                case DVKEY_H:
					ToggleHelp();
					break;
                
				case DVKEY_ESCAPE:
                {
                    UIControl *c = UIControlSystem::Instance()->GetFocusedControl();
                    if(c == this || c == scene3dView)
                    {
                        ResetSelection();
                    }
                    
                    break;
                }
					
				case DVKEY_BACKSPACE:
                {
					OnRemoveNodeButtonPressed(0,0,0);
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

                case DVKEY_Q:
                    modState = MOD_MOVE;
                    break;

                case DVKEY_E:
                    modState = MOD_ROTATE;
                    break;

                case DVKEY_R:
                    modState = MOD_SCALE;
                    break;

                case DVKEY_5:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_X;
                    }
                    break;
                }

                case DVKEY_6:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_Y;
                    }
                    break;
                }

                case DVKEY_7:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_Z;
                    }
                    break;
                }

                case DVKEY_8:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        if (modAxis < AXIS_XY) modAxis = AXIS_XY;
                        else modAxis = (eModAxis)(AXIS_XY + ((modAxis + 1 - AXIS_XY) % 3));
                    }
                    
                    break;
                }

                case DVKEY_P:
                {
					PlaceOnLandscape();
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
			UpdateModState();
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
				if (d.Length() > 5 && isModeModification)
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
//						sceneGraphTree->Refresh();
					}
					
					if (selection)
					{
						Logger::Debug(L"init %f %f", event->point.x, event->point.y);
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
				if (selection && isModeModification)
				{
					PrepareModMatrix(event->point);
					selection->SetLocalTransform(currTransform);
					nodesPropertyPanel->UpdateFieldsForCurrentNode();				
					Logger::Debug(L"mod %f %f", event->point.x, event->point.y);
				}
			}
		}
		else if (event->phase == UIEvent::PHASE_ENDED)
		{
			inTouch = false;
			if (isDrag)
			{
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
	switch (modAxis) 
	{
		case AXIS_X:
		case AXIS_Y:
		case AXIS_XY:
			planeNormal = Vector3(0,0,1);
			break;
		case AXIS_Z:
		case AXIS_YZ:
			planeNormal = Vector3(1,0,0);
			break;
		case AXIS_XZ:
			planeNormal = Vector3(0,1,0);
			break;
		default:
			break;
	}

	Vector3 from, dir;
	GetCursorVectors(&from, &dir, point);

	bool result = GetIntersectionVectorWithPlane(from, dir, planeNormal, rotationCenter, startDragPoint);
	
	Logger::Debug("startDragPoint %f %f %f", startDragPoint.x, startDragPoint.y, startDragPoint.z);
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
	
	if (modState == MOD_MOVE)
	{
		Vector3 from, dir;
		GetCursorVectors(&from, &dir, point);
		
		Vector3 currPoint;
		bool result = GetIntersectionVectorWithPlane(from, dir, planeNormal, rotationCenter, currPoint);
		
		if (result)
		{
			switch (modAxis) 
			{
				case AXIS_X:
					currPoint.y = startDragPoint.y;
					currPoint.z = startDragPoint.z;
					break;
				case AXIS_Y:
					currPoint.x = startDragPoint.x;
					currPoint.z = startDragPoint.z;
					break;
				case AXIS_Z:
					currPoint.x = startDragPoint.x;
					currPoint.y = startDragPoint.y;
					break;
			}
			modification.CreateTranslation(currPoint - startDragPoint);
		}
	}
	else if (modState == MOD_ROTATE)
	{
		Matrix4 d;
		switch (modAxis) 
		{
			case AXIS_X:
			case AXIS_Y:
				modification.CreateRotation(vect[modAxis], winy / 100.0f);
				break;
			case AXIS_Z:
				modification.CreateRotation(vect[modAxis], winx / 100.0f);
				break;
			case AXIS_XY:
				modification.CreateRotation(vect[AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[AXIS_Y], winy / 100.0f);
				modification *= d;
				break;
			case AXIS_YZ:
				modification.CreateRotation(vect[AXIS_Y], winx / 100.0f);
				d.CreateRotation(vect[AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			case AXIS_XZ:
				modification.CreateRotation(vect[AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			default:
				break;
		}
		modification = (translate1 * modification) * translate2;
		
	}
	else if (modState == MOD_SCALE)
	{
//		modification.CreateScale(Vector3(1,1,1) + vect[modAxis] * dist/100);
		modification.CreateScale(Vector3(1,1,1) + Vector3(1,1,1) * (winx/100.0f));
		modification = (translate1 * modification) * translate2;
	}
	currTransform = startTransform * modification;
}


void EditorBodyControl::DrawAfterChilds(const UIGeometricData &geometricData)
{
	UIControl::DrawAfterChilds(geometricData);
	SceneNode * selection = scene->GetProxy();
	if (selection && isModeModification)
	{
		const Rect & rect = scene3dView->GetLastViewportRect();
		Camera * cam = scene->GetCurrentCamera(); 
		Vector2 start = cam->GetOnScreenPosition(rotationCenter, rect);
		Vector2 end;
	
		const Vector3 & vc = cam->GetPosition();
		float32 kf = ((vc - rotationCenter).Length() - cam->GetZNear()) * 0.2;
		
		for(int i = 0; i < 3; i++)
		{
			if (modAxis == i
				|| (i == AXIS_X && (modAxis == AXIS_XY || modAxis == AXIS_XZ)) 
				|| (i == AXIS_Y && (modAxis == AXIS_XY || modAxis == AXIS_YZ)) 
				|| (i == AXIS_Z && (modAxis == AXIS_XZ || modAxis == AXIS_YZ)))
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

		
			if (i == AXIS_X 
				|| (i == AXIS_Y && modAxis == AXIS_Y)
				|| (i == AXIS_Y && modAxis == AXIS_YZ)
				)
			{
				axisSign[i] = (start.x > end.x) ? -1.0f: 1.0f;
			}
			else if (i == AXIS_Y && modAxis == AXIS_XY)
			{
				axisSign[i] = (start.y > end.y) ? -1.0f: 1.0f;				
			}
			else if (i == AXIS_Z)
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
	modificationPopUp->SetSelection(selection);
	if (isModeModification && selection && modificationPanel->GetParent() == 0)
	{
		AddControl(modificationPanel);
	}
	else if ((selection == 0 && modificationPanel->GetParent() != 0) || !isModeModification)
	{
		RemoveControl(modificationPanel);
		modificationPopUp->SetSelection(0);
		if (modificationPopUp->GetParent())
			modificationPopUp->GetParent()->RemoveControl(modificationPopUp);
	}
	
	if (selection)
	{
		rotationCenter = selection->GetWorldTransform().GetTranslationVector();
	}
	
    if(cameraController)
    {
        cameraController->Update(timeElapsed);
    }
    UIControl::Update(timeElapsed);

	BeastProxy::Instance()->Update(beastManager);
	if(BeastProxy::Instance()->IsJobDone(beastManager))
	{
		PackLightmaps();
		BeastProxy::Instance()->SafeDeleteManager(&beastManager);
	}
}

void EditorBodyControl::OnLookAtButtonPressed(BaseObject * obj, void *, void *)
{
    MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode*>(selectedSceneGraphNode);
    if (mesh)
    {
        AABBox3 bbox = mesh->GetBoundingBox();
        AABBox3 transformedBox;
        bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);
        Vector3 center = transformedBox.GetCenter();
        scene->GetCurrentCamera()->SetTarget(center);
    }
}
void EditorBodyControl::OnBakeMatricesPressed(BaseObject * obj, void *, void *)
{
    if (selectedSceneGraphNode)
    {
        selectedSceneGraphNode->BakeTransforms();
    }
}
void EditorBodyControl::OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *)
{
    if (selectedSceneGraphNode)
    {
        SceneNode * parentNode = selectedSceneGraphNode->GetParent();
        if (parentNode)
        {
			scene->ReleaseUserData(selectedSceneGraphNode);
			scene->SetSelection(0);
            parentNode->RemoveNode(selectedSceneGraphNode);
            
            selectedSceneGraphNode = NULL;
            UpdatePropertyPanel();

            sceneGraphTree->Refresh();
        }
        SceneValidator::Instance()->EnumerateSceneTextures();
    }
}

void EditorBodyControl::OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *)
{
    if (selectedSceneGraphNode)
    {
        if (selectedSceneGraphNode->GetDebugFlags() & SceneNode::DEBUG_DRAW_ALL)
        {
            selectedSceneGraphNode->SetDebugFlags(0, true);
        }else
        {
            selectedSceneGraphNode->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL, true);
        }
    }
}

void EditorBodyControl::OpenScene(const String &pathToFile, bool editScene)
{
    if (FileSystem::Instance()->GetExtension(pathToFile) == ".sce")
    {
        if(editScene)
        {
            SceneNode *rootNode = scene->GetRootNode(pathToFile);
            SceneValidator::Instance()->ValidateSceneNode(rootNode);
            
            mainFilePath = pathToFile;
            scene->AddNode(rootNode);
        }
        else
        {
            SceneNode *rootNode = scene->GetRootNode(pathToFile)->Clone();
            SceneValidator::Instance()->ValidateSceneNode(rootNode);
            
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
            SceneValidator::Instance()->ValidateSceneNode(rootNode);

            mainFilePath = pathToFile;
            for (int ci = 0; ci < rootNode->GetChildrenCount(); ++ci)
            {   
                //рут нода это сама сцена в данном случае
                scene->AddNode(rootNode->GetChild(ci));
            }
        }
        else
        {
            SceneNode * rootNode = scene->GetRootNode(pathToFile)->Clone();
            //SceneValidator::Instance()->ValidateSceneNode(rootNode);

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
                
				Matrix4 mod;
				mod.CreateTranslation(nodePos);
				rootNode->SetLocalTransform(rootNode->GetLocalTransform() * mod);
            }
            
            SafeRelease(rootNode); 
        }

        Refresh();
    }
    
    SelectNodeAtTree(scene->GetSelection());
    RefreshDataGraph();
    
    SceneValidator::Instance()->EnumerateSceneTextures();
}

void EditorBodyControl::ReloadRootScene(const String &pathToFile)
{
    scene->ReleaseRootNode(pathToFile);
    
    ReloadNode(scene, pathToFile);
    
    scene->SetSelection(0);
    for (int i = 0; i < nodesToAdd.size(); i++) 
    {
        scene->ReleaseUserData(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->RemoveNode(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->AddNode(nodesToAdd[i].nodeToAdd);
        SafeRelease(nodesToAdd[i].nodeToAdd);
    }
    nodesToAdd.clear();

    selectedSceneGraphNode = NULL;
    UpdatePropertyPanel();
    Refresh();
    sceneGraphTree->Refresh();
    RefreshDataGraph();
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



const String &EditorBodyControl::GetFilePath()
{
    return mainFilePath;
}

void EditorBodyControl::SetFilePath(const String &newFilePath)
{
    mainFilePath = newFilePath;
}

void EditorBodyControl::WillAppear()
{
    selectedSceneGraphNode = NULL;
    selectedDataGraphNode = NULL;

    cameraController->SetSpeed(EditorSettings::Instance()->GetCameraSpeed());
    sceneGraphTree->Refresh();
    RefreshDataGraph();
}

void EditorBodyControl::ShowProperties(bool show)
{
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    if(show && !rightPanel->GetParent())
    {
        if(!ControlsAreLocked())
        {
            AddControl(rightPanel);
            
            ChangeControlWidthRight(scene3dView, -rightSideWidth);
            if(outputPanel)
            {
                ChangeControlWidthRight(outputPanel, -rightSideWidth);   
            }
        }
    }
    else if(!show && rightPanel->GetParent())
    {
        RemoveControl(rightPanel);
        
        ChangeControlWidthRight(scene3dView, rightSideWidth);
        if(outputPanel)
        {
            ChangeControlWidthRight(outputPanel, rightSideWidth);
        }
    }
}

bool EditorBodyControl::PropertiesAreShown()
{
    return (rightPanel->GetParent() != NULL);
}

void EditorBodyControl::ShowSceneGraph(bool show)
{
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    if(show && !leftPanelSceneGraph->GetParent())
    {
        if(!ControlsAreLocked())
        {
            AddControl(leftPanelSceneGraph);
            
            ChangeControlWidthLeft(scene3dView, leftSideWidth);
            if(outputPanel)
            {
                ChangeControlWidthLeft(outputPanel, leftSideWidth);
            }
            
            sceneGraphTree->Refresh();
        }
    }
    else if(!show && leftPanelSceneGraph->GetParent())
    {
        RemoveControl(leftPanelSceneGraph);
        
        ChangeControlWidthLeft(scene3dView, -leftSideWidth);
        if(outputPanel)
        {
            ChangeControlWidthLeft(outputPanel, -leftSideWidth);
        }
    }
    
    if(show)
    {
        UpdatePropertyPanel();
    }
}

bool EditorBodyControl::SceneGraphAreShown()
{
    return (leftPanelSceneGraph->GetParent() != NULL);
}

void EditorBodyControl::ShowDataGraph(bool show)
{
//    ResetSelection();

    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    if(show && !leftPanelDataGraph->GetParent())
    {
        if(!ControlsAreLocked())
        {
            AddControl(leftPanelDataGraph);
            
            ChangeControlWidthLeft(scene3dView, leftSideWidth);
            if(outputPanel)
            {
                ChangeControlWidthLeft(outputPanel, leftSideWidth);
            }
            
            RefreshDataGraph(true);
        }
    }
    else if(!show && leftPanelDataGraph->GetParent())
    {
        RemoveControl(leftPanelDataGraph);
        
        ChangeControlWidthLeft(scene3dView, -leftSideWidth);
        if(outputPanel)
        {
            ChangeControlWidthLeft(outputPanel, -leftSideWidth);
        }
    }

    if(show)
    {
        UpdatePropertyPanel();
    }
}

bool EditorBodyControl::DataGraphAreShown()
{
    return (leftPanelDataGraph->GetParent() != NULL);
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
    scene->AddNode(node);
    sceneGraphTree->Refresh();
    RefreshDataGraph();
}

SceneNode * EditorBodyControl::GetSelectedSGNode()
{
    return selectedSceneGraphNode;
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

void EditorBodyControl::NodesPropertyChanged()
{
    if(selectedSceneGraphNode)
    {
        sceneGraphTree->Refresh();
    }
}

void EditorBodyControl::OnRefreshPressed(BaseObject * obj, void *, void *)
{
    NodesPropertyChanged();
}

void EditorBodyControl::Refresh()
{
    sceneGraphTree->Refresh();
    RefreshDataGraph();
}


void EditorBodyControl::SelectNodeAtTree(DAVA::SceneNode *node)
{
    selectedSceneGraphNode = node;
    if(node)
    {
        List<void *> nodesForSearch;
        
        SceneNode *nd = node;
        SceneNode *topSolidNode = NULL;
        while(nd)   //find solid node
        {
            if(nd->GetSolid())
            {
                topSolidNode = nd;
            }
            nd = nd->GetParent();
        }
        
        if(topSolidNode)
        {
            selectedSceneGraphNode = topSolidNode;
            nd = topSolidNode;
        }
        else
        {
            nd = node;
        }
        
        while(nd)   // fill list of nodes
        {
            nodesForSearch.push_front(nd);
            nd = nd->GetParent();
        }
        
        sceneGraphTree->OpenNodes(nodesForSearch);
        sceneGraphTree->ScrollToData(selectedSceneGraphNode);
        
        RefreshDataGraph();
    }
    else
    {
        sceneGraphTree->Refresh();
    }
    
    UpdatePropertyPanel();
}

void EditorBodyControl::RefreshProperties()
{
    if(selectedSceneGraphNode && (NULL != leftPanelSceneGraph->GetParent()))
    {
        nodesPropertyPanel->ReadFrom(selectedSceneGraphNode);
    }
    else if(selectedDataGraphNode && (NULL != leftPanelDataGraph))
    {
        nodesPropertyPanel->ReadFrom(selectedDataGraphNode);
    }
}

void EditorBodyControl::ResetSelection()
{
    scene->SetSelection(0);
    SelectNodeAtTree(0);
}

void EditorBodyControl::RefreshDataGraph(bool force/* = true*/)
{
    if(force || (NULL != leftPanelDataGraph->GetParent()))
    {
        selectedDataGraphNode = NULL;
        dataNodes.clear();
        
        if(selectedSceneGraphNode)
        {
            selectedSceneGraphNode->GetDataNodes(dataNodes);
        }
        
        dataGraphTree->Refresh();
    }
}

void EditorBodyControl::OnRefreshSceneGraph(BaseObject * obj, void *, void *)
{
    sceneGraphTree->Refresh();
}

void EditorBodyControl::OnRefreshDataGraph(BaseObject * obj, void *, void *)
{
    RefreshDataGraph();
}

void EditorBodyControl::RecreatePropertiesPanelForNode(SceneNode * node)
{
	if(nodesPropertyPanel && nodesPropertyPanel->GetParent())
	{
		nodesPropertyPanel->GetParent()->RemoveControl(nodesPropertyPanel);
	}
	SafeRelease(nodesPropertyPanel);

	nodesPropertyPanel = PropertyControlCreator::Instance()->CreateControlForNode(node, propertyPanelRect, false);
    SafeRetain(nodesPropertyPanel);
	nodesPropertyPanel->SetDelegate(this);
	nodesPropertyPanel->SetWorkingScene(scene);
}

void EditorBodyControl::RecreatePropertiesPanelForNode(DataNode * node)
{
	if(nodesPropertyPanel && nodesPropertyPanel->GetParent())
	{
		nodesPropertyPanel->GetParent()->RemoveControl(nodesPropertyPanel);
	}
	SafeRelease(nodesPropertyPanel);
    
	nodesPropertyPanel = PropertyControlCreator::Instance()->CreateControlForNode(node, propertyPanelRect, false);
    SafeRetain(nodesPropertyPanel);
	nodesPropertyPanel->SetDelegate(this);
	nodesPropertyPanel->SetWorkingScene(scene);
}


void EditorBodyControl::SetViewPortSize(int32 viewportID)
{
    if(currentViewPortID == viewportID)
        return;
    
    currentViewPortID = (eViewPortIDs)viewportID;
    
    ShowSceneGraph(false);
    ShowDataGraph(false);
    ShowProperties(false);
    
    Rect fullRect = GetRect();
    Rect viewRect = Rect(SCENE_OFFSET, SCENE_OFFSET, fullRect.dx - 2 * SCENE_OFFSET, fullRect.dy - 2 * SCENE_OFFSET);
    if(outputPanel)
    {
        viewRect.dy -= outputPanel->GetRect().dy;
    }
    
    Rect newRect = viewRect;
    switch (viewportID)
    {
        case EVPID_IPHONE:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 480, 320);
            break;

        case EVPID_RETINA:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 960, 640);
            break;

        case EVPID_IPAD:
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
        currentViewPortID = EVPID_DEFAULT;
    }
    
    scene3dView->SetRect(viewRect);
}

bool EditorBodyControl::ControlsAreLocked()
{
    return (EVPID_DEFAULT != currentViewPortID);
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

void EditorBodyControl::DragAndDrop(void *who, void *target, int32 mode)
{
    SceneNode *whoNode = SafeRetain((SceneNode *)who);
    SceneNode *targetNode = SafeRetain((SceneNode *)target);
    
    if(whoNode)
    {
        if(UIHierarchy::DRAG_CHANGE_PARENT == mode)
        {
            // select new parent for dragged node
            SceneNode *newParent = (targetNode) ? targetNode : scene;
            
            //skip unused drag
            if(whoNode->GetParent() != newParent)
            {
                // check correct hierarhy (can't drag to child)
                SceneNode *nd = newParent->GetParent();
                while(nd && nd != whoNode)
                {
                    nd = nd->GetParent();
                }
                
                if(!nd)
                {
                    //drag
                    whoNode->GetParent()->RemoveNode(whoNode);
                    newParent->AddNode(whoNode);
                }
            }
        }
        else if(UIHierarchy::DRAG_CHANGE_ORDER == mode)
        {
            if(targetNode && whoNode->GetParent() == targetNode->GetParent())
            {
                whoNode->GetParent()->RemoveNode(whoNode);
                targetNode->GetParent()->InsertBeforeNode(whoNode, targetNode);
            }
        }
        
        //refresh controls
        SelectNodeAtTree(NULL);
        RefreshDataGraph();
    }
    
    SafeRelease(whoNode);
    SafeRelease(targetNode);
}

void EditorBodyControl::PackLightmaps()
{
	LightmapsPacker packer;
	packer.SetInputDir("lightmaps_temp/");
	packer.SetOutputDir(GetFilePath()+"_lightmaps/");
	packer.Pack();
	packer.Compress();
	packer.ParseSpriteDescriptors();

	BeastProxy::Instance()->UpdateAtlas(beastManager, packer.GetAtlasingData());
}
