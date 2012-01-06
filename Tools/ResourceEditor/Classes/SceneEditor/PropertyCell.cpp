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
#include "ControlsFactory.h"

PropertyCell::PropertyCell(PropertyCellDelegate *propDelegate, const Rect &rect, PropertyCellData *prop)
:UIListCell(rect, GetTypeName(prop->cellType))
{
    ControlsFactory::CustomizePropertyCell(this, false);
    
    propertyDelegate = propDelegate;

    property = prop;
    keyName = new UIStaticText(Rect(0, 0, size.x, size.y));
    keyName->SetFont(ControlsFactory::GetFontLight());
    AddControl(keyName);
}

PropertyCell::~PropertyCell()
{
    SafeRelease(keyName);
}


void PropertyCell::SetData(PropertyCellData *prop)
{
    property = prop;
    property->currentCell = this;
    keyName->SetText(StringToWString(prop->key) + L" : ");
}

String PropertyCell::GetTypeName(int cellType)
{
    return Format("PropCellType%d", cellType);
}


//********************* PropertyTextCell *********************
PropertyTextCell::PropertyTextCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
: PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width/2;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);
  
    Font * font = ControlsFactory::GetFontLight();
    editableText = new UITextField(Rect(width/2, 0, width/2, size.y));
    ControlsFactory::CustomizeEditablePropertyCell(editableText);
    editableText->SetFont(font);
    editableText->SetDelegate(this);
    
    uneditableTextContainer = new UIControl(Rect(width/2, 0, width/2, size.y));
    ControlsFactory::CustomizeUneditablePropertyCell(uneditableTextContainer);
    uneditableText = new UIStaticText(Rect(0, 0, uneditableTextContainer->size.x, uneditableTextContainer->size.y));
    uneditableText->SetFont(font);
    uneditableTextContainer->AddControl(uneditableText);
    
    SetData(prop);
}

PropertyTextCell::~PropertyTextCell()
{
    SafeRelease(editableText);
    SafeRelease(uneditableTextContainer);
    SafeRelease(uneditableText);
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
}

void PropertyTextCell::TextFieldShouldCancel(UITextField * textField)
{
    SetData(property);
    editableText->ReleaseFocus();
}

void PropertyTextCell::TextFieldLostFocus(UITextField * textField)
{
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
}

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
            return true;
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
            return true;
        }
            break;
    }
    return false;
};

float32 PropertyTextCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT;
}

//********************* PropertyBoolCell *********************
PropertyBoolCell::PropertyBoolCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
: PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width/2;
    
    float32 usedWidth = keyName->size.x;
    float32 checkBoxWidth = GetHeightForWidth(usedWidth);
    
    textContainer = new UIControl(Rect(usedWidth + checkBoxWidth, 0, usedWidth - checkBoxWidth, checkBoxWidth));
    ControlsFactory::CustomizeUneditablePropertyCell(textContainer);
    AddControl(textContainer);
    
    Font * font = ControlsFactory::GetFontLight();
    falseText = new UIStaticText(Rect(0, 0, usedWidth - checkBoxWidth, checkBoxWidth));
    falseText->SetText(L"False");
    falseText->SetFont(font);
    
    trueText = new UIStaticText(Rect(0, 0, usedWidth - checkBoxWidth, checkBoxWidth));
    trueText->SetText(L"True");
    trueText->SetFont(font);
    
    checkBox = new UICheckBox("~res:/Gfx/UI/chekBox", Rect(usedWidth, 0, checkBoxWidth, checkBoxWidth));
    checkBox->SetDelegate(this);
    AddControl(checkBox);
    
    SetData(prop);
}

PropertyBoolCell::~PropertyBoolCell()
{
    SafeRelease(textContainer);
    SafeRelease(checkBox);
    SafeRelease(falseText);
    SafeRelease(trueText);
}

void PropertyBoolCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    
    switch (prop->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_BOOL:
            checkBox->SetChecked(prop->GetBool(), false);
            UpdateText();
            break;
            
        default:
            break;
    }
}

float32 PropertyBoolCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT;
}

void PropertyBoolCell::UpdateText()
{
    if(property->GetBool())
    {
        if(falseText->GetParent())
        {
            textContainer->RemoveControl(falseText);
        }
        
        if(!trueText->GetParent())
        {
            textContainer->AddControl(trueText);
        }
    }
    else
    {
        if(trueText->GetParent())
        {
            textContainer->RemoveControl(trueText);
        }
        
        if(!falseText->GetParent())
        {
            textContainer->AddControl(falseText);
        }
    }
}

void PropertyBoolCell::ValueChanged(bool newValue)
{
    property->SetBool(newValue);
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);

    UpdateText();
    
//    if(newValue)
//    {
//        if(falseText->GetParent())
//        {
//            textContainer->RemoveControl(falseText);
//        }
//        
//        if(!trueText->GetParent())
//        {
//            textContainer->AddControl(trueText);
//        }
//    }
//    else
//    {
//        if(trueText->GetParent())
//        {
//            textContainer->RemoveControl(trueText);
//        }
//        
//        if(!falseText->GetParent())
//        {
//            textContainer->AddControl(falseText);
//        }
//    }
}


