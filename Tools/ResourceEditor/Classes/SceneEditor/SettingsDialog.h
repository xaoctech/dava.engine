#ifndef __SETTINGS_DIALOG_H__
#define __SETTINGS_DIALOG_H__

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "PropertyList.h"

using namespace DAVA;

class SettingsDialog: public UIControl, public PropertyListDelegate
{
    
public:
    SettingsDialog(const Rect & rect);
    virtual ~SettingsDialog();
    
    virtual void WillAppear();
    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);


protected:

    void OnClose(BaseObject * object, void * userData, void * callerData);

	Rect propertyRect;
    
    DraggableDialog *dialogPanel;
    PropertyList *propertyList;
};



#endif // __SETTINGS_DIALOG_H__