#include "SceneEditorScreenMain.h"

#include "EditorBodyControl.h"
#include "LibraryControl.h"

#include "ControlsFactory.h"
#include "../EditorScene.h"
#include "MaterialEditor.h"
#include "../ParticlesEditor/ParticlesEditorControl.h"

#include "EditorSettings.h"
#include "SceneValidator.h"

#include "TextureTrianglesDialog.h"
#include "TextureConverterDialog.h"

#include "PropertyControlCreator.h"
#include "ErrorNotifier.h"

#include "HintManager.h"
#include "HelpDialog.h"

#include "UNDOManager.h"

#include "SceneExporter.h"

#if defined (DAVA_QT)
#include "../Qt/SceneData.h"
#include "../Qt/SceneDataManager.h"
#endif //#if defined (DAVA_QT)

void SceneEditorScreenMain::LoadResources()
{
    new ErrorNotifier();
    new HintManager();
    new UNDOManager();
    
    new PropertyControlCreator();
    
    ControlsFactory::CustomizeScreenBack(this);

    font = ControlsFactory::GetFontLight();
    
    //init file system dialog
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fileSystemDialog->SetDelegate(this);
    
    String path = EditorSettings::Instance()->GetDataSourcePath();
    if(path.length())
    fileSystemDialog->SetCurrentDir(path);
    
    // add line after menu
    Rect fullRect = GetRect();
    AddLineControl(Rect(0, ControlsFactory::BUTTON_HEIGHT, fullRect.dx, LINE_HEIGHT));
    CreateTopMenu();
    
    //
    settingsDialog = new SettingsDialog(fullRect, this);
    UIButton *settingsButton = ControlsFactory::CreateImageButton(Rect(fullRect.dx - ControlsFactory::BUTTON_HEIGHT, 0, ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), "~res:/Gfx/UI/settingsicon");

    settingsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSettingsPressed));
    AddControl(settingsButton);
    SafeRelease(settingsButton);
        
    menuPopup = new MenuPopupControl(GetRect(), ControlsFactory::BUTTON_HEIGHT + LINE_HEIGHT);
    menuPopup->SetDelegate(this);
    
    InitializeNodeDialogs();    
    
    textureTrianglesDialog = new TextureTrianglesDialog();
    
    textureConverterDialog = new TextureConverterDialog(fullRect);
    
    materialEditor = new MaterialEditor();
	particlesEditor = new ParticlesEditorControl();
    
    //add line before body
    AddLineControl(Rect(0, BODY_Y_OFFSET, fullRect.dx, LINE_HEIGHT));
    
    //Library
    libraryButton = ControlsFactory::CreateButton(
                                                  Rect(fullRect.dx - ControlsFactory::BUTTON_WIDTH, 
                                                       BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT, 
                                                       ControlsFactory::BUTTON_WIDTH, 
                                                       ControlsFactory::BUTTON_HEIGHT), 
                        LocalizedString(L"panel.library"));
    libraryButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnLibraryPressed));
    AddControl(libraryButton);
    
    int32 libraryWidth = EditorSettings::Instance()->GetRightPanelWidth();
    libraryControl = new LibraryControl(
                            Rect(fullRect.dx - libraryWidth, BODY_Y_OFFSET + 1, libraryWidth, fullRect.dy - BODY_Y_OFFSET - 1)); 
    libraryControl->SetDelegate(this);
    libraryControl->SetPath(path);

    //properties
    propertiesButton = ControlsFactory::CreateButton(
                            Vector2(libraryButton->GetRect().x - ControlsFactory::BUTTON_WIDTH, 
                            BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), 
                        LocalizedString(L"panel.properties"));
    propertiesButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnPropertiesPressed));
    AddControl(propertiesButton);
    
    
    //scene ingo
    sceneInfoButton = ControlsFactory::CreateButton(
                            Vector2(propertiesButton->GetRect().x - ControlsFactory::BUTTON_WIDTH, 
                            BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), 
                            LocalizedString(L"panel.sceneinfo"));
    sceneInfoButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSceneInfoPressed));
    AddControl(sceneInfoButton);
    
    
    sceneGraphButton = ControlsFactory::CreateButton( Vector2(0, BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), LocalizedString(L"panel.graph.scene"));
    sceneGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSceneGraphPressed));
    AddControl(sceneGraphButton);
    
    dataGraphButton = ControlsFactory::CreateButton(Vector2(ControlsFactory::BUTTON_WIDTH, BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), LocalizedString(L"panel.graph.data"));
    dataGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnDataGraphPressed));
    AddControl(dataGraphButton);

	entitiesButton = ControlsFactory::CreateButton(Vector2(ControlsFactory::BUTTON_WIDTH*2, BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), LocalizedString(L"panel.graph.entities"));
	entitiesButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnEntitiesPressed));
	AddControl(entitiesButton);
    
    InitializeBodyList();
    
    SetupAnimation();
    
    helpDialog = new HelpDialog();
}

