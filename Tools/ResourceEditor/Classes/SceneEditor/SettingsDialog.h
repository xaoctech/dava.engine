#ifndef __SETTINGS_DIALOG_H__
#define __SETTINGS_DIALOG_H__

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "PropertyList.h"

using namespace DAVA;

class SettingsDialogDelegate
{
public:
    
    virtual void SettingsChanged() = 0;
};

class SettingsDialog: public UIControl, public PropertyListDelegate
{
    
public:
    SettingsDialog(const Rect & rect, SettingsDialogDelegate *newDelegate);
    virtual ~SettingsDialog();
    
    virtual void WillAppear();
    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);


protected:

    void OnClose(BaseObject * object, void * userData, void * callerData);

	Rect propertyRect;
    
    DraggableDialog *dialogPanel;
    PropertyList *propertyList;
    
    Vector<String> languages;
    
    SettingsDialogDelegate *delegate;
};



#endif // __SETTINGS_DIALOG_H__