#include "SettingsDialog.h"
#include "ControlsFactory.h"

#include "EditorSettings.h"

SettingsDialog::SettingsDialog(const Rect & rect)
    :   UIControl(rect)
{
    ControlsFactory::CustomizeDialogFreeSpace(this);

    Rect dialogRect(rect.dx/4, rect.dy/4, rect.dx / 2, rect.dy/2);
    dialogPanel = new DraggableDialog(dialogRect);
    ControlsFactory::CustomizeDialog(dialogPanel);
    AddControl(dialogPanel);

    UIStaticText * header = new UIStaticText(Rect(0, 0, dialogRect.dx, ControlsFactory::BUTTON_HEIGHT));
    header->SetFont(ControlsFactory::GetFontLight());
    header->SetText(LocalizedString(L"settings.header"));
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    dialogPanel->AddControl(header);
    SafeRelease(header);
    
    propertyList = new PropertyList(
                            Rect(0, ControlsFactory::BUTTON_HEIGHT, dialogRect.dx, 
                            dialogRect.dy - ControlsFactory::BUTTON_HEIGHT * 2), this);
    dialogPanel->AddControl(propertyList);
    
    
    int32 buttonY = dialogRect.dy - ControlsFactory::BUTTON_HEIGHT;
    int32 buttonX = (dialogRect.dx - ControlsFactory::BUTTON_WIDTH * 2) / 2;
    UIButton *btnClose = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.close"));
    btnClose->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SettingsDialog::OnClose));
    dialogPanel->AddControl(btnClose);
    SafeRelease(btnClose);
    
    propertyList->AddIntProperty("settingsdialog.screenwidth", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddIntProperty("settingsdialog.screenheight", PropertyList::PROPERTY_IS_EDITABLE);
}
    
SettingsDialog::~SettingsDialog()
{
    SafeRelease(propertyList);
    SafeRelease(dialogPanel);
}

void SettingsDialog::OnClose(BaseObject * object, void * userData, void * callerData)
{
    GetParent()->RemoveControl(this);
}

void SettingsDialog::WillAppear()
{
    propertyList->SetIntPropertyValue("settingsdialog.screenwidth", EditorSettings::Instance()->GetScreenWidth());
    propertyList->SetIntPropertyValue("settingsdialog.screenheight", EditorSettings::Instance()->GetScreenHeight());
}

void SettingsDialog::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    
}

void SettingsDialog::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    
}

void SettingsDialog::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    if("settingsdialog.screenwidth" == forKey)
    {
        EditorSettings::Instance()->SetScreenWidth(newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.screenheight" == forKey)
    {
        EditorSettings::Instance()->SetScreenHeight(newValue);
        EditorSettings::Instance()->Save();
    }
}

void SettingsDialog::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    
}