void SceneEditorScreenMain::UnloadResources()
{
    SafeRelease(helpDialog);
    
    SafeRelease(textureConverterDialog);
    SafeRelease(textureTrianglesDialog);
    SafeRelease(sceneInfoButton);
    
    SafeRelease(settingsDialog);
    
    SafeRelease(sceneGraphButton);
    SafeRelease(dataGraphButton);
	SafeRelease(entitiesButton);
    
    ReleaseNodeDialogs();
    
    SafeRelease(menuPopup);
    
    SafeRelease(propertiesButton);
    
    SafeRelease(libraryControl);
    SafeRelease(libraryButton);
    
    SafeRelease(fileSystemDialog);

	SafeRelease(particlesEditor);
    
    ReleaseBodyList();
        
    ReleaseTopMenu();
    
    HintManager::Instance()->Release();
    PropertyControlCreator::Instance()->Release();
    ErrorNotifier::Instance()->Release();
    UNDOManager::Instance()->Release();
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
    int32 dx = ControlsFactory::BUTTON_WIDTH;
    int32 dy = ControlsFactory::BUTTON_HEIGHT;
    btnOpen = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.open"), true);
    ControlsFactory::CustomizeButtonExpandable(btnOpen);
    x += dx;
    btnSave = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.save"), true);
    x += dx;
    btnExport = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.export"));
    ControlsFactory::CustomizeButtonExpandable(btnExport);
    x += dx;
    btnMaterials = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.materials"), true);
    x += dx;
    btnCreate = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.createnode"), true);
    ControlsFactory::CustomizeButtonExpandable(btnCreate);
    x += dx;
    btnProject = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.openproject"));
#ifdef __DAVAENGINE_BEAST__
	x += dx;
	btnBeast = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.beast"), true);
#endif //#ifdef __DAVAENGINE_BEAST__
	x += dx;
	btnLandscapeHeightmap = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.landscape.heightmap"), true);
	x += dx;
	btnLandscapeColor = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.landscape.colormap"), true);
	x += dx;
	btnViewPortSize = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.viewport"));
    ControlsFactory::CustomizeButtonExpandable(btnViewPortSize);
	x += dx;
	btnTextureConverter = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.textureconvertor"));
    x += dx;
    btnNew = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.new"));
    
    

    AddControl(btnOpen);
    AddControl(btnSave);
    AddControl(btnExport);
    AddControl(btnMaterials);
    AddControl(btnCreate);
    AddControl(btnNew);
    AddControl(btnProject);
#ifdef __DAVAENGINE_BEAST__
	AddControl(btnBeast);
#endif
    AddControl(btnLandscapeHeightmap);
    AddControl(btnLandscapeColor);
    AddControl(btnViewPortSize);
    AddControl(btnTextureConverter);
    

    btnOpen->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenPressed));
    btnSave->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSavePressed));
    btnExport->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnExportPressed));
    btnMaterials->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnMaterialsPressed));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnCreatePressed));
    btnNew->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnNewPressed));
    btnProject->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenProjectPressed));
#ifdef __DAVAENGINE_BEAST__
	btnBeast->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnBeastPressed));
#endif// #ifdef __DAVAENGINE_BEAST__
	btnLandscapeHeightmap->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnLandscapeHeightmapPressed));
	btnLandscapeColor->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnLandscapeColorPressed));
	btnViewPortSize->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnViewPortSize));
	btnTextureConverter->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnTextureConverter));
}

