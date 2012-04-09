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
    enum eLocalConst
    {
        TEXTFIELD_WIDTH = 50,
        TEXT_WIDTH = 100
    };

    
public:
    LandscapeToolsPanelHeightmap(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanelHeightmap();
    
    virtual void WillAppear();
    
    //UITextFieldDelegate
    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, 
                                     int32 replacementLength, const WideString & replacementString);

    //UICheckBoxDelegate
    virtual void ValueChanged(UICheckBox *forCheckbox, bool newValue);

    
protected:

	void OnToolSelected(BaseObject * object, void * userData, void * callerData);

    UICheckBox *CreateCkeckbox(const Rect &rect, const WideString &text);
    
    UIControl *toolButtons[LandscapeTool::EBT_COUNT_COLOR];
    LandscapeTool *tools[LandscapeTool::EBT_COUNT_COLOR];

    UISlider *sizeSlider;
    UITextField *sizeValue;
    UISlider *strengthSlider;
    UITextField *strengthValue;
	void OnSizeChanged(BaseObject * object, void * userData, void * callerData);
	void OnStrengthChanged(BaseObject * object, void * userData, void * callerData);
    
    UITextField *CreateTextField(const Rect &rect);
    
    UICheckBox *relative;
    UICheckBox *average;
};

#endif // __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__