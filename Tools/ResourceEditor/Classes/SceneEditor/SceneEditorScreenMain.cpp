#include "SceneEditorScreenMain.h"

#include "EditorBodyControl.h"
#include "LibraryControl.h"

#include "ControlsFactory.h"
#include "../EditorScene.h"
#include "MaterialEditor.h"

#include "LandscapeEditorControl.h"

#include "EditorSettings.h"
#include "SceneValidator.h"

#include "TextureTrianglesDialog.h"

#include "PropertyControlCreator.h"
void SceneEditorScreenMain::LoadResources()
{
    //RenderManager::Instance()->EnableOutputDebugStatsEveryNFrame(30);
    new PropertyControlCreator();
    
    ControlsFactory::CustomizeScreenBack(this);

    font = ControlsFactory::GetFontLight();
    
    //init file system dialog
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fileSystemDialog->SetDelegate(this);
    
    KeyedArchive *keyedArchieve = EditorSettings::Instance()->GetSettings();
    String path = keyedArchieve->GetString("3dDataSourcePath", "/");
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
    
    landscapeEditor = new LandscapeEditorControl(Rect(0, ControlsFactory::BUTTON_HEIGHT + 1, 
                                                      fullRect.dx, fullRect.dy - ControlsFactory::BUTTON_HEIGHT-1));
    
    menuPopup = new MenuPopupControl(GetRect(), ControlsFactory::BUTTON_HEIGHT + LINE_HEIGHT);
    menuPopup->SetDelegate(this);
    
    InitializeNodeDialogs();    
    
    textureTrianglesDialog = new TextureTrianglesDialog();
    
    materialEditor = new MaterialEditor();
    
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
    
    
    sceneGraphButton = ControlsFactory::CreateButton(
                                        Vector2(0, BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), LocalizedString(L"panel.graph.scene"));
    sceneGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSceneGraphPressed));
    AddControl(sceneGraphButton);
    
    dataGraphButton = ControlsFactory::CreateButton(
                                                     Vector2(ControlsFactory::BUTTON_WIDTH, BODY_Y_OFFSET - ControlsFactory::BUTTON_HEIGHT), LocalizedString(L"panel.graph.data"));
    dataGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnDataGraphPressed));
    AddControl(dataGraphButton);
    
    InitializeBodyList();
    
    SetupAnimation();
}

void SceneEditorScreenMain::UnloadResources()
{
    SafeRelease(textureTrianglesDialog);
    SafeRelease(sceneInfoButton);
    
    SafeRelease(settingsDialog);
    
    SafeRelease(sceneGraphButton);
    SafeRelease(dataGraphButton);
    
    ReleaseNodeDialogs();
    
    SafeRelease(menuPopup);
    
    SafeRelease(propertiesButton);
    
    SafeRelease(libraryControl);
    SafeRelease(libraryButton);
    
    SafeRelease(fileSystemDialog);
    
    ReleaseBodyList();
    
    SafeRelease(landscapeEditor);
    
    ReleaseTopMenu();
    
    PropertyControlCreator::Instance()->Release();
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
    btnOpen = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.open"));
    x += dx;
    btnSave = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.save"));
    x += dx;
    btnExport = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.export"));
    x += dx;
    btnMaterials = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.materials"));
    x += dx;
    btnCreate = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.createnode"));
    x += dx;
    btnNew = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.new"));
    x += dx;
    btnProject = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.openproject"));
#ifdef __DAVAENGINE_BEAST__
	x += dx;
	btnBeast = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.beast"));
#endif //#ifdef __DAVAENGINE_BEAST__
	x += dx;
	btnLandscape = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.landscape"));
	x += dx;
	btnViewPortSize = ControlsFactory::CreateButton(Rect(x, y, dx, dy), LocalizedString(L"menu.viewport"));
    
    

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
    AddControl(btnLandscape);
    AddControl(btnViewPortSize);
    

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
	btnLandscape->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnLandscapePressed));
	btnViewPortSize->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnViewPortSize));
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
    SafeRelease(btnLandscape);
    SafeRelease(btnViewPortSize);
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
//        //опен всегда загружает только уровень, но не отдельные части сцены
//            bodies[0]->bodyControl->OpenScene(pathToFile, true);
//            bodies[0]->bodyControl->SetFilePath(pathToFile);
            break;
        }
            
        case DIALOG_OPERATION_MENU_SAVE:
        {
            BodyItem *iBody = FindCurrentBody();
            iBody->bodyControl->SetFilePath(pathToFile);
			
			iBody->bodyControl->PushDebugCamera();

            Scene * scene = iBody->bodyControl->GetScene();


            SceneFileV2 * file = new SceneFileV2();
            file->EnableDebugLog(true);
            file->SaveScene(pathToFile, scene);
            SafeRelease(file);
			iBody->bodyControl->PopDebugCamera();			
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


void SceneEditorScreenMain::OnOpenPressed(BaseObject * obj, void *, void *)
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


void SceneEditorScreenMain::OnSavePressed(BaseObject * obj, void *, void *)
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".sc2");
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_SAVE);
        
        BodyItem *iBody = FindCurrentBody();
        String path = iBody->bodyControl->GetFilePath();
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

