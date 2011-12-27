#include "SceneEditorScreenMain.h"

#include "EditorBodyControl.h"
#include "LibraryControl.h"

#include "ControlsFactory.h"
#include "../EditorScene.h"
#include "MaterialEditor.h"

#include "CreateLandscapeDialog.h"
#include "CreateLightDialog.h"
#include "CreateServiceNodeDialog.h"
#include "CreateBoxDialog.h"
#include "CreateSphereDialog.h"
#include "CreateCameraDialog.h"


void SceneEditorScreenMain::LoadResources()
{
    //RenderManager::Instance()->EnableOutputDebugStatsEveryNFrame(30);
    ControlsFactory::CustomizeScreenBack(this);

    font = ControlsFactory::CreateFontLight();
    
    //init file system dialog
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fileSystemDialog->SetDelegate(this);
    
    keyedArchieve = new KeyedArchive();
    keyedArchieve->Load("~doc:/ResourceEditorOptions.archive");
    String path = keyedArchieve->GetString("LastSavedPath", "/");
    if(path.length())
    fileSystemDialog->SetCurrentDir(path);
    
    // add line after menu
    Rect fullRect = GetRect();
    AddLineControl(Rect(0, MENU_HEIGHT, fullRect.dx, LINE_HEIGHT));
    CreateTopMenu();
    
    menuPopup = new MenuPopupControl(fullRect, BUTTON_WIDTH, MENU_HEIGHT + LINE_HEIGHT);
    menuPopup->SetDelegate(this);
    
    InitializeNodeDialogs();    
    
    materialEditor = new MaterialEditor();
    
    //add line before body
    AddLineControl(Rect(0, BODY_Y_OFFSET, fullRect.dx, LINE_HEIGHT));
    
    //Library
    libraryButton = ControlsFactory::CreateButton(
                        Rect(fullRect.dx - BUTTON_WIDTH, BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                        L"Library");
    libraryButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnLibraryPressed));
    AddControl(libraryButton);
    
    libraryControl = new LibraryControl(
                            Rect(fullRect.dx - LIBRARY_WIDTH, BODY_Y_OFFSET + 1, LIBRARY_WIDTH, fullRect.dy - BODY_Y_OFFSET - 1)); 
    libraryControl->SetDelegate(this);
    libraryControl->SetPath(path);

    //properties
    propertiesButton = ControlsFactory::CreateButton(
                        Rect(fullRect.dx - (BUTTON_WIDTH*2 + 1), BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                        L"Properties");
    propertiesButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnPropertiesPressed));
    AddControl(propertiesButton);
    
    
    sceneGraphButton = ControlsFactory::CreateButton(
                         Rect(0, BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                         L"Scene Graph");
    sceneGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSceneGraphPressed));
    AddControl(sceneGraphButton);
    
    InitializeBodyList();
}

void SceneEditorScreenMain::UnloadResources()
{
    ReleaseNodeDialogs();
    
    SafeRelease(menuPopup);
    
    SafeRelease(keyedArchieve);
    
    SafeRelease(propertiesButton);
    
    SafeRelease(libraryControl);
    SafeRelease(libraryButton);
    
    SafeRelease(fileSystemDialog);
    
    ReleaseBodyList();
    
    ReleaseTopMenu();
    
    SafeRelease(font);
}


void SceneEditorScreenMain::WillAppear()
{
}

void SceneEditorScreenMain::WillDisappear()
{
	
}

void SceneEditorScreenMain::Update(float32 timeElapsed)
{
    UIScreen::Update(timeElapsed);
}

void SceneEditorScreenMain::Draw(const UIGeometricData &geometricData)
{
    UIScreen::Draw(geometricData);
}

void SceneEditorScreenMain::CreateTopMenu()
{
    int32 x = 0;
    int32 y = 0;
    int32 dx = BUTTON_WIDTH;
    int32 dy = BUTTON_HEIGHT;
    btnOpen = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Open");
    x += dx + 1;
    btnSave = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Save");
    x += dx + 1;
    btnMaterials = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Materials");
    x += dx + 1;
    btnCreate = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Create Node");
    x += dx + 1;
    btnNew = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"New");
    x += dx + 1;
    btnProject = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Open Project");
	x += dx + 1;
	btnBeast = ControlsFactory::CreateButton(Rect(x, y, dx, dy), L"Beast");
    
    

    AddControl(btnOpen);
    AddControl(btnSave);
    AddControl(btnMaterials);
    AddControl(btnCreate);
    AddControl(btnNew);
    AddControl(btnProject);
#ifdef __DAVAENGINE_BEAST__
	AddControl(btnBeast);
