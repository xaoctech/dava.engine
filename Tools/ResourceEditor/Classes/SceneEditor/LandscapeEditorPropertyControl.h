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
    virtual void TextureWillChanged(const String &forKey) = 0;
    virtual void TextureDidChanged(const String &forKey) = 0;
};

class LandscapeEditorPropertyControl: public LandscapePropertyControl
{
public:
    
    enum eEditorMode
    {
        MASK_EDITOR_MODE = 0,
        HEIGHT_EDITOR_MODE,
		COLORIZE_EDITOR_MODE
    };
    
	LandscapeEditorPropertyControl(const Rect & rect, bool createNodeProperties, eEditorMode mode);
	virtual ~LandscapeEditorPropertyControl();

    virtual void Input(UIEvent *currentInput);

    
	virtual void ReadFrom(Entity * sceneNode);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnTexturePreviewPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);

    void SetDelegate(LandscapeEditorPropertyControlDelegate *newDelegate);
    
    LandscapeEditorSettings * Settings();
    
protected:

    void SetValuesFromSettings();
    
    LandscapeEditorSettings *settings;
    LandscapeEditorPropertyControlDelegate *delegate;
    
    eEditorMode editorMode;
};

#endif //__LANDSCAPE_PROPERTY_CONTROL_H__