void SceneEditorScreenMain::OnExportPressed(BaseObject * obj, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    String path = iBody->bodyControl->GetFilePath();
	String lightmapsSource = path + "_lightmaps/";
	String lightmapsDestination = lightmapsSource;
    if(String::npos == path.find("DataSource"))
    {
        return;
    }
    path.replace(path.find("DataSource"), strlen("DataSource"), "Data");
    
    String fileOnly;
    String pathOnly;
    FileSystem::SplitPath(path, pathOnly, fileOnly);
    FileSystem::Instance()->CreateDirectory(pathOnly, true);
    path = FileSystem::Instance()->ReplaceExtension(path, ".sc2");

	
	iBody->bodyControl->PushDebugCamera();
	
    Scene * scene = iBody->bodyControl->GetScene();

//    for (int i = 0; i < scene->GetMaterialCount(); i++)
//    {
//        Material *m = scene->GetMaterial(i);
//        for (int n = 0; n < Material::TEXTURE_COUNT; n++) 
//        {
//            if (m->names[n].length() > 0 && m->names[n].find("DataSource") != m->names[n].npos)
//            {
//                m->names[n].replace(m->names[n].find("DataSource"), strlen("DataSource"), "Data");
//            }
//        }
//    }
    
    Vector<Material*> materials;
    scene->GetDataNodes(materials);
    for (int i = 0; i < (int)materials.size(); i++)
    {
        Material *m = materials[i];
        if (m->GetName().find("editor.") == String::npos)
        {
            for (int n = 0; n < Material::TEXTURE_COUNT; n++) 
            {
                if (m->textures[n])
                {
                    if (!m->textures[n]->relativePathname.empty()) 
                    {
                        ExportTexture(m->textures[n]->relativePathname);
                    }
                }
            }
        }
    }
    
    NodeExportPreparation(scene);

	lightmapsDestination.replace(lightmapsDestination.find("DataSource"), strlen("DataSource"), "Data");
	FileSystem::Instance()->CreateDirectory(lightmapsDestination, false);
    FileSystem::Instance()->CopyDirectory(lightmapsSource, lightmapsDestination);

    SceneFileV2 * file = new SceneFileV2();
    file->EnableSaveForGame(true);
    file->EnableDebugLog(true);
    file->SaveScene(path.c_str(), scene);
    SafeRelease(file);

    
	iBody->bodyControl->PopDebugCamera();

	
    libraryControl->RefreshTree();
}

void SceneEditorScreenMain::NodeExportPreparation(SceneNode *node)
{
    LandscapeNode *land = dynamic_cast<LandscapeNode *>(node);
    if (land) 
    {
        ExportTexture(land->GetHeightMapPathname());
        for (int i = 0; i < LandscapeNode::TEXTURE_COUNT; i++)
        {
            Texture *t = land->GetTexture((LandscapeNode::eTextureLevel)i);
            if (t) 
            {
                ExportTexture(t->relativePathname);
            }
        }
    }


    
    
    
    
    
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        SceneNode * child = node->GetChild(ci);
        NodeExportPreparation(child);
    }
}