void SceneEditorScreenMain::ReleaseTopMenu()
{
    SafeRelease(btnOpen);
    SafeRelease(btnSave);
    SafeRelease(btnExport);
    SafeRelease(btnMaterials);
    SafeRelease(btnCreate);
    SafeRelease(btnNew);
    SafeRelease(btnProject);
#ifdef __DAVAENGINE_BEAST__
	SafeRelease(btnBeast);
#endif// #ifdef __DAVAENGINE_BEAST__
    SafeRelease(btnLandscapeHeightmap);
    SafeRelease(btnLandscapeColor);
    SafeRelease(btnViewPortSize);
    SafeRelease(btnTextureConverter);
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
            EditorSettings::Instance()->AddLastOpenedFile(pathToFile);
            OpenFileAtScene(pathToFile);
            break;
        }
            
        case DIALOG_OPERATION_MENU_SAVE:
        {
            EditorSettings::Instance()->AddLastOpenedFile(pathToFile);
            SaveSceneToFile(pathToFile);
            break;
        }
            
        case DIALOG_OPERATION_MENU_PROJECT:
        {
            KeyedArchive *keyedArchieve = EditorSettings::Instance()->GetSettings();
            String projectPath = pathToFile;
            if('/' != projectPath[projectPath.length() - 1])
            {
                projectPath += '/';
            }
            keyedArchieve->SetString("ProjectPath", projectPath);
            keyedArchieve->SetString("3dDataSourcePath", projectPath + "DataSource/3d/");
            EditorSettings::Instance()->Save();
            
            SceneValidator::Instance()->SetPathForChecking(EditorSettings::Instance()->GetProjetcPath());
            libraryControl->SetPath(EditorSettings::Instance()->GetDataSourcePath());
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


void SceneEditorScreenMain::OnOpenPressed(BaseObject *, void *, void *)
{
    if(EditorSettings::Instance()->GetLastOpenedCount())
    {
        Rect rect = btnOpen->GetRect();
        rect.dx = GetRect().dx / 2;
        menuPopup->InitControl(MENUID_OPEN, rect);
        AddControl(menuPopup);
    }
    else
    {
        ShowOpenFileDialog();
    }
}


void SceneEditorScreenMain::OnSavePressed(BaseObject *, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();

    if(iBody->bodyControl->LandscapeEditorActive())
    {
        ErrorNotifier::Instance()->ShowError("Can't save level at Landscape Editor Mode.");
    }
    else if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".sc2");
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_SAVE);
        
#if defined (DAVA_QT)
        SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
        String path = sceneData->GetScenePathname();
#else //#if defined (DAVA_QT)
        String path = iBody->bodyControl->GetFilePath();
#endif //#if defined (DAVA_QT)        
        if (path.length() > 0)
        {
            path = FileSystem::Instance()->ReplaceExtension(path, ".sc2");
            fileSystemDialog->SetCurrentDir(path);
        }
        else 
        {
            fileSystemDialog->SetCurrentDir(EditorSettings::Instance()->GetDataSourcePath());
        }


        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_SAVE;
    }
}

void SceneEditorScreenMain::OnExportPressed(BaseObject *, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    if(iBody->bodyControl->LandscapeEditorActive())
    {
        ErrorNotifier::Instance()->ShowError("Can't save level at Landscape Editor Mode.");
    }
    else
    {
        menuPopup->InitControl(MENUID_EXPORTTOGAME, btnExport->GetRect());
        AddControl(menuPopup);
    }
}


void SceneEditorScreenMain::OnMaterialsPressed(BaseObject *, void *, void *)
{
    MaterialsTriggered();
}


void SceneEditorScreenMain::OnCreatePressed(BaseObject *, void *, void *)
{
    menuPopup->InitControl(MENUID_CREATENODE, btnCreate->GetRect());
    AddControl(menuPopup);
}


void SceneEditorScreenMain::OnNewPressed(BaseObject * obj, void *, void *)
{
    NewScene();
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
    AddBodyItem(LocalizedString(L"panel.level"), false);
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
#if defined (DAVA_QT)
    EditorScene *scene = SceneDataManager::Instance()->RegisterNewScene();
    SceneDataManager::Instance()->ActivateScene(scene);
#endif //#if defined (DAVA_QT)
    
    BodyItem *c = new BodyItem();
    
    int32 count = bodies.size();
    c->headerButton = ControlsFactory::CreateButton(
                        Vector2(TAB_BUTTONS_OFFSET + count * (ControlsFactory::BUTTON_WIDTH + 1), 
                             BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), 
                        text);

    c->headerButton->SetTag(count);
    c->headerButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSelectBody));
    if(isCloseable)
    {
        c->closeButton = ControlsFactory::CreateCloseWindowButton(
                                        Rect(ControlsFactory::BUTTON_WIDTH - ControlsFactory::BUTTON_HEIGHT, 0, 
                                        ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT));
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
    
#if defined (DAVA_QT)
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    c->bodyControl->SetScene(sceneData->GetScene());
    c->bodyControl->SetCameraController(sceneData->GetCameraController());
#endif //#if defined (DAVA_QT)
    
    c->bodyControl->SetTag(count);    
    
    AddControl(c->headerButton);    
    bodies.push_back(c);
    
    //set as current
    c->headerButton->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
}


