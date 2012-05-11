#include "LibraryControl.h"
#include "./../Collada/ColladaConvert.h"

#include "ControlsFactory.h"
#include "ScenePreviewControl.h"

LibraryControl::LibraryControl(const Rect & rect)
    :   UIControl(rect)
{
    controlDelegate = NULL;
    folderPath = "/";
    selectedFileName = "";
    selectedFileNameShort = "";
    
    fontLight = ControlsFactory::GetFontLight();
    fontDark = ControlsFactory::GetFontDark();

    ControlsFactory::CustomizePanelControl(this);
    
    int32 panelHeight = ControlsFactory::PREVIEW_PANEL_HEIGHT;
    fileTreeControl = new UIFileTree(Rect(0, ControlsFactory::BUTTON_HEIGHT, 
                                          rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT - panelHeight));
    ControlsFactory::CusomizeListControl(fileTreeControl);
    ControlsFactory::SetScrollbar(fileTreeControl);
	fileTreeControl->SetDelegate(this);
	fileTreeControl->SetFolderNavigation(false);
    fileTreeControl->EnableRootFolderChange(false);
    fileTreeControl->DisableRootFolderExpanding(true);
	fileTreeControl->SetPath(folderPath, ".dae;.sc2");
    AddControl(fileTreeControl);

    //button
    refreshButton = ControlsFactory::CreateButton(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                                  LocalizedString(L"library.refresh"));
    refreshButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnRefreshPressed));
    AddControl(refreshButton);

    panelDAE = ControlsFactory::CreatePanelControl(Rect(0, rect.dy - panelHeight, rect.dx, panelHeight));
    btnConvert = ControlsFactory::CreateButton(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                               LocalizedString(L"library.convert"));
    btnConvert->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnConvertPressed));
    panelDAE->AddControl(btnConvert);
    
    int32 btnwidth = (rect.dx) / 2;
    panelSCE = ControlsFactory::CreatePanelControl(Rect(0, rect.dy - panelHeight, rect.dx, panelHeight));
    btnAdd = ControlsFactory::CreateButton(Rect(0, 0, btnwidth, ControlsFactory::BUTTON_HEIGHT), 
                                           LocalizedString(L"library.add"));
    btnAdd->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnAddPressed));
    btnEdit = ControlsFactory::CreateButton(Rect(rect.dx - btnwidth, 0, btnwidth, ControlsFactory::BUTTON_HEIGHT), 
                                            LocalizedString(L"library.edit"));
    btnEdit->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnEditPressed));
    btnReload = ControlsFactory::CreateButton(Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                              LocalizedString(L"library.reload"));
    btnReload->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnReloadPressed));

    preview = new ScenePreviewControl(Rect(0, ControlsFactory::BUTTON_HEIGHT * 2, 
                                           rect.dx, panelHeight - ControlsFactory::BUTTON_HEIGHT * 2));
    preview->SetDebugDraw(true);
    panelSCE->AddControl(btnAdd);
    panelSCE->AddControl(btnEdit);
    panelSCE->AddControl(btnReload);
    panelSCE->AddControl(preview);
    
    errorMessage = new UIStaticText(panelSCE->GetRect());
    errorMessage->SetMultiline(true);
    errorMessage->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    errorMessage->SetFont(ControlsFactory::GetFontError());
}
    
LibraryControl::~LibraryControl()
{
    SafeRelease(errorMessage);
    
    SafeRelease(btnAdd);
    SafeRelease(btnConvert);
    SafeRelease(btnEdit);
    SafeRelease(btnReload);
    SafeRelease(preview);
    
    SafeRelease(panelDAE);
    SafeRelease(panelSCE);
    
    SafeRelease(refreshButton);
    
    SafeRelease(fileTreeControl);
    
}


void LibraryControl::Update(float32 timeElapsed)
{
    UIControl::Update(timeElapsed);
}

void LibraryControl::WillAppear()
{
    RefreshTree();
}

void LibraryControl::WillDisappear()
{
    if(panelSCE->GetParent())
    {
        preview->ReleaseScene();
        preview->RecreateScene();
        
        RemoveControl(panelSCE);

//        selectedFileName = "";
//        selectedFileNameShort = "";
    }
}


void LibraryControl::SetPath(const String &path)
{
    folderPath = path;
    fileTreeControl->SetPath(folderPath, ".dae;.sc2");

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

    // load sce to scene object
    String path = FileSystem::Instance()->ReplaceExtension(selectedFileName, ".sce");
    Scene * scene = new Scene();
//    scene->RegisterLodLayer(0, 1);
//    scene->RegisterLodLayer(1, 2);
//    scene->RegisterLodLayer(2, 3);
//    scene->RegisterLodLayer(3, 4);
//    scene->RegisterLodLayer(4, 5);
//    scene->RegisterLodLayer(5, 6);
//    scene->RegisterLodLayer(6, 7);
//    scene->RegisterLodLayer(7, 8);
    SceneNode *rootNode = scene->GetRootNode(path);
    scene->AddNode(rootNode);

    // Export to *.sc2    
    path = FileSystem::Instance()->ReplaceExtension(path, ".sc2");
    SceneFileV2 * file = new SceneFileV2();
    file->EnableDebugLog(true);
    file->SaveScene(path.c_str(), scene);
    SafeRelease(file);
    
    SafeRelease(scene);

    RefreshTree();
}