#endif

    btnOpen->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenPressed));
    btnSave->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSavePressed));
    btnMaterials->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnMaterialsPressed));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnCreatePressed));
    btnNew->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnNewPressed));
    btnProject->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenProjectPressed));
	btnBeast->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnBeastPressed));
}

void SceneEditorScreenMain::ReleaseTopMenu()
{
    SafeRelease(btnOpen);
    SafeRelease(btnSave);
    SafeRelease(btnMaterials);
    SafeRelease(btnCreate);
    SafeRelease(btnNew);
    SafeRelease(btnProject);
	SafeRelease(btnBeast);
}

void SceneEditorScreenMain::AddLineControl(DAVA::Rect r)
{
    UIControl *lineControl = ControlsFactory::CreateLine(r);
    AddControl(lineControl);
    SafeRelease(lineControl);
}

void SceneEditorScreenMain::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    switch (fileSystemDialogOpMode) 
    {
        case DIALOG_OPERATION_MENU_OPEN:
        {
            BodyItem *iBody = FindCurrentBody();
            iBody->bodyControl->OpenScene(pathToFile);
            
            break;
        }
            
        case DIALOG_OPERATION_MENU_SAVE:
        {
            break;
        }
            
        case DIALOG_OPERATION_MENU_PROJECT:
        {
            keyedArchieve->SetString("LastSavedPath", pathToFile);
            keyedArchieve->Save("~doc:/ResourceEditorOptions.archive");
            
            libraryControl->SetPath(pathToFile);
            break;
        }

        default:
            break;
    }

    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}

void SceneEditorScreenMain::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}


void SceneEditorScreenMain::OnOpenPressed(BaseObject * obj, void *, void *)
{
//    if(!fileSystemDialog->GetParent())
//    {
//        fileSystemDialog->SetExtensionFilter(".sce");
//        fileSystemDialog->Show(this);
//        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_OPEN;
//    }

    
    Scene * scene = bodies[0]->bodyControl->GetScene();
    
    SceneFile2 * file = new SceneFile2();
    file->EnableDebugLog(true);
    file->LoadScene("scene.sc2", scene);
    SafeRelease(file);
}


void SceneEditorScreenMain::OnSavePressed(BaseObject * obj, void *, void *)
{
    Scene * scene = bodies[0]->bodyControl->GetScene();
    
    SceneFile2 * file = new SceneFile2();
    file->EnableDebugLog(true);
    file->SaveScene("scene.sc2", scene);
    SafeRelease(file);
    
//    if(!fileSystemDialog->GetParent())
//    {
//        fileSystemDialog->SetExtensionFilter(".sc2");
//        fileSystemDialog->Show(this);
//        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_SAVE;
//    }
}


void SceneEditorScreenMain::OnMaterialsPressed(BaseObject * obj, void *, void *)
{
    if (!materialEditor->GetParent())
    {
        BodyItem *iBody = FindCurrentBody();
        materialEditor->SetWorkingScene(iBody->bodyControl->GetScene());

        AddControl(materialEditor);
    }
    else 
    {
        RemoveControl(materialEditor);
    }

}


void SceneEditorScreenMain::OnCreatePressed(BaseObject * obj, void *, void *)
{
    menuPopup->InitControl(MENUID_CREATENODE, btnCreate->GetRect());
    AddControl(menuPopup);
}


void SceneEditorScreenMain::OnNewPressed(BaseObject * obj, void *, void *)
{
}


void SceneEditorScreenMain::OnOpenProjectPressed(BaseObject * obj, void *, void *)
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_CHOOSE_DIR);
        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_PROJECT;
    }
}


void SceneEditorScreenMain::InitializeBodyList()
{
    AddBodyItem(L"Level", false);
}