void SceneEditorScreenMain::OnSelectBody(BaseObject * owner, void * userData, void * callerData)
{
    UIButton *btn = (UIButton *)owner;
    
    for(int32 i = 0; i < (int32)bodies.size(); ++i)
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
    
    
#if defined (DAVA_QT)
    SceneDataManager::Instance()->ActivateScene(bodies[btn->GetTag()]->bodyControl->GetScene());
#endif //#if defined (DAVA_QT)
}
void SceneEditorScreenMain::OnCloseBody(BaseObject * owner, void * userData, void * callerData)
{
    UIButton *btn = (UIButton *)owner;
    int32 tag = btn->GetTag();
    
    bool needToSwitchBody = false;
    Vector<BodyItem*>::iterator it = bodies.begin();
    for(int32 i = 0; i < (int32)bodies.size(); ++i, ++it)
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
            
#if defined (DAVA_QT)
            SceneDataManager::Instance()->ReleaseScene(bodies[i]->bodyControl->GetScene());
#endif //#if defined (DAVA_QT)

            SafeRelease(bodies[i]->headerButton);
            SafeRelease(bodies[i]->closeButton);
            SafeRelease(bodies[i]->bodyControl);
            
            SafeDelete(*it);

            bodies.erase(it);
            break;
        }
    }

    for(int32 i = 0; i < (int32)bodies.size(); ++i)
    {
        bodies[i]->headerButton->SetRect(
                            Rect(TAB_BUTTONS_OFFSET + i * (ControlsFactory::BUTTON_WIDTH), 
                            BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT, 
                            ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT));
        
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
    BodyItem *iBody = FindCurrentBody();
    if(!iBody->bodyControl->ControlsAreLocked())
    {
        if(libraryControl->GetParent())
        {
            RemoveControl(libraryControl);
        }
        else
        {
            AddControl(libraryControl);
        }
        
        iBody->bodyControl->ShowProperties(false);
        iBody->bodyControl->UpdateLibraryState(libraryControl->GetParent(), libraryControl->GetRect().dx);
    }
}

SceneEditorScreenMain::BodyItem * SceneEditorScreenMain::FindCurrentBody()
{
    for(int32 i = 0; i < (int32)bodies.size(); ++i)
    {
        if(bodies[i]->bodyControl->GetParent())
        {
            return bodies[i];
        }
    }
    
    return NULL;
}

void SceneEditorScreenMain::OnPropertiesPressed(DAVA::BaseObject *, void *, void *)
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
#if defined (DAVA_QT)
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    sceneData->EditScene(pathName);
#else //#if defined (DAVA_QT)
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->OpenScene(pathName, true);
#endif //#if defined (DAVA_QT)
}

void SceneEditorScreenMain::OnReloadSCE(const String &pathName)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->ReloadRootScene(pathName);
}

void SceneEditorScreenMain::OnAddSCE(const String &pathName)
{
#if defined (DAVA_QT)
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    sceneData->OpenScene(pathName);
#else //#if defined (DAVA_QT)
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->OpenScene(pathName, false);
#endif //#if defined (DAVA_QT)
}

void SceneEditorScreenMain::OnSceneGraphPressed(BaseObject *, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();

    iBody->bodyControl->ToggleSceneGraph();
}

void SceneEditorScreenMain::OnDataGraphPressed(BaseObject * obj, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();

    iBody->bodyControl->ToggleDataGraph();
}

