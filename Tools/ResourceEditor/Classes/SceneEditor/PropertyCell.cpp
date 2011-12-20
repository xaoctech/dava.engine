/*
 *  PropertyCell.cpp
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/13/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "PropertyCell.h"
#include "PropertyCellData.h"

PropertyCell::PropertyCell(PropertyCellDelegate *propDelegate, const Rect &rect, Font *font, PropertyCellData *prop)
:UIListCell(rect, GetTypeName(prop->cellType))
{
    background->SetDrawType(UIControlBackground::DRAW_FILL);
    background->SetColor(Color(0.5, 0.5, 0.5, 0.5));
    
    propertyDelegate = propDelegate;

    property = prop;
    keyName = new UIStaticText(Rect(0, 0, size.x, size.y));
    keyName->SetFont(font);
    AddControl(keyName);
}


void PropertyCell::SetData(PropertyCellData *prop)
{
    property = prop;
    property->currentCell = this;
    keyName->SetText(StringToWString(prop->key) + L" :");
}

String PropertyCell::GetTypeName(int cellType)
{
    return Format("PropCellType%d", cellType);
}


PropertyTextCell::PropertyTextCell(PropertyCellDelegate *propDelegate, Font *font, PropertyCellData *prop, float32 width)
: PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), font, prop)
{
    keyName->size.x = width/2;
  
    editableText = new UITextField(Rect(width/2 + 5, 0, width/2 - 10, size.y));
    editableText->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    editableText->GetBackground()->SetColor(Color(0.2, 0.2, 0.2, 0.6));
    editableText->SetFont(font);
    editableText->SetDelegate(this);
    
    uneditableTextContainer = new UIControl(Rect(width/2 + 5, 0, width/2 - 10, size.y));
    uneditableTextContainer->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    uneditableTextContainer->GetBackground()->SetColor(Color(0.4, 0.4, 0.4, 0.5));
    uneditableText = new UIStaticText(Rect(0, 0, uneditableTextContainer->size.x, uneditableTextContainer->size.y));
    uneditableText->SetFont(font);
    uneditableTextContainer->AddControl(uneditableText);
    
    SetData(prop);
}

void PropertyTextCell::DidAppear()
{
    editableText->ReleaseFocus();
}

void PropertyTextCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);

    switch (prop->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
            editableText->SetText(StringToWString(prop->GetString()));
            uneditableText->SetText(StringToWString(prop->GetString()));
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
            editableText->SetText( StringToWString( Format("%d", prop->GetInt()) ) );
            uneditableText->SetText( StringToWString( Format("%d", prop->GetInt()) ) );
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
            editableText->SetText( StringToWString( Format("%.3f", prop->GetFloat()) ) );
            uneditableText->SetText( StringToWString( Format("%.3f", prop->GetFloat()) ) );
            break;
    }
    
    if (prop->isEditable) 
    {
        if (!editableText->GetParent()) 
        {
            RemoveControl(uneditableTextContainer);
            AddControl(editableText);
            editableText->ReleaseFocus();
        }
    }
    else 
    {
        if (!uneditableTextContainer->GetParent()) 
        {
            RemoveControl(editableText);
            AddControl(uneditableTextContainer);
            editableText->ReleaseFocus();
        }
    }
    
}

void PropertyTextCell::TextFieldShouldReturn(UITextField * textField)
{
    editableText->ReleaseFocus();
    switch (property->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
            property->SetString(WStringToString(editableText->GetText()));
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
            property->SetInt(atoi(WStringToString(editableText->GetText()).c_str()));
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
            property->SetFloat(atof(WStringToString(editableText->GetText()).c_str()));
            break;
    }
    propertyDelegate->OnPropertyChanged(property);
};

bool PropertyTextCell::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (replacementLength < 0) 
    {
        return true;
    }
    switch (property->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
            return true;
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
        {
            WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
            bool allOk;
            for (int i = 0; i < newText.length(); i++) 
            {
                allOk = false;
                if (newText[i] == L'-' && i == 0)
                {
                    allOk = true;
                }
                else if(newText[i] >= L'0' && newText[i] <= L'9')
                {
                    allOk = true;
                }
                if (!allOk) 
                {
                    return false;
                }
            }
        }
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
        {
            WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
            bool allOk;
            int pointsCount = 0;
            for (int i = 0; i < newText.length(); i++) 
            {
                allOk = false;
                if (newText[i] == L'-' && i == 0)
                {
                    allOk = true;
                }
                else if(newText[i] >= L'0' && newText[i] <= L'9')
                {
                    allOk = true;
                }
                else if(newText[i] == L'.' && pointsCount == 0)
                {
                    allOk = true;
                    pointsCount++;
                }
                if (!allOk) 
                {
                    return false;
                }
            }
        }
            break;
    }
    return false;
};

float32 PropertyTextCell::GetHeightForWidth(float32 currentWidth)
{
    return 30.f;
}
