#ifndef __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__
#define __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeTool.h"

using namespace DAVA;

class LandscapeToolsPanelHeightmap: 
    public LandscapeToolsPanel, 
    public UITextFieldDelegate
{

public:
    LandscapeToolsPanelHeightmap(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanelHeightmap();
    
    virtual void WillAppear();
    virtual void Update(float32 timeElapsed);

    //UITextFieldDelegate
    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, 
                                     int32 replacementLength, const WideString & replacementString);


    //UICheckBoxDelegate
    virtual void ValueChanged(UICheckBox *forCheckbox, bool newValue);

    //LandscapeToolsSelectionDelegate
    virtual void OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool);

protected:

    void OnDropperTool(BaseObject * object, void * userData, void * callerData);
    void OnCopypasteTool(BaseObject * object, void * userData, void * callerData);
    virtual void ToolIconSelected(UIControl *focused);

    virtual void OnSizeChanged(BaseObject * object, void * userData, void * callerData);
	virtual void OnStrengthChanged(BaseObject * object, void * userData, void * callerData);

    
    UITextField *sizeValue;
    UITextField *strengthValue;
    
    UITextField *CreateTextField(const Rect &rect);
    
    UICheckBox *relative;
    UICheckBox *average;
    UICheckBox *absoluteDropper;
    
    UITextField *heightValue;
    
    UIControl *dropperIcon;
    LandscapeTool *dropperTool;

    UIControl *copypasteIcon;
    LandscapeTool *copypasteTool;
    UICheckBox *copyHeightmap;
    UICheckBox *copyTilemask;

    
    UISlider *averageStrength;
    void OnAverageSizeChanged(BaseObject * object, void * userData, void * callerData);

    UICheckBox *showGrid;
    
    float32 prevHeightValue;
};

#endif // __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__