void SceneEditorScreenMain::OnEntitiesPressed(BaseObject * obj, void *, void *)
{
	BodyItem *iBody = FindCurrentBody();

	iBody->bodyControl->ToggleEntities();
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
        case MENUID_OPEN:
        {
            if(EOMID_OPEN == itemID)
            {
                ShowOpenFileDialog();
            }
            else
            {
                int32 lastIndex = itemID - EOMID_OPENLAST_STARTINDEX;
                String path = EditorSettings::Instance()->GetLastOpenedFile(lastIndex);
                OpenFileAtScene(path);
            }

            break;
        }
            
            
        case MENUID_CREATENODE:
        {
            CreateNode((ResourceEditor::eNodeType)itemID);
            break;
        }
                        
        case MENUID_VIEWPORT:
        {
            SetViewport((ResourceEditor::eViewportType)itemID);
            break;
        }
            
        case MENUID_EXPORTTOGAME:
        {
            ExportAs((ResourceEditor::eExportFormat)itemID);
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
        case MENUID_OPEN:
        {
            if(EOMID_OPEN == itemID)
            {
                text = LocalizedString(L"menu.open.open");
            }
            else
            {
                int32 lastIndex = itemID - EOMID_OPENLAST_STARTINDEX;
                String path = EditorSettings::Instance()->GetLastOpenedFile(lastIndex);
                text = StringToWString(path);
            }
            break;
        }
            
        case MENUID_CREATENODE:
        {
            switch (itemID) 
            {
                case ResourceEditor::NODE_LANDSCAPE:
                {
                    text = LocalizedString(L"menu.createnode.landscape");
                    break;
                }
                    
                case ResourceEditor::NODE_LIGHT:
                {
                    text = LocalizedString(L"menu.createnode.light");
                    break;
                }
                    
                case ResourceEditor::NODE_SERVICE_NODE:
                {
                    text = LocalizedString(L"menu.createnode.servicenode");
                    break;
                }
                    
                case ResourceEditor::NODE_BOX:
                {
                    text = LocalizedString(L"menu.createnode.box");
                    break;
                }
                    
                case ResourceEditor::NODE_SPHERE:
                {
                    text = LocalizedString(L"menu.createnode.sphere");
                    break;
                }
                    
                case ResourceEditor::NODE_CAMERA:
                {
                    text = LocalizedString(L"menu.createnode.camera");
                    break;
                }

				case ResourceEditor::NODE_IMPOSTER:
				{
					text = LocalizedString(L"menu.createnode.imposter");
					break;
				}

				case ResourceEditor::NODE_PARTICLE_EMITTER:
				{
					text = LocalizedString(L"menu.createnode.particleemitter");
					break;
				}

				case ResourceEditor::NODE_USER_NODE:
                {
                    text = LocalizedString(L"menu.createnode.usernode");
                    break;
                }


                default:
                    break;
            }
            
            
            break;
        }
            
        case MENUID_VIEWPORT:
        {
            switch (itemID)
            {
                case ResourceEditor::VIEWPORT_IPHONE:
                    text = LocalizedString("menu.viewport.iphone");
                    break;

                case ResourceEditor::VIEWPORT_RETINA:
                    text = LocalizedString("menu.viewport.retina");
                    break;

                case ResourceEditor::VIEWPORT_IPAD:
                    text = LocalizedString("menu.viewport.ipad");
                    break;

                case ResourceEditor::VIEWPORT_DEFAULT:
                    text = LocalizedString("menu.viewport.default");
                    break;

                default:
                    break;
            }
            break;
        }
            
        case MENUID_EXPORTTOGAME:
        {
            switch (itemID) 
            {
                case ResourceEditor::FORMAT_PNG:
                    text = LocalizedString(L"menu.export.png");
                    break;
                    
                case ResourceEditor::FORMAT_PVR:
                    text = LocalizedString(L"menu.export.pvr");
                    break;

                case ResourceEditor::FORMAT_DXT:
                    text = LocalizedString(L"menu.export.dxt");
                    break;
                    
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
        case MENUID_OPEN:
        {
            retCount = EditorSettings::Instance()->GetLastOpenedCount() + 1;
            break;
        }
        case MENUID_CREATENODE:
        {
            retCount = ResourceEditor::NODE_COUNT;
            break;
        }
            
        case MENUID_VIEWPORT:
        {
            retCount = ResourceEditor::VIEWPORT_COUNT;
            break;
        }

        case MENUID_EXPORTTOGAME:
        {
            retCount = ResourceEditor::FORMAT_COUNT;
            break;
        }
            
        default:
            break;
    }

    return retCount;
}

void SceneEditorScreenMain::DialogClosed(int32 retCode)
{
    RemoveControl(nodeDialog);
    RemoveControl(dialogBack);
    
    if(CreateNodesDialog::RCODE_OK == retCode)
    {
        BodyItem *iBody = FindCurrentBody();
        iBody->bodyControl->AddNode(nodeDialog->GetSceneNode());
    }
}

void SceneEditorScreenMain::InitializeNodeDialogs()
{
    Rect rect = GetRect();
    dialogBack = ControlsFactory::CreatePanelControl(rect);
    ControlsFactory::CustomizeDialogFreeSpace(dialogBack);
    
    Rect r;
    r.dx = rect.dx / 2;
    r.dy = rect.dy / 2;
    
    r.x = rect.x + r.dx / 2;
    r.y = rect.y + r.dy / 2;

    nodeDialog = new CreateNodesDialog(r);
    nodeDialog->SetDelegate(this);
}

void SceneEditorScreenMain::ReleaseNodeDialogs()
{
    SafeRelease(nodeDialog);
    SafeRelease(dialogBack);
}

void SceneEditorScreenMain::OnLandscapeHeightmapPressed(BaseObject *, void *, void *)
{
    HeightmapTriggered();
}

void SceneEditorScreenMain::OnLandscapeColorPressed(BaseObject *, void *, void *)
{
    TilemapTriggered();
}

void SceneEditorScreenMain::EditMaterial(Material *material)
{
    BodyItem *iBody = FindCurrentBody();
    if (!materialEditor->GetParent())
    {
        materialEditor->EditMaterial(iBody->bodyControl->GetScene(), material);
        
        AddControl(materialEditor);
    }
}

void SceneEditorScreenMain::OnSettingsPressed(BaseObject *, void *, void *)
{
    if(!settingsDialog->GetParent())
    {
        AddControl(settingsDialog);
    }
}

void SceneEditorScreenMain::AutoSaveLevel(BaseObject *, void *, void *)
{
    time_t now = time(0);
    tm* utcTime = localtime(&now);
    
    String folderPath = EditorSettings::Instance()->GetDataSourcePath() + "Autosave";
    bool folderExcists = FileSystem::Instance()->IsDirectory(folderPath);
    if(!folderExcists)
    {
        FileSystem::Instance()->CreateDirectory(folderPath);
    }

    
    
    String pathToFile = folderPath + Format("/AutoSave_%04d.%02d.%02d_%02d_%02d.sc2",   
                                            utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday, 
                                            utcTime->tm_hour, utcTime->tm_min);
    
    BodyItem *iBody = bodies[0];
    Scene * scene = iBody->bodyControl->GetScene();
    SceneFileV2 * file = new SceneFileV2();
    file->EnableDebugLog(false);
    file->SaveScene(pathToFile, scene);
    SafeRelease(file);
    
    SetupAnimation();
}

void SceneEditorScreenMain::SetupAnimation()
{
    float32 minutes = EditorSettings::Instance()->GetAutosaveTime();
    Animation * anim = WaitAnimation(minutes * 60.f); 
    anim->AddEvent(Animation::EVENT_ANIMATION_END, Message(this, &SceneEditorScreenMain::AutoSaveLevel));
}

void SceneEditorScreenMain::OnViewPortSize(DAVA::BaseObject *, void *, void *)
{
    menuPopup->InitControl(MENUID_VIEWPORT, btnViewPortSize->GetRect());
    AddControl(menuPopup);
}

void SceneEditorScreenMain::OnSceneInfoPressed(DAVA::BaseObject *, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->ToggleSceneInfo();
}

void SceneEditorScreenMain::SettingsChanged()
{
//    BodyItem *iBody = FindCurrentBody();
//    SceneNode *node = iBody->bodyControl->GetSelectedSGNode();
//    EditorScene *editorScene = iBody->bodyControl->GetScene();
//    editorScene->SetForceLodLayer(node, EditorSettings::Instance()->GetForceLodLayer());
    

    for(int32 i = 0; i < (int32)bodies.size(); ++i)
    {
        EditorScene *scene = bodies[i]->bodyControl->GetScene();
//        scene->SetForceLodLayer(EditorSettings::Instance()->GetForceLodLayer());
//        int32 lodCount = EditorSettings::Instance()->GetLodLayersCount();
//        for(int32 iLod = 0; iLod < lodCount; ++iLod)
//        {
//            float32 nearDistance = EditorSettings::Instance()->GetLodLayerNear(iLod);
//            float32 farDistance = EditorSettings::Instance()->GetLodLayerFar(iLod);
//            
//            scene->ReplaceLodLayer(i, nearDistance, farDistance);
//        }
        
        scene->SetDrawGrid(EditorSettings::Instance()->GetDrawGrid());
    }
}

void SceneEditorScreenMain::Input(DAVA::UIEvent *event)
{
    if(UIEvent::PHASE_KEYCHAR == event->phase)
    {
        bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
        if(altIsPressed)
        {
            int32 key = event->tid - DVKEY_1;
            if(0 <= key && key < 8)
            {
                BodyItem *iBody = FindCurrentBody();
                SceneNode *node = iBody->bodyControl->GetSelectedSGNode();
                EditorScene *editorScene = iBody->bodyControl->GetScene();
                editorScene->SetForceLodLayer(node, key);
            }
            else if(DVKEY_0 == event->tid)
            {
//                for(int32 i = 0; i < bodies.size(); ++i)
//                {
//                    EditorScene *scene = bodies[i]->bodyControl->GetScene();
//                    scene->SetForceLodLayer(-1);
//                }
//                EditorSettings::Instance()->SetForceLodLayer(-1);
                EditorSettings::Instance()->Save();
            }
        }
        
        //ckecking help
        if (event->phase == UIEvent::PHASE_KEYCHAR)
        {
            UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
            if(!tf)
            {
                if((DVKEY_F1 == event->tid) || (DVKEY_H == event->tid))
                {
                    if(helpDialog->GetParent())
                    {
                        helpDialog->Close();
                    }
                    else 
                    {
                        helpDialog->Show();
                    }
                }
            }
        }
    }
}

void SceneEditorScreenMain::ShowOpenFileDialog()
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".sc2");
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_LOAD);
        
        
#if defined (DAVA_QT)
        SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
        String path = sceneData->GetScenePathname();
