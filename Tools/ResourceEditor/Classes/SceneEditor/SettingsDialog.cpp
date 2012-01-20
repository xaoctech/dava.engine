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
    propertyList->AddFloatProperty("settingsdialog.autosave", PropertyList::PROPERTY_IS_EDITABLE);
    languages.push_back("en");
    languages.push_back("ru");
    propertyList->AddComboProperty("settingsdialog.language", languages);
    propertyList->AddBoolProperty("settingsdialog.output", PropertyList::PROPERTY_IS_EDITABLE);
    
    
    propertyList->AddFloatProperty("settingsdialog.cameraspeed1", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("settingsdialog.cameraspeed2", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("settingsdialog.cameraspeed3", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("settingsdialog.cameraspeed4", PropertyList::PROPERTY_IS_EDITABLE);
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
    propertyList->SetFloatPropertyValue("settingsdialog.autosave", EditorSettings::Instance()->GetAutosaveTime());
    
    String language = EditorSettings::Instance()->GetLanguage();
    int32 index = 0;
    for(int32 i = 0; i < languages.size(); ++i)
    {
        if(language == languages[i])
        {
            index = i;
            break;
        }
    }
    propertyList->SetComboPropertyIndex("settingsdialog.language", index);
    propertyList->SetBoolPropertyValue("settingsdialog.output", EditorSettings::Instance()->GetShowOutput());
    
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed1", EditorSettings::Instance()->GetCameraSpeed(0));
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed2", EditorSettings::Instance()->GetCameraSpeed(1));
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed3", EditorSettings::Instance()->GetCameraSpeed(2));
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed4", EditorSettings::Instance()->GetCameraSpeed(3));
}

void SettingsDialog::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    
}

void SettingsDialog::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if ("settingsdialog.autosave" == forKey) 
    {
        EditorSettings::Instance()->SetAutosaveTime(newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.cameraspeed1" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(0, newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.cameraspeed2" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(1, newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.cameraspeed3" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(2, newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.cameraspeed4" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(3, newValue);
        EditorSettings::Instance()->Save();
    }
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
    if("settingsdialog.output" == forKey)
    {
        EditorSettings::Instance()->SetShowOuput(newValue);
        EditorSettings::Instance()->Save();
    }
}

void SettingsDialog::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("settingsdialog.language" == forKey)
    {
        EditorSettings::Instance()->SetLanguage(newItemKey);
        EditorSettings::Instance()->Save();
    }
}

