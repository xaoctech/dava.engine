#ifndef __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__
#define __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeTool.h"
#include "UICheckBox.h"

using namespace DAVA;

class LandscapeToolsPanelHeightmap: 
    public LandscapeToolsPanel, 
    public UITextFieldDelegate,
    public UICheckBoxDelegate
{

public:
    LandscapeToolsPanelHeightmap(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanelHeightmap();
    
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


    UICheckBox *CreateCkeckbox(const Rect &rect, const WideString &text);
    
    UITextField *sizeValue;
    UITextField *strengthValue;
    
    UITextField *CreateTextField(const Rect &rect);
    
    UICheckBox *relative;
    UICheckBox *average;
    
    UITextField *heightValue;
};

#endif // __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__