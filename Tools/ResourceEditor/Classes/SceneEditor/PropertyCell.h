/*
 *  PropertyCell.h
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/13/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef PROPERTY_CELL
#define PROPERTY_CELL

#include "DAVAEngine.h"
#include "UICheckBox.h"
#include "ComboBox.h"
#include "EditMatrixControl.h"
#include "ColorPickerDelegate.h"

using namespace DAVA;

class PropertyValue;

class PropertyCellData;
class PropertyCellDelegate
{
public:
    virtual void OnPropertyChanged(PropertyCellData *changedProperty) = 0;
};

class PropertyCell : public UIListCell
{
protected:
    
    enum eConst
    {
        CELL_HEIGHT = 15,
        KEY_NAME_DEVIDER = 2,
    };
    
public:
    
    enum CellType 
    {
        PROP_CELL_TEXT = 0,
        PROP_CELL_DIGITS,
        PROP_CELL_FILEPATH,
        PROP_CELL_BOOL,
        PROP_CELL_COMBO,
        PROP_CELL_MATRIX4,
        PROP_CELL_SECTION,
        PROP_CELL_BUTTON,
        PROP_CELL_COLOR,
        PROP_CELL_SUBSECTION,
        PROP_CELL_SLIDER,
        PROP_CELL_TEXTUREPREVIEW,

        
        PROP_CELL_COUNT
    };
    
    PropertyCell(PropertyCellDelegate *propDelegate, const Rect &rect, PropertyCellData *prop);
    virtual ~PropertyCell();

    virtual void SetData(PropertyCellData *prop);

    static String GetTypeName(int cellType);

    PropertyCellData *property;
    UIStaticText *keyName;
    PropertyCellDelegate *propertyDelegate;
};

class PropertyTextCell : public PropertyCell, public UITextFieldDelegate
{
public:
    PropertyTextCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyTextCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);

    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);
    
    virtual void DidAppear();

    UITextField *editableText;
    UIStaticText *uneditableText;
    UIControl *uneditableTextContainer;
};

class PropertyBoolCell : public PropertyCell, public UICheckBoxDelegate
{
public:
    PropertyBoolCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyBoolCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);

    virtual void ValueChanged(UICheckBox *forCheckbox, bool newValue);
    
protected:    

    UICheckBox *checkBox;
};

class PropertyFilepathCell : public PropertyCell, public UIFileSystemDialogDelegate
{
public:
    PropertyFilepathCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyFilepathCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);
    
    virtual void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

    void OnButton(BaseObject * object, void * userData, void * callerData);
    void OnClear(BaseObject * object, void * userData, void * callerData);

    void OnHint(BaseObject * object, void * userData, void * callerData);

    virtual void Input(UIEvent *currentInput);

    virtual void WillAppear();
    
protected:    

    int32 moveCounter;
//    float32 lastHintTime
    
    UIStaticText *pathText;
    UIControl *pathTextContainer;
    UIButton *browseButton;
    UIButton *clearButton;
    UIFileSystemDialog *dialog;
};

class PropertyComboboxCell: public PropertyCell, public ComboBoxDelegate
{
public:
    
    PropertyComboboxCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyComboboxCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);

    virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex);
    
private:
    
    ComboBox *combo;
};

class PropertyMatrix4Cell: public PropertyCell
{
public:
    
    PropertyMatrix4Cell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyMatrix4Cell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);
    
private:
    
    void OnLocalTransformChanged(BaseObject * object, void * userData, void * callerData);

    EditMatrixControl *matrix;
};

class PropertySectionCell: public PropertyCell
{
public:
    
    PropertySectionCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertySectionCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    void OnButton(BaseObject * object, void * userData, void * callerData);
    
private:

    
};

class PropertyButtonCell: public PropertyCell
{
public:
    
    PropertyButtonCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyButtonCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);

private:
    
    Message buttonEvent;
};

class ColorPicker;
class PropertyColorCell: public PropertyCell, public ColorPickerDelegate
{
public:
    
    PropertyColorCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyColorCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);
    
    virtual void ColorPickerDone(const Color &newColor);

private:

    void OnButtonPressed(BaseObject * owner, void * userData, void * callerData);
    UIControl *colorPreview;
    ColorPicker *colorPicker;
};

class PropertySubsectionCell: public PropertyCell
{
public:
    
    PropertySubsectionCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertySubsectionCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    
private:
    
};

class UISliderWithText;
class PropertySliderCell: public PropertyCell
{
public:
    
    PropertySliderCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertySliderCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);
    
private:
    
    void OnValueChanged(BaseObject * owner, void * userData, void * callerData);

    UIStaticText *minValue;
    UIStaticText *maxValue;
    UISliderWithText *slider;
};



class PropertyTexturePreviewCell: public PropertyCell, public UICheckBoxDelegate
{
public:
    
    PropertyTexturePreviewCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width);
    virtual ~PropertyTexturePreviewCell();
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);

    virtual void ValueChanged(UICheckBox *forCheckbox, bool newValue);

private:
    
    void OnClick(BaseObject * owner, void * userData, void * callerData);
    
    UIControl *previewControl;
    UICheckBox *checkBox;
};



#endif