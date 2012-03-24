#include "LandscapeEditorPropertyControl.h"

//*********************  LandscapeEditorSettings  **********************
LandscapeEditorSettings::LandscapeEditorSettings()
{
    maskSize = 1024;
    ResetAll();
}

void LandscapeEditorSettings::ResetAll()
{
    redMask = false;
    greenMask = false;
    blueMask = false;
    alphaMask = false;
}

//*********************  LandscapeEditorPropertyControl  **********************
LandscapeEditorPropertyControl::LandscapeEditorPropertyControl(const Rect & rect, bool createNodeProperties)
    :	LandscapePropertyControl(rect, createNodeProperties)
{
    settings = new LandscapeEditorSettings();
    delegate = NULL;
}

LandscapeEditorPropertyControl::~LandscapeEditorPropertyControl()
{
    SafeRelease(settings);
}


void LandscapeEditorPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("landscapeeditor.maskred" == forKey)
    {
        settings->ResetAll();
        settings->redMask = newValue;
        SetValuesFromSettings();
        if(delegate)
        {
            delegate->LandscapeEditorSettingsChanged(settings);
        }
    }
    else if("landscapeeditor.maskgreen" == forKey)
    {
        settings->ResetAll();
        settings->greenMask = newValue;
        SetValuesFromSettings();
        if(delegate)
        {
            delegate->LandscapeEditorSettingsChanged(settings);
        }
    }
    else if("landscapeeditor.maskblue" == forKey)
    {
        settings->ResetAll();
        settings->blueMask = newValue;
        SetValuesFromSettings();
        if(delegate)
        {
            delegate->LandscapeEditorSettingsChanged(settings);
        }
    }
    else if("landscapeeditor.maskalpha" == forKey)
    {
        settings->ResetAll();
        settings->alphaMask = newValue;
        SetValuesFromSettings();
        if(delegate)
        {
            delegate->LandscapeEditorSettingsChanged(settings);
        }
    }
    else 
    {
        LandscapePropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
    }
}

void LandscapeEditorPropertyControl::SetValuesFromSettings()
{
    propertyList->SetBoolPropertyValue("landscapeeditor.maskred", settings->redMask);
    propertyList->SetBoolPropertyValue("landscapeeditor.maskgreen", settings->greenMask);
    propertyList->SetBoolPropertyValue("landscapeeditor.maskblue", settings->blueMask);
    propertyList->SetBoolPropertyValue("landscapeeditor.maskalpha", settings->alphaMask);
    
}


void LandscapeEditorPropertyControl::ReadFrom(DAVA::SceneNode *sceneNode)
{
    LandscapePropertyControl::ReadFrom(sceneNode);
    
    propertyList->AddSection("landscapeeditor.landscapeeditor", true);
    
    propertyList->AddIntProperty("landscapeeditor.size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetIntPropertyValue("landscapeeditor.size", settings->maskSize);

    
    propertyList->AddBoolProperty("landscapeeditor.maskred", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddBoolProperty("landscapeeditor.maskgreen", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddBoolProperty("landscapeeditor.maskblue", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddBoolProperty("landscapeeditor.maskalpha", PropertyList::PROPERTY_IS_EDITABLE);
    SetValuesFromSettings();

    propertyList->AddMessageProperty("landscapeeditor.savemask", 
                                     Message(this, &LandscapeEditorPropertyControl::OnSavePressed));
}

void LandscapeEditorPropertyControl::SetDelegate(LandscapeEditorPropertyControlDelegate *newDelegate)
{
    delegate = newDelegate;
}

void LandscapeEditorPropertyControl::OnSavePressed(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(delegate)
    {
        delegate->SaveMask();
    }
}

LandscapeEditorSettings * LandscapeEditorPropertyControl::Settings()
{
    return settings;
}

