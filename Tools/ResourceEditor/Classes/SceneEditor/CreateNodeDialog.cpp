#include "CreateNodeDialog.h"
#include "ControlsFactory.h"


CreateNodeDialog::CreateNodeDialog(const Rect & rect)
    :   UIControl(rect)
{
    projectPath = "/";
    
    sceneNode = NULL;
    scene = NULL;
    
    dialogDelegate = NULL;
    
    Rect r;
    r.dx = rect.dx / 2;
    r.dy = rect.dy / 2;
    
    r.x = rect.x + r.dx / 2;
    r.y = rect.y + r.dy / 2;
    UIControl *panel = ControlsFactory::CreatePanelControl(r);
    AddControl(panel);
    
    header = new UIStaticText(Rect(0, 0, r.dx, BUTTON_HEIGHT));
    Font *font = ControlsFactory::CreateFontLight();
    header->SetFont(font);
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    SafeRelease(font);
    panel->AddControl(header);
    
    int32 buttonY = r.dy - BUTTON_HEIGHT - 2;
    int32 buttonX = (r.dx - BUTTON_WIDTH * 2 - 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Rect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT), L"Cancel");
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnCancel));
    panel->AddControl(btnCancel);
    
    buttonX += BUTTON_WIDTH + 1;
    UIButton *btnOk = ControlsFactory::CreateButton(Rect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT), L"Ok");
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnOk));
    panel->AddControl(btnOk);
    
    Rect propertyRect(0, BUTTON_HEIGHT, r.dx, buttonY - BUTTON_HEIGHT);
    propertyList = new PropertyList(propertyRect, this);
    panel->AddControl(propertyList);

    
    SafeRelease(btnCancel);
    SafeRelease(btnOk);
    SafeRelease(panel);
}
    
CreateNodeDialog::~CreateNodeDialog()
{
    SafeRelease(header);
    SafeRelease(propertyList);
    dialogDelegate = NULL;
}

void CreateNodeDialog::SetDelegate(CreateNodeDialogDelegeate *delegate)
{
    dialogDelegate = delegate;
}

void CreateNodeDialog::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_CANCEL);
    }
}

void CreateNodeDialog::OnOk(BaseObject * object, void * userData, void * callerData)
{
    CreateNode();
    
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_OK);
    }
}

//void CreateNodeDialog::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
//{
//    
//}
//
//void CreateNodeDialog::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
//{
//    
//}
//
//void CreateNodeDialog::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
//{
//    
//}
//
//void CreateNodeDialog::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
//{
//    
//}


SceneNode * CreateNodeDialog::GetSceneNode()
{
    return sceneNode;
}


void CreateNodeDialog::WillAppear()
{
    if(0 == propertyList->ElementsCount(NULL))
    {
        InitializeProperties();
    }
    
    ClearPropertyValues();
    
    UIControl::WillAppear();
}

void CreateNodeDialog::SetScene(Scene *_scene)
{
    scene = _scene;
}

void CreateNodeDialog::SetHeader(const WideString &headerText)
{
    header->SetText(headerText);
}

void CreateNodeDialog::SetProjectPath(const String &path)
{
    projectPath = path;
}


