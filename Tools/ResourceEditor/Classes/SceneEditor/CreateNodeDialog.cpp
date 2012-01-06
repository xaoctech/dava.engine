#include "CreateNodeDialog.h"
#include "ControlsFactory.h"

#include "NodePropertyControl.h"

CreateNodeDialog::CreateNodeDialog(const Rect & rect)
    :   DraggableDialog(rect)
{
    ControlsFactory::CustomizeDialog(this);

    projectPath = "/";
    
    sceneNode = NULL;
    scene = NULL;
    
    dialogDelegate = NULL;
    
    header = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    header->SetFont(ControlsFactory::GetFontLight());
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    AddControl(header);
    
    int32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT - 2;
    int32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH * 2 - 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), L"Cancel");
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnCancel));
    AddControl(btnCancel);
    
    buttonX += ControlsFactory::BUTTON_WIDTH + 1;
    UIButton *btnOk = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), L"Ok");
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnOk));
    AddControl(btnOk);
    
    propertyRect = Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, buttonY - ControlsFactory::BUTTON_HEIGHT);
    
    SafeRelease(btnCancel);
    SafeRelease(btnOk);
}
    
CreateNodeDialog::~CreateNodeDialog()
{
    SafeRelease(sceneNode);

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


void CreateNodeDialog::CreateNode()
{
    propertyList->WriteTo(sceneNode);
}