#else //#if defined (DAVA_QT)
        BodyItem *iBody = FindCurrentBody();
        String path = iBody->bodyControl->GetFilePath();
#endif //#if defined (DAVA_QT)        
        if (path.length() > 0) 
        {
            path = FileSystem::Instance()->ReplaceExtension(path, ".sc2");
            fileSystemDialog->SetCurrentDir(path);
        }
        else 
        {
            fileSystemDialog->SetCurrentDir(EditorSettings::Instance()->GetDataSourcePath());
        }

        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_OPEN;
    }
}

void SceneEditorScreenMain::OpenFileAtScene(const String &pathToFile)
{
    //опен всегда загружает только уровень, но не отдельные части сцены
#if defined (DAVA_QT)
    SceneData *levelScene = SceneDataManager::Instance()->GetLevelScene();
    levelScene->EditScene(pathToFile);
    levelScene->SetScenePathname(pathToFile);
#else //#if defined (DAVA_QT)
    bodies[0]->bodyControl->OpenScene(pathToFile, true);
    bodies[0]->bodyControl->SetFilePath(pathToFile);
#endif //#if defined (DAVA_QT)
}

void SceneEditorScreenMain::ShowTextureTriangles(PolygonGroup *polygonGroup)
{
    if(textureTrianglesDialog)
    {
        textureTrianglesDialog->Show(polygonGroup);
    }
}

