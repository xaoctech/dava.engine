#include "CreateNodeDialog.h"
#include "ControlsFactory.h"

CreateNodeDialog::CreateNodeDialog(const Rect & rect)
    :   UIControl(rect)
{
    dialogDelegate = NULL;

    Rect r;
    r.dx = rect.dx / 2;
    r.dy = rect.dy / 2;
    
    r.x = rect.x + r.dx / 2;
    r.y = rect.y + r.dy / 2;
    UIControl *panel = ControlsFactory::CreatePanelControl(r);
    AddControl(panel);
    
    int32 buttonY = r.dy - BUTTON_HEIGHT - 10;
    int32 buttonX = (r.dx - BUTTON_WIDTH * 2 - 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Rect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT), L"Cancel");
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnCancel));
    panel->AddControl(btnCancel);
    
    buttonX += BUTTON_WIDTH + 1;
    UIButton *btnOk = ControlsFactory::CreateButton(Rect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT), L"Ok");
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodeDialog::OnOk));
    panel->AddControl(btnOk);
    
    SafeRelease(btnCancel);
    SafeRelease(btnOk);
    SafeRelease(panel);
}
    
CreateNodeDialog::~CreateNodeDialog()
{
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
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_OK);
    }
}
