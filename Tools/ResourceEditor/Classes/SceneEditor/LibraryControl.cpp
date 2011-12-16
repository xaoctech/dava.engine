#include "LibraryControl.h"
#include "./../Collada/ColladaConvert.h"

#include "ControlsFactory.h"

LibraryControl::LibraryControl(const Rect & rect)
    :   UIControl(rect)
{
    scene = NULL;
    controlDelegate = NULL;
    folderPath = "/";
    selectedFileName = "";
    selectedFileNameShort = "";
    
    fontLight = ControlsFactory::CreateFontLight();
    fontDark = ControlsFactory::CreateFontDark();

    ControlsFactory::CustomizePanelControl(this);
    
    fileTreeControl = new UIFileTree(Rect(0, BUTTON_HEIGHT, rect.dx, rect.dy - BUTTON_HEIGHT - rect.dx));
    ControlsFactory::CusomizeListControl(fileTreeControl);
	fileTreeControl->SetDelegate(this);
	fileTreeControl->SetFolderNavigation(false);
    fileTreeControl->EnableRootFolderChange(false);
    fileTreeControl->DisableRootFolderExpanding(true);
	fileTreeControl->SetPath(folderPath, ".dae;.sce;");
    AddControl(fileTreeControl);

    
    //button
    refreshButton = ControlsFactory::CreateButton(Rect(0, 0, rect.dx, BUTTON_HEIGHT), L"Refresh Library");
    refreshButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnRefreshPressed));
    AddControl(refreshButton);
    
    panelDAE = ControlsFactory::CreatePanelControl(Rect(0, rect.dy - rect.dx, rect.dx, rect.dx));
    btnConvert = ControlsFactory::CreateButton(Rect(0, 0, rect.dx, BUTTON_HEIGHT), L"Convert");
    btnConvert->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnConvertPressed));
    panelDAE->AddControl(btnConvert);
    
    int32 btnwidth = (rect.dx - 2) / 2;
    panelSCE = ControlsFactory::CreatePanelControl(Rect(0, rect.dy - rect.dx, rect.dx, rect.dx));
    btnAdd = ControlsFactory::CreateButton(Rect(0, 0, btnwidth, BUTTON_HEIGHT), L"Add");
    btnAdd->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnAddPressed));
    btnEdit = ControlsFactory::CreateButton(Rect(rect.dx - btnwidth, 0, btnwidth, BUTTON_HEIGHT), L"Edit");
    btnEdit->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnEditPressed));
    preview = new UI3DView(Rect(0, BUTTON_HEIGHT, rect.dx, rect.dx - BUTTON_HEIGHT));
    preview->SetDebugDraw(true);
    preview->SetInputEnabled(false);
    panelSCE->AddControl(btnAdd);
    panelSCE->AddControl(btnEdit);
    panelSCE->AddControl(preview);
    
//    scene = new Scene();
//    // Camera setup
//    Camera * cam = new Camera(scene);
//    cam->SetName("editor-camera");
//    cam->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
//    cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
//    cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
//    cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
//    cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
//    
//    scene->AddNode(cam);
//    scene->AddCamera(cam);
//    scene->SetCurrentCamera(cam);
//    SafeRelease(cam);
//
//    preview->SetScene(scene);
}
    
LibraryControl::~LibraryControl()
{
    SafeRelease(scene);
    
    SafeRelease(btnAdd);
    SafeRelease(btnConvert);
    SafeRelease(btnEdit);
    SafeRelease(preview);
    
    SafeRelease(panelDAE);
    SafeRelease(panelSCE);
    
    
    SafeRelease(refreshButton);
    
    SafeRelease(fileTreeControl);
    
    SafeRelease(fontLight);
    SafeRelease(fontDark);
}


void LibraryControl::Update(float32 timeElapsed)
{
    UIControl::Update(timeElapsed);
}

void LibraryControl::WillAppear()
{
    RefreshTree();
}

void LibraryControl::SetPath(const String &path)
{
    folderPath = path + "/DataSource/3d";
    fileTreeControl->SetPath(folderPath, ".dae;.sce");

    if(GetParent())
    {
        RefreshTree();
    }
}


void LibraryControl::OnAddPressed(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(controlDelegate)
    {
        controlDelegate->OnAddSCE(selectedFileName);
    }
}

void LibraryControl::OnEditPressed(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(controlDelegate)
    {
        controlDelegate->OnEditSCE(selectedFileName, selectedFileNameShort);
    }
}

void LibraryControl::OnConvertPressed(DAVA::BaseObject *object, void *userData, void *callerData)
{
    ConvertDaeToSce(selectedFileName);
    RefreshTree();
}

void LibraryControl::OnRefreshPressed(DAVA::BaseObject *object, void *userData, void *callerData)
{
    RefreshTree();
}

void LibraryControl::OnCellSelected(DAVA::UIFileTree *tree, DAVA::UIFileTreeCell *selectedCell)
{
    UITreeItemInfo * itemInfo = selectedCell->GetItemInfo();
	String extension = FileSystem::GetExtension(itemInfo->GetName());
//	if (extension == ".dae" || extension == ".DAE")
	if (0 == UIFileTree::CompareExtensions(extension, ".dae"))
	{
        selectedFileName = itemInfo->GetPathname();
        if(panelSCE->GetParent())
        {
            RemoveControl(panelSCE);
        }
        if(!panelDAE->GetParent())
        {
            AddControl(panelDAE);
        }
	}
    else if (0 == UIFileTree::CompareExtensions(extension, ".sce"))
//    else if(extension == ".sce" || extension == ".SCE")
    {
        selectedFileName = itemInfo->GetPathname();
        selectedFileNameShort = itemInfo->GetName();
        
        if(panelDAE->GetParent())
        {
            RemoveControl(panelDAE);
        }
        
        if(!panelSCE->GetParent())
        {
            AddControl(panelSCE);
        }
        else
        {
            scene->ReleaseRootNode(selectedFileName);
        }

        SafeRelease(scene);
        scene = new Scene();
        preview->SetScene(scene);
        
        SceneFile * file = new SceneFile();
        file->SetDebugLog(true);
        file->LoadScene(selectedFileName.c_str(), scene);
        scene->AddNode(scene->GetRootNode(selectedFileName));
        SafeRelease(file);
        
        if (scene->GetCamera(0))
        {
            scene->SetCurrentCamera(scene->GetCamera(0));
        }
        else
        {
            Camera * cam = new Camera(scene);
            cam->SetName("editor-camera");
            cam->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
            cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            cam->SetPosition(Vector3(10.0f, 0.0f, 0.0f));
            cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
            cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
            
            scene->AddNode(cam);
            scene->AddCamera(cam);
            scene->SetCurrentCamera(cam);
            SafeRelease(cam);
        }
    }
    else
    {
        if(panelDAE->GetParent())
        {
            RemoveControl(panelDAE);
        }
        if(panelSCE->GetParent())
        {
            RemoveControl(panelSCE);
        }
    }
    
    List<UIControl*> children = tree->GetVisibleCells();
    for(List<UIControl*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        UIControl *ctrl = (*it);
        ctrl->SetSelected(false, false);
    }
    
    selectedCell->SetSelected(true, false);

}

void LibraryControl::RefreshTree()
{
    fileTreeControl->Refresh();
}

void LibraryControl::SetDelegate(LibraryControlDelegate *delegate)
{
    controlDelegate = delegate;
}

int32 LibraryControl::CellHeight(UIList *forList, int32 index)
{
    return CELL_HEIGHT;
}
