#ifndef __LANDSCAPE_EDITOR_PROPERTY_CONTROL_H__
#define __LANDSCAPE_EDITOR_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "LandscapePropertyControl.h"

class LandscapeEditorSettings: public BaseObject
{
public:
    
    LandscapeEditorSettings(); 
    void ResetAll();
    
    bool redMask;
    bool greenMask;
    bool blueMask;
    bool alphaMask;
    
    int32 maskSize;
};

class LandscapeEditorPropertyControlDelegate
{
public: 
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings) = 0;
    virtual void SaveMask() = 0;
    
};

class LandscapeEditorPropertyControl: public LandscapePropertyControl
{
public:
	LandscapeEditorPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~LandscapeEditorPropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);

    void SetDelegate(LandscapeEditorPropertyControlDelegate *newDelegate);
    
    LandscapeEditorSettings * Settings();
    
protected:

    void OnSavePressed(BaseObject * object, void * userData, void * callerData);

    void SetValuesFromSettings();
    
    LandscapeEditorSettings *settings;
    LandscapeEditorPropertyControlDelegate *delegate;
};

#endif //__LANDSCAPE_PROPERTY_CONTROL_H__
