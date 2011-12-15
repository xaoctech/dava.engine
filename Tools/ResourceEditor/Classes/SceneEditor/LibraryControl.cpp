#include "LibraryControl.h"
#include "./../Collada/ColladaConvert.h"


LibraryControl::LibraryControl(const Rect & rect)
    :   UIControl(rect)
{
    scene = NULL;
    controlDelegate = NULL;
    folderPath = "/";
    selectedFileName = "";
    selectedFileNameShort = "";
    
    fontLight = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    fontLight->SetSize(12);
    fontLight->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

    fontDark = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    fontDark->SetSize(12);
    fontDark->SetColor(Color(0.1f, 0.1f, 0.1f, 1.0f));

    
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
    
    fileTreeControl = new UIFileTree(Rect(0, BUTTON_HEIGHT, rect.dx, rect.dy - BUTTON_HEIGHT - rect.dx));
	fileTreeControl->SetDelegate(this);
	fileTreeControl->SetFolderNavigation(false);
    fileTreeControl->EnableRootFolderChange(false);
	fileTreeControl->SetPath(folderPath, ".dae;.sce;");
    AddControl(fileTreeControl);

    
    //button
    refreshButton = CreateButton(Rect(0, 0, rect.dx, BUTTON_HEIGHT), L"Refresh Library");
    refreshButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnRefreshPressed));
    AddControl(refreshButton);
    
    panelDAE = CreatePanel(Rect(0, rect.dy - rect.dx, rect.dx, rect.dx));
    btnConvert = CreateButton(Rect(0, 0, rect.dx, BUTTON_HEIGHT), L"Convert");
    btnConvert->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnConvertPressed));
    panelDAE->AddControl(btnConvert);
    
    int32 btnwidth = (rect.dx - 2) / 2;
    panelSCE = CreatePanel(Rect(0, rect.dy - rect.dx, rect.dx, rect.dx));
    btnAdd = CreateButton(Rect(0, 0, btnwidth, BUTTON_HEIGHT), L"Add");
    btnAdd->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnAddPressed));
    btnEdit = CreateButton(Rect(rect.dx - btnwidth, 0, btnwidth, BUTTON_HEIGHT), L"Edit");
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

UIButton * LibraryControl::CreateButton(Rect r, const WideString &text)
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

UIControl * LibraryControl::CreatePanel(Rect r)
{
    UIControl *ctrl = new UIControl(r);
    ctrl->GetBackground()->color = Color(0.4f, 0.4f, 0.4f, 1.0f);
    ctrl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    
    return ctrl;
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
}

void LibraryControl::RefreshTree()
{
//    fileTreeControl->SetPath(folderPath, ".dae;.sce;.DAE;.SCE");
//    fileTreeControl->SetPath(folderPath, ".dae;.sce");
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