void SceneEditorScreenMain::ExportTexture(const String &textureDataSourcePath)
{
    String fileOnly;
    String pathOnly;
    String pathTo = textureDataSourcePath;
    pathTo.replace(textureDataSourcePath.find("DataSource"), strlen("DataSource"), "Data");
    FileSystem::SplitPath(pathTo, pathOnly, fileOnly);
    FileSystem::Instance()->CreateDirectory(pathOnly, true);
	FileSystem::Instance()->DeleteFile(pathTo);
    FileSystem::Instance()->CopyFile(textureDataSourcePath, pathTo);
}


void SceneEditorScreenMain::OnMaterialsPressed(BaseObject * obj, void *, void *)
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


void SceneEditorScreenMain::OnCreatePressed(BaseObject * obj, void *, void *)
{
    menuPopup->InitControl(MENUID_CREATENODE, btnCreate->GetRect());
    AddControl(menuPopup);
}


void SceneEditorScreenMain::OnNewPressed(BaseObject * obj, void *, void *)
{
    menuPopup->InitControl(MENUID_NEW, btnNew->GetRect());
    AddControl(menuPopup);
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
    iBody->bodyControl->OpenScene(pathName, true);
}

void SceneEditorScreenMain::OnReloadSCE(const String &pathName)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->ReloadRootScene(pathName);
}

void SceneEditorScreenMain::OnAddSCE(const String &pathName)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->OpenScene(pathName, false);
}

void SceneEditorScreenMain::OnSceneGraphPressed(BaseObject * obj, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->ShowDataGraph(false);

    bool areShown = iBody->bodyControl->SceneGraphAreShown();
    iBody->bodyControl->ShowSceneGraph(!areShown);
}

void SceneEditorScreenMain::OnDataGraphPressed(BaseObject * obj, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->ShowSceneGraph(false);
    
    bool areShown = iBody->bodyControl->DataGraphAreShown();
    iBody->bodyControl->ShowDataGraph(!areShown);
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
            nodeDialog->CreateNode(itemID);
            
            AddControl(dialogBack);
            AddControl(nodeDialog);
            break;
        }
            
        case MENUID_NEW:
        {
            switch (itemID) 
            {
                case ENMID_ENPTYSCENE:
                    bodies[0]->bodyControl->ReleaseScene();
                    bodies[0]->bodyControl->CreateScene(false);
                    bodies[0]->bodyControl->Refresh();
                    break;
                    
                case ENMID_SCENE_WITH_CAMERA:
                    bodies[0]->bodyControl->ReleaseScene();
                    bodies[0]->bodyControl->CreateScene(true);
                    bodies[0]->bodyControl->Refresh();
                    break;
                    
                default:
                    break;
            }
            
            break;
        }
            
        case MENUID_VIEWPORT:
        {
            BodyItem *iBody = FindCurrentBody();
            
            if(libraryControl->GetParent())
            {
                RemoveControl(libraryControl);
            }
            
            iBody->bodyControl->UpdateLibraryState(libraryControl->GetParent(), libraryControl->GetRect().dx);
            
            iBody->bodyControl->SetViewPortSize(itemID);
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
                case ECNID_LANDSCAPE:
                {
                    text = LocalizedString(L"menu.createnode.landscape");
                    break;
                }
                    
                case ECNID_LIGHT:
                {
                    text = LocalizedString(L"menu.createnode.light");
                    break;
                }
                    
                case ECNID_SERVICENODE:
                {
                    text = LocalizedString(L"menu.createnode.servicenode");
                    break;
                }
                    
                case ECNID_BOX:
                {
                    text = LocalizedString(L"menu.createnode.box");
                    break;
                }
                    
                case ECNID_SPHERE:
                {
                    text = LocalizedString(L"menu.createnode.sphere");
                    break;
                }
                    
                case ECNID_CAMERA:
                {
                    text = LocalizedString(L"menu.createnode.camera");
                    break;
                }
                    
                default:
                    break;
            }
            
            
            break;
        }
            
        case MENUID_NEW:
        {
            switch (itemID) 
            {
                case ENMID_ENPTYSCENE:
                    text = LocalizedString(L"menu.new.emptyscene");
                    break;
                    
                case ENMID_SCENE_WITH_CAMERA:
                    text = LocalizedString(L"menu.new.scenewithcamera");
                    break;
                    
                default:
                    break;
            }
            
            break;
        }
            
        case MENUID_VIEWPORT:
        {
            switch (itemID)
            {
                case EditorBodyControl::EVPID_IPHONE:
                    text = LocalizedString("menu.viewport.iphone");
                    break;

                case EditorBodyControl::EVPID_RETINA:
                    text = LocalizedString("menu.viewport.retina");
                    break;

                case EditorBodyControl::EVPID_IPAD:
                    text = LocalizedString("menu.viewport.ipad");
                    break;

                case EditorBodyControl::EVPID_DEFAULT:
                    text = LocalizedString("menu.viewport.default");
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
            retCount = ECNID_COUNT;
            break;
        }
            
        case MENUID_NEW:
        {
            retCount = ENMID_COUNT;
            break;
        }
            
        case MENUID_VIEWPORT:
            retCount = EditorBodyControl::EVPID_COUNT;
            break;
            
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
    
//    String path = keyedArchieve->GetString("ProjectPath", "/");
//    
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
//    for(int32 iDlg = 0; iDlg < ECNID_COUNT; ++iDlg)
//    {
//        SafeRelease(nodeDialogs[iDlg]);
//    }

    SafeRelease(nodeDialog);
    
    SafeRelease(dialogBack);
}