void SceneEditorScreenMain::ReleaseBodyList()
{
    for(Vector<BodyItem*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
    {
        BodyItem *iBody = *it;

        RemoveControl(iBody->headerButton);
        RemoveControl(iBody->bodyControl);
        
        SafeRelease(iBody->headerButton);
        SafeRelease(iBody->closeButton);
        SafeRelease(iBody->bodyControl);

        SafeDelete(iBody);
    }
    bodies.clear();
}

void SceneEditorScreenMain::AddBodyItem(const WideString &text, bool isCloseable)
{
    BodyItem *c = new BodyItem();
    
    int32 count = bodies.size();
    c->headerButton = ControlsFactory::CreateButton(
                        Rect(TAB_BUTTONS_OFFSET + count * (BUTTON_WIDTH + 1), 
                             BODY_Y_OFFSET - BUTTON_HEIGHT, 
                             BUTTON_WIDTH, 
                             BUTTON_HEIGHT), 
                        text);

    c->headerButton->SetTag(count);
    c->headerButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSelectBody));
    if(isCloseable)
    {
        c->closeButton = ControlsFactory::CreateCloseWindowButton(Rect(BUTTON_WIDTH - BUTTON_HEIGHT, 0, BUTTON_HEIGHT, BUTTON_HEIGHT));
        c->closeButton->SetTag(count);
        c->closeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnCloseBody));
        c->headerButton->AddControl(c->closeButton);
    }
    else 
    {
        c->closeButton = NULL;
    }


    Rect fullRect = GetRect();
    c->bodyControl = new EditorBodyControl(Rect(0, BODY_Y_OFFSET + 1, fullRect.dx, fullRect.dy - BODY_Y_OFFSET - 1));
    c->bodyControl->SetTag(count);    
    
    AddControl(c->headerButton);    
    bodies.push_back(c);
    
    //set as current
    c->headerButton->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
}


void SceneEditorScreenMain::OnSelectBody(BaseObject * owner, void * userData, void * callerData)
{
    UIButton *btn = (UIButton *)owner;
    
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        if(bodies[i]->bodyControl->GetParent())
        {
            if(btn == bodies[i]->headerButton)
            {
                // click on selected body - nothing to do
                return;
            }
            
            if(libraryControl->GetParent())
            {
                RemoveControl(libraryControl);
                bodies[i]->bodyControl->UpdateLibraryState(false, libraryControl->GetRect().dx);
            }
            
            RemoveControl(bodies[i]->bodyControl);
            bodies[i]->headerButton->SetSelected(false, false);
        }
    }
    AddControl(bodies[btn->GetTag()]->bodyControl);
    bodies[btn->GetTag()]->headerButton->SetSelected(true, false);    
    
    if(libraryControl->GetParent())
    {
        BringChildFront(libraryControl);
    }
}
void SceneEditorScreenMain::OnCloseBody(BaseObject * owner, void * userData, void * callerData)
{
    UIButton *btn = (UIButton *)owner;
    int32 tag = btn->GetTag();
    
    bool needToSwitchBody = false;
    Vector<BodyItem*>::iterator it = bodies.begin();
    for(int32 i = 0; i < bodies.size(); ++i, ++it)
    {
        if(btn == bodies[i]->closeButton)
        {
            if(bodies[i]->bodyControl->GetParent())
            {
                RemoveControl(libraryControl);
                
                RemoveControl(bodies[i]->bodyControl);
                needToSwitchBody = true;
            }
            RemoveControl(bodies[i]->headerButton);
            
            SafeRelease(bodies[i]->headerButton);
            SafeRelease(bodies[i]->closeButton);
            SafeRelease(bodies[i]->bodyControl);            
            SafeDelete(*it);

            bodies.erase(it);
            break;
        }
    }

    for(int32 i = 0; i < bodies.size(); ++i)
    {
        bodies[i]->headerButton->SetRect(Rect(TAB_BUTTONS_OFFSET + i * (BUTTON_WIDTH + 1), BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT));
        
        bodies[i]->headerButton->SetTag(i);
        if(bodies[i]->closeButton)
        {
            bodies[i]->closeButton->SetTag(i);
        }
        bodies[i]->bodyControl->SetTag(i);
    }
    
    if(bodies.size())
    {
        if(tag)
        {
            --tag;
        }

        //set as current
        bodies[tag]->headerButton->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
    }
}


void SceneEditorScreenMain::OnLibraryPressed(DAVA::BaseObject *obj, void *, void *)
{
    if(libraryControl->GetParent())
    {
        RemoveControl(libraryControl);
    }
    else
    {
        AddControl(libraryControl);
    }

    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->ShowProperties(false);
    iBody->bodyControl->UpdateLibraryState(libraryControl->GetParent(), libraryControl->GetRect().dx);
}

SceneEditorScreenMain::BodyItem * SceneEditorScreenMain::FindCurrentBody()
{
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        if(bodies[i]->bodyControl->GetParent())
        {
            return bodies[i];
        }
    }
    
    return NULL;
}

void SceneEditorScreenMain::OnPropertiesPressed(DAVA::BaseObject *obj, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    bool areShown = iBody->bodyControl->PropertiesAreShown();
    iBody->bodyControl->ShowProperties(!areShown);
    
    if(!areShown)
    {
        if(libraryControl->GetParent())
        {
            RemoveControl(libraryControl);
            iBody->bodyControl->UpdateLibraryState(false, libraryControl->GetRect().dx);
        }
    }
}

