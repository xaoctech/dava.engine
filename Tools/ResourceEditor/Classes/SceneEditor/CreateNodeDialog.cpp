#include "CreateNodeDialog.h"
#include "ControlsFactory.h"

CreateNodeDialog::CreateNodeDialog(const Rect & rect)
    :   UIControl(rect)
{
    dialogDelegate = NULL;
    currentDescription = NULL;
    
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
    properties = new PropertyList(propertyRect, this);
    panel->AddControl(properties);

    
    SafeRelease(btnCancel);
    SafeRelease(btnOk);
    SafeRelease(panel);
}
    
CreateNodeDialog::~CreateNodeDialog()
{
    SafeRelease(header);
    SafeRelease(properties);
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

void CreateNodeDialog::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    
}

void CreateNodeDialog::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    
}

void CreateNodeDialog::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    
}

void CreateNodeDialog::SetProperties(NodeDescription *description)
{
    currentDescription = description;
    
    header->SetText(currentDescription->name);
    
    properties->ReleaseProperties();
    
    for (int32 i = 0; i < currentDescription->properties.size(); ++i)
    {
        properties->AddPropertyByData(currentDescription->properties[i]);
    }
    
}