//********************* PropertyFilepathCell *********************
PropertyFilepathCell::PropertyFilepathCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
: PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    dialog = NULL;
    
    keyName->size.x = size.x;
    keyName->size.y = size.y/2;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_LEFT);
    
    Font * font = ControlsFactory::GetFontLight();
    
    pathTextContainer = new UIControl(Rect(2, size.y/2, size.x - size.y/2 - 4, size.y/2));
    ControlsFactory::CustomizeEditablePropertyCell(pathTextContainer);
    pathText = new UIStaticText(Rect(0, 0, pathTextContainer->size.x, pathTextContainer->size.y));
    pathText->SetFont(font);
    pathText->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);
    pathTextContainer->AddControl(pathText);
    AddControl(pathTextContainer);
    
    browseButton = ControlsFactory::CreateButton(Rect(size.x - size.y/2, size.y/2, size.y/2, size.y/2), L"...");
	browseButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &PropertyFilepathCell::OnButton));
    AddControl(browseButton);
    
    SetData(prop);
}

PropertyFilepathCell::~PropertyFilepathCell()
{
    SafeRelease(browseButton);
    SafeRelease(pathText);
    SafeRelease(pathTextContainer);
}

float32 PropertyFilepathCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT * 2;
}

void PropertyFilepathCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    pathText->SetText(StringToWString(prop->GetString()));
}

void PropertyFilepathCell::OnButton(BaseObject * object, void * userData, void * callerData)
{
    if(dialog) 
    {
        return;
    }
    dialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    dialog->SetOperationType(UIFileSystemDialog::OPERATION_LOAD);
    dialog->SetDelegate(this);
    dialog->SetTitle(keyName->GetText());
    dialog->SetExtensionFilter(property->GetExtensionFilter());
    String p, f;
    FileSystem::SplitPath(property->GetString(), p, f);
    dialog->SetCurrentDir(p);
    
    dialog->Show(UIScreenManager::Instance()->GetScreen());
}

void PropertyFilepathCell::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    property->SetString(pathToFile);
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
    SafeRelease(dialog);
}

void PropertyFilepathCell::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    SafeRelease(dialog);
}



//void PropertyFilepathCell::DidAppear()
//{
//}


//*************** PropertyComboboxCell **************
PropertyComboboxCell::PropertyComboboxCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
    :       PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width/2;
    
    float32 usedWidth = keyName->size.x;
//    Vector<String> empty;
//    empty.push_back("Empty combo");
//    combo = new ComboBox(Rect(usedWidth, 0, usedWidth, GetHeightForWidth(width)), this, empty);
    combo = new ComboBox(Rect(usedWidth, 0, usedWidth, GetHeightForWidth(width)), this, prop->GetStringVector());
    AddControl(combo);
    SetData(prop);
}

PropertyComboboxCell::~PropertyComboboxCell()
{
    SafeRelease(combo);
}

float32 PropertyComboboxCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT;
}

void PropertyComboboxCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    
    combo->SetNewItemsSet(prop->GetStringVector());
    combo->SetSelectedIndex(prop->GetItemIndex(), false);
}

void PropertyComboboxCell::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
    property->SetItemIndex(itemIndex);
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
}

//*************** PropertyMatrix4Cell **************
PropertyMatrix4Cell::PropertyMatrix4Cell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
:       PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width/2;
    
    float32 usedWidth = keyName->size.x;

    matrix = new EditMatrixControl(Rect(usedWidth, 0, usedWidth, GetHeightForWidth(width)));
    matrix->OnMatrixChanged = Message(this, &PropertyMatrix4Cell::OnLocalTransformChanged);

    AddControl(matrix);
    SetData(prop);
}

PropertyMatrix4Cell::~PropertyMatrix4Cell()
{
    SafeRelease(matrix);
}

float32 PropertyMatrix4Cell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT * 4;
}

void PropertyMatrix4Cell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    
    switch (prop->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_MATRIX4:
            matrix->SetMatrix(prop->GetMatrix4());
            matrix->SetReadOnly(!prop->isEditable);
            break;
            
        default:
            break;
    }
}

void PropertyMatrix4Cell::OnLocalTransformChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    property->SetMatrix4(matrix->GetMatrix());
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
}


//*************** PropertySectionHeaderCell **************
PropertySectionHeaderCell::PropertySectionHeaderCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
    :   PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    ControlsFactory::CustomizePropertySectionCell(this);
    keyName->size.x = width;

    SetData(prop);
}

PropertySectionHeaderCell::~PropertySectionHeaderCell()
{
}

float32 PropertySectionHeaderCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT;
}

void PropertySectionHeaderCell::ToggleExpand()
{
    property->SetBool(!property->GetBool());
    SetData(property);
}