void SceneEditorScreenMain::OnEditSCE(const String &pathName, const String &name)
{
    AddBodyItem(StringToWString(name), true);
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->OpenScene(pathName);
}

void SceneEditorScreenMain::OnAddSCE(const String &pathName)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->OpenScene(pathName);
}

void SceneEditorScreenMain::OnSceneGraphPressed(BaseObject * obj, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    bool areShown = iBody->bodyControl->SceneGraphAreShown();
    iBody->bodyControl->ShowSceneGraph(!areShown);
}

void SceneEditorScreenMain::OnBeastPressed(BaseObject * obj, void *, void *)
{
	bodies[0]->bodyControl->BeastProcessScene();
}

void SceneEditorScreenMain::MenuCanceled()
{
    RemoveControl(menuPopup);
}

void SceneEditorScreenMain::MenuSelected(int32 menuID, int32 itemID)
{
    RemoveControl(menuPopup);
    
    switch (menuID) 
    {
        case MENUID_CREATENODE:
        {
            currentNodeDialog = itemID;
            
            BodyItem *iBody = FindCurrentBody();
            EditorScene *scene = iBody->bodyControl->GetScene();
            nodeDialogs[currentNodeDialog]->SetScene(scene);

            AddControl(dialogBack);
            AddControl(nodeDialogs[currentNodeDialog]);
            break;
        }
            
        default:
            break;
    }
}

WideString SceneEditorScreenMain::MenuItemText(int32 menuID, int32 itemID)
{
    WideString text = L"";
    
    switch (menuID) 
    {
        case MENUID_CREATENODE:
        {
            switch (itemID) 
            {
                case ECNID_LANDSCAPE:
                {
                    text = L"Landscape";
                    break;
                }
                    
                case ECNID_LIGHT:
                {
                    text = L"Light";
                    break;
                }
                    
                case ECNID_SERVICENODE:
                {
                    text = L"Service Node";
                    break;
                }
                    
                case ECNID_BOX:
                {
                    text = L"Box";
                    break;
                }
                    
                case ECNID_SPHERE:
                {
                    text = L"Sphere";
                    break;
                }
                    
                case ECNID_CAMERA:
                {
                    text = L"Camera";
                    break;
                }
                    
                default:
                    break;
            }
            
            
            break;
        }
            
        default:
            break;
    }

    return text;
}

int32 SceneEditorScreenMain::MenuItemsCount(int32 menuID)
{
    int32 retCount = 0;

    switch (menuID) 
    {
        case MENUID_CREATENODE:
        {
            retCount = ECNID_COUNT;
            break;
        }
            
        default:
            break;
    }

    return retCount;
}

void SceneEditorScreenMain::DialogClosed(int32 retCode)
{
    RemoveControl(nodeDialogs[currentNodeDialog]);
    RemoveControl(dialogBack);
    
    if(CreateNodeDialog::RCODE_OK == retCode)
    {
        BodyItem *iBody = FindCurrentBody();
        iBody->bodyControl->AddNode(nodeDialogs[currentNodeDialog]->GetSceneNode());
    }
}



void SceneEditorScreenMain::InitializeNodeDialogs()
{
    Rect rect = GetRect();
    dialogBack = ControlsFactory::CreatePanelControl(rect);
    ControlsFactory::CustomizeDialogFreeSpace(dialogBack);
    
    String path = keyedArchieve->GetString("LastSavedPath", "/");
    
    Rect r;
    r.dx = rect.dx / 2;
    r.dy = rect.dy / 2;
    
    r.x = rect.x + r.dx / 2;
    r.y = rect.y + r.dy / 2;

    nodeDialogs[ECNID_LANDSCAPE] = new CreateLandscapeDialog(r);    
    nodeDialogs[ECNID_LIGHT] = new CreateLightDialog(r);    
    nodeDialogs[ECNID_SERVICENODE] = new CreateServiceNodeDialog(r);    
    nodeDialogs[ECNID_BOX] = new CreateBoxDialog(r);    
    nodeDialogs[ECNID_SPHERE] = new CreateSphereDialog(r);    
    nodeDialogs[ECNID_CAMERA] = new CreateCameraDialog(r);    
    
    for(int32 iDlg = 0; iDlg < ECNID_COUNT; ++iDlg)
    {
        nodeDialogs[iDlg]->SetDelegate(this);
        nodeDialogs[iDlg]->SetProjectPath(path);
    }
}

void SceneEditorScreenMain::ReleaseNodeDialogs()
{
    for(int32 iDlg = 0; iDlg < ECNID_COUNT; ++iDlg)
    {
        SafeRelease(nodeDialogs[iDlg]);
    }
    
    SafeRelease(dialogBack);
}