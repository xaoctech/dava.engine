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
public:
    
    enum CellType 
    {
        PROP_CELL_TEXT = 0,
        PROP_CELL_DIGITS,
        PROP_CELL_FILEPATH,
        
        PROP_CELL_COUNT
    };
    
    PropertyCell(PropertyCellDelegate *propDelegate, const Rect &rect, PropertyCellData *prop);

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
    
    static float32 GetHeightForWidth(float32 currentWidth);
    virtual void SetData(PropertyCellData *prop);

    virtual void TextFieldShouldReturn(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);
    
    virtual void DidAppear();

    UITextField *editableText;
    UIStaticText *uneditableText;
    UIControl *uneditableTextContainer;
};

#endif