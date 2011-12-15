#include "SceneEditorScreenMain.h"

#include "EditorBodyControl.h"
#include "LibraryControl.h"

void SceneEditorScreenMain::LoadResources()
{
    RenderManager::Instance()->EnableOutputDebugStatsEveryNFrame(30);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0.7f, 0.7f, 0.7f, 1.0f));

    font = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    font->SetSize(12);
    font->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    
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
    
    //add line before body
    AddLineControl(Rect(0, BODY_Y_OFFSET, fullRect.dx, LINE_HEIGHT));
    
    //Library
    libraryButton = CustomiseMenuButton(
                        Rect(fullRect.dx - BUTTON_WIDTH, BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                        L"Library");
    libraryButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnLibraryPressed));
    AddControl(libraryButton);
    libraryControl = new LibraryControl(Rect(fullRect.dx - LIBRARY_WIDTH, BODY_Y_OFFSET, LIBRARY_WIDTH, fullRect.dy - BODY_Y_OFFSET)); 
    libraryControl->SetDelegate(this);
    libraryControl->SetPath(path);

    //properties
    propertiesButton = CustomiseMenuButton(
                        Rect(fullRect.dx - (BUTTON_WIDTH*2 - 1), BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT), 
                        L"Properties");
    propertiesButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnPropertiesPressed));
    AddControl(propertiesButton);
    
    
    InitializeBodyList();
}

void SceneEditorScreenMain::UnloadResources()
{
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
    btnOpen = CustomiseMenuButton(Rect(x, y, dx, dy), L"Open");
    x += dx + 1;
    btnSave = CustomiseMenuButton(Rect(x, y, dx, dy), L"Save");
    x += dx + 1;
    btnMaterials = CustomiseMenuButton(Rect(x, y, dx, dy), L"Materials");
    x += dx + 1;
    btnCreate = CustomiseMenuButton(Rect(x, y, dx, dy), L"Create");
    x += dx + 1;
    btnNew = CustomiseMenuButton(Rect(x, y, dx, dy), L"New");
    x += dx + 1;
    btnProject = CustomiseMenuButton(Rect(x, y, dx, dy), L"Open Project");
    
    

    AddControl(btnOpen);
    AddControl(btnSave);
    AddControl(btnMaterials);
    AddControl(btnCreate);
    AddControl(btnNew);
    AddControl(btnProject);

    btnOpen->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenPressed));
    btnSave->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSavePressed));
    btnMaterials->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnMaterialsPressed));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnCreatePressed));
    btnNew->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnNewPressed));
    btnProject->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnOpenProjectPressed));
}

void SceneEditorScreenMain::ReleaseTopMenu()
{
    SafeRelease(btnOpen);
    SafeRelease(btnSave);
    SafeRelease(btnMaterials);
    SafeRelease(btnCreate);
    SafeRelease(btnNew);
    SafeRelease(btnProject);
}

UIButton * SceneEditorScreenMain::CustomiseMenuButton(Rect r, const WideString &text)
{
    UIButton *btn = new UIButton(r);
    CustomizeButton(btn, text);

    return btn;
}

void SceneEditorScreenMain::CustomizeButton(UIButton *btn, const WideString &text)
{
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    
    btn->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    btn->GetStateBackground(UIControl::STATE_SELECTED)->SetColor(Color(0.0f, 0.0f, 1.0f, 0.2f));
    
    
    btn->SetStateFont(UIControl::STATE_PRESSED_INSIDE, font);
    btn->SetStateFont(UIControl::STATE_DISABLED, font);
    btn->SetStateFont(UIControl::STATE_NORMAL, font);
    btn->SetStateFont(UIControl::STATE_SELECTED, font);
    
    btn->SetStateText(UIControl::STATE_PRESSED_INSIDE, text);
    btn->SetStateText(UIControl::STATE_DISABLED, text);
    btn->SetStateText(UIControl::STATE_NORMAL, text);
    btn->SetStateText(UIControl::STATE_SELECTED, text);
}


void SceneEditorScreenMain::AddLineControl(DAVA::Rect r)
{
    UIControl *lineControl = new UIControl(r); 
    lineControl->GetBackground()->color = Color(0.8f, 0.8f, 0.8f, 1.0f);
    lineControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    AddControl(lineControl);
    SafeRelease(lineControl);
}

void SceneEditorScreenMain::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    switch (fileSystemDialogOpMode) 
    {
        case DIALOG_OPERATION_MENU_OPEN:
        {
            int32 iBody = FindCurrentBody();
            if(-1 != iBody)
            {
                bodies[iBody].bodyControl->OpenScene(pathToFile);
            }
            
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
            
            libraryControl->SetPath(pathToFile+"/DataSource");
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
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".sce");
        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_OPEN;
    }
}


void SceneEditorScreenMain::OnSavePressed(BaseObject * obj, void *, void *)
{
    if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".dae");
        fileSystemDialog->Show(this);
        fileSystemDialogOpMode = DIALOG_OPERATION_MENU_SAVE;
    }
}