void LibraryControl::OnRefreshPressed(DAVA::BaseObject *object, void *userData, void *callerData)
{
    RefreshTree();
}

UIFileTreeCell *LibraryControl::CellAtIndex(UIFileTree * tree, UITreeItemInfo *entry, int32 index)
{
    int32 width = tree->GetRect().dx;
    
	UIFileTreeCell *c = (UIFileTreeCell *)tree->GetReusableCell("FileTreeCell"); //try to get cell from the reusable cells store
	if(!c)
	{ //if cell of requested type isn't find in the store create new cell
		c = new UIFileTreeCell(Rect(0, 0, width, 20), "FileTreeCell");
	}
	//fill cell whith data
	//c->serverName = GameServer::Instance()->totalServers[index].name + LocalizedString("'s game");

    ControlsFactory::CustomizeExpandButton(c->openButton);
    c->text->SetText(StringToWString(entry->GetName()));
    
    Font *font = ControlsFactory::GetFontDark();
    c->text->SetFont(font);
    c->text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(1.0f, 0.8f, 0.8f, 1.0f);
    
    c->SetSelected(false, false);


    float32 shiftX = entry->GetLevel() * 10.0f;
    Rect r = Rect(width - shiftX - 16, 2, 16, 16);
    
    UIControl *sceneFlagBox = SafeRetain(c->FindByName("sceneFlagBox"));
    if(!sceneFlagBox)
    {
        sceneFlagBox = new UIControl();
        sceneFlagBox->SetName("sceneFlagBox");
        sceneFlagBox->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
        sceneFlagBox->SetSprite("~res:/Gfx/UI/marker", 0);
        sceneFlagBox->SetInputEnabled(false);
        c->AddControl(sceneFlagBox);
    }
    sceneFlagBox->SetVisible(false, false);
    sceneFlagBox->SetRect(r);

    
    String path = entry->GetPathname();
    if(     (String::npos != path.find("DataSource"))
       &&   (".sc2" == FileSystem::Instance()->GetExtension(path)))
    {
        path.replace(path.find("DataSource"), strlen("DataSource"), "Data");
//        path = FileSystem::Instance()->ReplaceExtension(path, ".sc2");

        File *f = File::Create(path, File::OPEN | File::READ);
        
        sceneFlagBox->SetVisible(NULL != f, false);
        
        SafeRelease(f);
    }
    
    if(FileSystem::Instance()->IsDirectory(path))
    {
        c->openButton->SetVisible(true);
    }
    else
    {
        c->openButton->SetVisible(false);
    }
    
    SafeRelease(sceneFlagBox);
    
	return c;//returns cell
}


void LibraryControl::OnCellSelected(DAVA::UIFileTree *tree, DAVA::UIFileTreeCell *selectedCell)
{
    if(errorMessage->GetParent())
    {
        RemoveControl(errorMessage);
    }

    UITreeItemInfo * itemInfo = selectedCell->GetItemInfo();
	String extension = FileSystem::GetExtension(itemInfo->GetName());
	if (0 == CompareStrings(extension, ".dae"))
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
    else if (0 == CompareStrings(extension, ".sc2"))
    {
        selectedFileName = itemInfo->GetPathname();
        selectedFileNameShort = itemInfo->GetName();

        if(panelDAE->GetParent())
        {
            RemoveControl(panelDAE);
        }
        
        int32 error = preview->OpenScene(selectedFileName);
        if(SceneFileV2::ERROR_NO_ERROR == error)
        {
            if(!panelSCE->GetParent())
            {
                AddControl(panelSCE);
            }
        }
        else
        {
            switch (error)
            {
                case SceneFileV2::ERROR_FAILED_TO_CREATE_FILE:
                {
                    errorMessage->SetText(LocalizedString(L"library.errormessage.failedtocreeatefile"));
                    break;
                }

                case SceneFileV2::ERROR_FILE_WRITE_ERROR:
                {
                    errorMessage->SetText(LocalizedString(L"library.errormessage.filewriteerror"));
                    break;
                }

                case SceneFileV2::ERROR_VERSION_IS_TOO_OLD:
                {
                    errorMessage->SetText(LocalizedString(L"library.errormessage.versionistooold"));
                    break;
                }

                case ScenePreviewControl::ERROR_CANNOT_OPEN_FILE:
                {
                    errorMessage->SetText(LocalizedString(L"library.errormessage.cannotopenfile"));
                    break;
                }

                case ScenePreviewControl::ERROR_WRONG_EXTENSION:
                {
                    errorMessage->SetText(LocalizedString(L"library.errormessage.wrongextension"));
                    break;
                }

                default:
                    errorMessage->SetText(LocalizedString(L"library.errormessage.unknownerror"));
                    break;
            }
            
            AddControl(errorMessage);
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

void LibraryControl::OnReloadPressed(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(controlDelegate)
    {
        controlDelegate->OnReloadSCE(selectedFileName);
    }
}