void SceneEditorScreenMain::OnTextureConverter(DAVA::BaseObject *, void *, void *)
{
    TextureConverterTriggered();
}


void SceneEditorScreenMain::RecreteFullTilingTexture()
{
    for(int32 i = 0; i < (int32)bodies.size(); ++i)
    {
        bodies[i]->bodyControl->RecreteFullTilingTexture();
    }
}

void SceneEditorScreenMain::EditParticleEmitter(ParticleEmitterNode * emitter)
{
	//BodyItem *iBody = FindCurrentBody();
	if (!particlesEditor->GetParent())
	{
		particlesEditor->SetEmitter(emitter->GetEmitter());
		AddControl(particlesEditor);
	}
}

void SceneEditorScreenMain::NewScene()
{
#if defined (DAVA_QT)
    SceneData *levelScene = SceneDataManager::Instance()->GetLevelScene();
    levelScene->CreateScene(true);
    
    bodies[0]->bodyControl->SetScene(levelScene->GetScene());
    bodies[0]->bodyControl->Refresh();
#else //#if defined (DAVA_QT)
    bodies[0]->bodyControl->ReleaseScene();
    bodies[0]->bodyControl->CreateScene(true);
    bodies[0]->bodyControl->Refresh();
#endif //#if defined (DAVA_QT)    
}


bool SceneEditorScreenMain::SaveIsAvailable()
{
    if(FindCurrentBody()->bodyControl->LandscapeEditorActive())
    {
        ErrorNotifier::Instance()->ShowError("Can't save level at Landscape Editor Mode.");
        return false;
    }

    return true;
}

String SceneEditorScreenMain::CurrentScenePathname()
{
#if defined (DAVA_QT)
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    String pathname = sceneData->GetScenePathname();
#else //#if defined (DAVA_QT)    
    String pathname = FindCurrentBody()->bodyControl->GetFilePath();
#endif //#if defined (DAVA_QT)
    if (0 < pathname.length())
    {
        pathname = FileSystem::Instance()->ReplaceExtension(pathname, ".sc2");
    }

    return pathname;
}


void SceneEditorScreenMain::SaveSceneToFile(const String &pathToFile)
{
#if defined (DAVA_QT)
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    sceneData->SetScenePathname(pathToFile);

    BodyItem *iBody = FindCurrentBody();
#else //#if defined (DAVA_QT)
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->SetFilePath(pathToFile);
#endif //#if defined (DAVA_QT)    
    iBody->bodyControl->PushDebugCamera();
    
    Scene * scene = iBody->bodyControl->GetScene();
    
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    SceneFileV2 * file = new SceneFileV2();
    file->EnableDebugLog(false);
    file->SaveScene(pathToFile, scene);
    SafeRelease(file);
    uint64 endTime = SystemTimer::Instance()->AbsoluteMS();
    Logger::Info("[SAVE SCENE TIME] %d ms", (endTime - startTime));
    
    iBody->bodyControl->PopDebugCamera();			
}