void SceneEditorScreenMain::OnLandscapePressed(BaseObject * obj, void *, void *)
{
    if(landscapeEditor->GetParent())
    {
        RemoveControl(landscapeEditor);
    }
    else
    {
        AddControl(landscapeEditor);
    }
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

void SceneEditorScreenMain::OnSettingsPressed(BaseObject * obj, void *, void *)
{
    if(!settingsDialog->GetParent())
    {
        AddControl(settingsDialog);
    }
}

void SceneEditorScreenMain::AutoSaveLevel(BaseObject * obj, void *, void *)
{
    time_t now = time(0);
    tm* utcTime = localtime(&now);
    
    String pathToFile = EditorSettings::Instance()->GetDataSourcePath();
    pathToFile += Format("AutoSave_%04d.%02d.%02d_%02d_%02d.sc2",   utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday, 
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

void SceneEditorScreenMain::OnViewPortSize(DAVA::BaseObject *obj, void *, void *)
{
    menuPopup->InitControl(MENUID_VIEWPORT, btnViewPortSize->GetRect());
    AddControl(menuPopup);
}

void SceneEditorScreenMain::OnSceneInfoPressed(DAVA::BaseObject *obj, void *, void *)
{
    BodyItem *iBody = FindCurrentBody();
    iBody->bodyControl->ToggleSceneInfo();
}

void SceneEditorScreenMain::SettingsChanged()
{
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        EditorScene *scene = bodies[i]->bodyControl->GetScene();
        
        scene->SetForceLodLayer(EditorSettings::Instance()->GetForceLodLayer());
        int32 lodCount = EditorSettings::Instance()->GetLodLayersCount();
        for(int32 iLod = 0; iLod < lodCount; ++iLod)
        {
            float32 nearDistance = EditorSettings::Instance()->GetLodLayerNear(iLod);
            float32 farDistance = EditorSettings::Instance()->GetLodLayerFar(iLod);
            
            scene->ReplaceLodLayer(i, nearDistance, farDistance);
        }
        
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
                for(int32 i = 0; i < bodies.size(); ++i)
                {
                    EditorScene *scene = bodies[i]->bodyControl->GetScene();
                    scene->SetForceLodLayer(key);
                }
                EditorSettings::Instance()->SetForceLodLayer(key);
                EditorSettings::Instance()->Save();
            }
            else if(DVKEY_0 == event->tid)
            {
                for(int32 i = 0; i < bodies.size(); ++i)
                {
                    EditorScene *scene = bodies[i]->bodyControl->GetScene();
                    scene->SetForceLodLayer(-1);
                }
                EditorSettings::Instance()->SetForceLodLayer(-1);
                EditorSettings::Instance()->Save();
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
        
        BodyItem *iBody = FindCurrentBody();
        String path = iBody->bodyControl->GetFilePath();
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
    bodies[0]->bodyControl->OpenScene(pathToFile, true);
    bodies[0]->bodyControl->SetFilePath(pathToFile);
}

void SceneEditorScreenMain::ShowTextureTriangles(PolygonGroup *polygonGroup)
{
    if(textureTrianglesDialog)
    {
        textureTrianglesDialog->Show(polygonGroup);
    }
}

