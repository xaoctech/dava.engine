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
    
    fileTreeControl = new UIFileTree(Rect(0, BUTTON_HEIGHT, rect.dx, rect.dy - BUTTON_HEIGHT - rect.dx));
    ControlsFactory::CusomizeListControl(fileTreeControl);
    ControlsFactory::SetScrollbar(fileTreeControl);
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
    
    int32 btnwidth = (rect.dx) / 2;
    panelSCE = ControlsFactory::CreatePanelControl(Rect(0, rect.dy - rect.dx, rect.dx, rect.dx));
    btnAdd = ControlsFactory::CreateButton(Rect(0, 0, btnwidth, BUTTON_HEIGHT), L"Add");
    btnAdd->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnAddPressed));
    btnEdit = ControlsFactory::CreateButton(Rect(rect.dx - btnwidth, 0, btnwidth, BUTTON_HEIGHT), L"Edit");
    btnEdit->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LibraryControl::OnEditPressed));
    preview = new ScenePreviewControl(Rect(0, BUTTON_HEIGHT, rect.dx, rect.dx - BUTTON_HEIGHT));
    preview->SetDebugDraw(true);
    panelSCE->AddControl(btnAdd);
    panelSCE->AddControl(btnEdit);
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

void LibraryControl::SetPath(const String &path)
{
    folderPath = path;
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
    
	c->SetStateText(UIControl::STATE_NORMAL, StringToWString(entry->GetName()));
    c->GetStateTextControl(UIControl::STATE_NORMAL)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    c->SetStateText(UIControl::STATE_SELECTED, StringToWString(entry->GetName()));
	c->GetStateTextControl(UIControl::STATE_SELECTED)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    
    c->SetSelected(false, false);


    float32 shiftX = entry->GetLevel() * 10.0f;
    Rect r = Rect(width - shiftX - 16, 2, 16, 16);
    
    UIControl *sceneFlagBox = SafeRetain(c->FindByName("sceneFlagBox"));
    if(!sceneFlagBox)
    {
        sceneFlagBox = new UIControl();
        sceneFlagBox->SetName("sceneFlagBox");
        sceneFlagBox->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
        sceneFlagBox->SetSprite("~res:/Gfx/UI/chekBox", 1);
        sceneFlagBox->SetInputEnabled(false);
        c->AddControl(sceneFlagBox);
    }
    sceneFlagBox->SetVisible(false, false);
    sceneFlagBox->SetRect(r);

    
    String path = entry->GetPathname();
    if(     (String::npos != path.find("DataSource"))
       &&   (".sce" == FileSystem::Instance()->GetExtension(path)))
    {
        path.replace(path.find("DataSource"), strlen("DataSource"), "Data");
        path = FileSystem::Instance()->ReplaceExtension(path, ".sc2");

        File *f = File::Create(path, File::OPEN | File::READ);
        
        sceneFlagBox->SetVisible(NULL != f, false);
        
        SafeRelease(f);
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
    {
        selectedFileName = itemInfo->GetPathname();
        selectedFileNameShort = itemInfo->GetName();

        if(panelDAE->GetParent())
        {
            RemoveControl(panelDAE);
        }
        
        bool isOpened = preview->OpenScene(selectedFileName);
        if(isOpened)
        {
            if(!panelSCE->GetParent())
            {
                AddControl(panelSCE);
            }
        }
        else
        {
            errorMessage->SetText(L"Format is too old, reconvert .dae file.");
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
