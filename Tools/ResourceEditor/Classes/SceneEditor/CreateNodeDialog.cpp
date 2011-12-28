#include "CreateNodeDialog.h"
#include "ControlsFactory.h"


CreateNodeDialog::CreateNodeDialog(const Rect & rect)
    :   DraggableDialog(rect)
{
    ControlsFactory::CustomizeDialogFreeSpace(this);

    projectPath = "/";
    
    sceneNode = NULL;
    scene = NULL;
    
    dialogDelegate = NULL;
    
    header = new UIStaticText(Rect(0, 0, rect.dx, BUTTON_HEIGHT));
    Font *font = ControlsFactory::GetFontLight();
    header->SetFont(font);
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    AddControl(header);
    
    int32 buttonY = rect.dy - BUTTON_HEIGHT - 2;
    int32 buttonX = (rect.dx - BUTTON_WIDTH * 2 - 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Rect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT), L"Cancel");
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnCancel));
    AddControl(btnCancel);
    
    buttonX += BUTTON_WIDTH + 1;
    UIButton *btnOk = ControlsFactory::CreateButton(Rect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT), L"Ok");
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnOk));
    AddControl(btnOk);
    
    Rect propertyRect(0, BUTTON_HEIGHT, rect.dx, buttonY - BUTTON_HEIGHT);
    propertyList = new PropertyList(propertyRect, this);
    AddControl(propertyList);
    
    SafeRelease(btnCancel);
    SafeRelease(btnOk);
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
    
    DraggableDialog::WillAppear();
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