void SceneEditorScreenMain::OnMaterialsPressed(BaseObject * obj, void *, void *)
{
    
}


void SceneEditorScreenMain::OnCreatePressed(BaseObject * obj, void *, void *)
{
    
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
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        RemoveControl(bodies[i].headerButton);
        RemoveControl(bodies[i].bodyControl);
        
        SafeRelease(bodies[i].headerButton);
        SafeRelease(bodies[i].closeButton);
        SafeRelease(bodies[i].bodyControl);
    }
    bodies.clear();
}

void SceneEditorScreenMain::AddBodyItem(const WideString &text, bool isCloseable)
{
    BodyItem c;
    
    int32 count = bodies.size();
    c.headerButton = new UIButton(Rect(count * (BUTTON_WIDTH + 1), BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT));
    CustomizeButton(c.headerButton, text);
    c.headerButton->SetTag(count);
    c.headerButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnSelectBody));
    if(isCloseable)
    {
        c.closeButton = new UIButton(Rect(BUTTON_WIDTH - BUTTON_HEIGHT, 0, BUTTON_HEIGHT, BUTTON_HEIGHT));
        c.closeButton->SetTag(count);
        
        c.closeButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
        c.closeButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.5f, 0.0, 0.0, 1.f));
        
        c.closeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneEditorScreenMain::OnCloseBody));
        c.headerButton->AddControl(c.closeButton);
    }
    else 
    {
        c.closeButton = NULL;
    }


    Rect fullRect = GetRect();
    c.bodyControl = new EditorBodyControl(Rect(0, BODY_Y_OFFSET, fullRect.dx, fullRect.dy - BODY_Y_OFFSET));
    c.bodyControl->SetTag(count);    
    
    AddControl(c.headerButton);    
    bodies.push_back(c);
    
    //set as current
    c.headerButton->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
}


void SceneEditorScreenMain::OnSelectBody(BaseObject * owner, void * userData, void * callerData)
{
    UIButton *btn = (UIButton *)owner;
    
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        if(bodies[i].bodyControl->GetParent())
        {
            if(btn == bodies[i].headerButton)
            {
                // click on selected body - nothing to do
                return;
            }
            
            RemoveControl(bodies[i].bodyControl);
            bodies[i].headerButton->SetSelected(false, false);
        }
    }
    AddControl(bodies[btn->GetTag()].bodyControl);
    bodies[btn->GetTag()].headerButton->SetSelected(true, false);    
    
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
    Vector<BodyItem>::iterator it = bodies.begin();
    for(int32 i = 0; i < bodies.size(); ++i, ++it)
    {
        if(btn == bodies[i].closeButton)
        {
            if(bodies[i].bodyControl->GetParent())
            {
                RemoveControl(bodies[i].bodyControl);
                needToSwitchBody = true;
            }
            RemoveControl(bodies[i].headerButton);
            
            SafeRelease(bodies[i].headerButton);
            SafeRelease(bodies[i].closeButton);
            SafeRelease(bodies[i].bodyControl);            

            bodies.erase(it);
            break;
        }
    }

    for(int32 i = 0; i < bodies.size(); ++i, ++it)
    {
        bodies[i].headerButton->SetRect(Rect(i * (BUTTON_WIDTH + 1), BODY_Y_OFFSET - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT));

        
        bodies[i].headerButton->SetTag(i);
        if(bodies[i].closeButton)
        {
            bodies[i].closeButton->SetTag(i);
        }
        bodies[i].bodyControl->SetTag(i);
    }
    
    if(bodies.size())
    {
        if(tag)
        {
            --tag;
        }

        //set as current
        bodies[tag].headerButton->PerformEvent(UIControl::EVENT_TOUCH_UP_INSIDE);
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
}

int32 SceneEditorScreenMain::FindCurrentBody()
{
    for(int32 i = 0; i < bodies.size(); ++i)
    {
        if(bodies[i].bodyControl->GetParent())
        {
            return i;
        }
    }
    
    return -1;
}

void SceneEditorScreenMain::OnPropertiesPressed(DAVA::BaseObject *obj, void *, void *)
{
    int32 iBody = FindCurrentBody();
    if(-1 != iBody)
    {
        bool areShown = bodies[iBody].bodyControl->PropertiesAreShown();
        bodies[iBody].bodyControl->ShowProperties(!areShown);
    }
}

void SceneEditorScreenMain::OnEditSCE(const String &pathName, const String &name)
{
    AddBodyItem(StringToWString(name), true);
    int32 iBody = bodies.size() - 1;
    bodies[iBody].bodyControl->OpenScene(pathName);
}

void SceneEditorScreenMain::OnAddSCE(const String &pathName)
{
    int32 iBody = FindCurrentBody();
    if(-1 != iBody)
    {
        bodies[iBody].bodyControl->OpenScene(pathName);
    }
}