void SceneEditorScreenMain::ExportAs(ResourceEditor::eExportFormat format)
{
    String formatStr;
    switch (format) 
    {
        case ResourceEditor::FORMAT_PNG:
            formatStr = String("png");
            break;
            
        case ResourceEditor::FORMAT_PVR:
            formatStr = String("pvr");
            break;
            
        case ResourceEditor::FORMAT_DXT:
            DVASSERT(0);
            return;
            
        default:
			DVASSERT(0);
            return;
    }
    
    
    BodyItem *iBody = FindCurrentBody();
	iBody->bodyControl->PushDebugCamera();
    
#if defined (DAVA_QT)
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    String filePath = sceneData->GetScenePathname();
#else //#if defined (DAVA_QT)
    String filePath = iBody->bodyControl->GetFilePath();
#endif //#if defined (DAVA_QT)    
    String dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
    String::size_type pos = filePath.find(dataSourcePath);
    if(String::npos != pos)
    {
        filePath = filePath.replace(pos, dataSourcePath.length(), "");
    }
    else 
    {
        DVASSERT(0);
    }
    
    // Get project path
    KeyedArchive *keyedArchieve = EditorSettings::Instance()->GetSettings();
    String projectPath = keyedArchieve->GetString(String("ProjectPath"));
    
    if(!SceneExporter::Instance()) new SceneExporter();
    
    String inFolder = projectPath + String("DataSource/3d/");
    SceneExporter::Instance()->SetInFolder(inFolder);
    SceneExporter::Instance()->SetOutFolder(projectPath + String("Data/3d/"));
    
    SceneExporter::Instance()->SetExportingFormat(formatStr);
    
    //TODO: how to be with removed nodes?
    Set<String> errorsLog;
    SceneExporter::Instance()->ExportScene(iBody->bodyControl->GetScene(), filePath, errorsLog);
    
	iBody->bodyControl->PopDebugCamera();
    libraryControl->RefreshTree();
    
    if(0 < errorsLog.size())
    {
        ErrorNotifier::Instance()->ShowError(errorsLog);
    }
}


void SceneEditorScreenMain::CreateNode(ResourceEditor::eNodeType nodeType)
{
    nodeDialog->CreateNode(nodeType);
    
    AddControl(dialogBack);
    AddControl(nodeDialog);
}

void SceneEditorScreenMain::SetViewport(ResourceEditor::eViewportType viewportType)
{
    BodyItem *iBody = FindCurrentBody();
    
    if(libraryControl->GetParent())
    {
        RemoveControl(libraryControl);
    }
    
    iBody->bodyControl->UpdateLibraryState(libraryControl->GetParent(), libraryControl->GetRect().dx);
    
    iBody->bodyControl->SetViewportSize(viewportType);
}

void SceneEditorScreenMain::MaterialsTriggered()
{
    BodyItem *iBody = FindCurrentBody();
    if (!materialEditor->GetParent())
    {
        materialEditor->SetWorkingScene(iBody->bodyControl->GetScene(), iBody->bodyControl->GetSelectedSGNode());
        
        AddControl(materialEditor);
    }
    else 
    {
        RemoveControl(materialEditor);
        
        iBody->bodyControl->RefreshProperties();
        SceneValidator::Instance()->EnumerateSceneTextures();
    }
}

void SceneEditorScreenMain::TextureConverterTriggered()
{
    if(textureConverterDialog)
    {
        BodyItem *body = FindCurrentBody();
        
        textureConverterDialog->Show(body->bodyControl->GetScene());
    }
}

void SceneEditorScreenMain::HeightmapTriggered()
{
    BodyItem *iBody = FindCurrentBody();
    bool ret = iBody->bodyControl->ToggleLandscapeEditor(ELEMID_HEIGHTMAP);
    if(ret)
    {
        bool selected = btnLandscapeHeightmap->GetSelected();
        btnLandscapeHeightmap->SetSelected(!selected);
    }
}

void SceneEditorScreenMain::TilemapTriggered()
{
    BodyItem *iBody = FindCurrentBody();
    bool ret = iBody->bodyControl->ToggleLandscapeEditor(ELEMID_COLOR_MAP);
    if(ret)
    {
        bool selected = btnLandscapeColor->GetSelected();
        btnLandscapeColor->SetSelected(!selected);
    }
}
