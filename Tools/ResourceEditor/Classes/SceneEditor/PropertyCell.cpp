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
#include "EditorSettings.h"

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
    keyName->size.x = width/KEY_NAME_DEVIDER;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);
  
    float32 activeWidth = width - keyName->size.x;
    
    Font * font = ControlsFactory::GetFontLight();
    editableText = new UITextField(Rect(keyName->size.x, 0, activeWidth, size.y));
    ControlsFactory::CustomizeEditablePropertyCell(editableText);
    editableText->SetFont(font);
    editableText->SetDelegate(this);
    
    uneditableTextContainer = new UIControl(Rect( keyName->size.x, 0, activeWidth, size.y));
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
    keyName->size.x = width/KEY_NAME_DEVIDER;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);

    float32 checkBoxWidth = GetHeightForWidth(width - keyName->size.x);
    checkBox = new UICheckBox("~res:/Gfx/UI/chekBox", Rect(keyName->size.x, 0, checkBoxWidth, checkBoxWidth));
    checkBox->SetDelegate(this);
    AddControl(checkBox);
    
    SetData(prop);
}

PropertyBoolCell::~PropertyBoolCell()
{
    SafeRelease(checkBox);
}

void PropertyBoolCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    
    switch (prop->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_BOOL:
            checkBox->SetChecked(prop->GetBool(), false);
            break;
            
        default:
            break;
    }
}

float32 PropertyBoolCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT;
}

void PropertyBoolCell::ValueChanged(bool newValue)
{
    property->SetBool(newValue);
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
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
    
    bool clearDataEnabled = prop->GetClearDataEnabled();
    float32 xOffset = (clearDataEnabled) ? (size.y/2 + 5.0f) : 0.0f;
    
    pathTextContainer = new UIControl(Rect(2, size.y/2, size.x - size.y/2 - 4 - xOffset, size.y/2));
    ControlsFactory::CustomizeEditablePropertyCell(pathTextContainer);
    pathText = new UIStaticText(Rect(0, 0, pathTextContainer->size.x, pathTextContainer->size.y));
    pathText->SetFont(font);
    pathText->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);
    pathTextContainer->AddControl(pathText);
    AddControl(pathTextContainer);
    
    browseButton = ControlsFactory::CreateButton(Rect(size.x - size.y/2 - xOffset, size.y/2, size.y/2, size.y/2), L"...");
	browseButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &PropertyFilepathCell::OnButton));
    AddControl(browseButton);

    if(clearDataEnabled)
    {
        clearButton = ControlsFactory::CreateButton(Rect(size.x - size.y/2, size.y/2, size.y/2, size.y/2), L"X");
        ControlsFactory::CustomizeCloseWindowButton(clearButton);
        clearButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &PropertyFilepathCell::OnClear));
        AddControl(clearButton);
    }
    else
    {
        clearButton = NULL;
    }
    
    SetData(prop);
}

PropertyFilepathCell::~PropertyFilepathCell()
{
    SafeRelease(clearButton);
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
    
    String fullpath = prop->GetString();
    String datasourcePath = EditorSettings::Instance()->GetDataSourcePath();
    int32 pos = fullpath.find(datasourcePath);
    if(String::npos == pos)
    {
        pathText->SetText(StringToWString(prop->GetString()));
    }
    else
    {
        fullpath = fullpath.substr(datasourcePath.length());
        pathText->SetText(StringToWString(fullpath));
    }
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
    if(property->GetString().length() > 0)
    {
        dialog->SetCurrentDir(property->GetString());
    }
    else 
    {
        dialog->SetCurrentDir(EditorSettings::Instance()->GetDataSourcePath());
    }

    
    dialog->Show(UIScreenManager::Instance()->GetScreen());
}

void PropertyFilepathCell::OnClear(BaseObject * object, void * userData, void * callerData)
{
    if(dialog) 
    {
        return;
    }

    property->SetString("");
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
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
    keyName->size.x = width/KEY_NAME_DEVIDER;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);

    combo = new ComboBox(Rect(keyName->size.x, 0, width - keyName->size.x, GetHeightForWidth(width)), this, prop->GetStringVector());
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
    combo->SetSelectedIndex(property->GetItemIndex(), false);
    propertyDelegate->OnPropertyChanged(property);
}

//*************** PropertyMatrix4Cell **************
PropertyMatrix4Cell::PropertyMatrix4Cell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
:       PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = size.x;
    keyName->size.y = CELL_HEIGHT;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_LEFT);
    
    matrix = new EditMatrixControl(Rect(0, CELL_HEIGHT, size.x, GetHeightForWidth(width) - CELL_HEIGHT));
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
    return CELL_HEIGHT * 5;
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
PropertySectionCell::PropertySectionCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
    :   PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    ControlsFactory::CustomizePropertySectionCell(this);
    keyName->size.x = width;

	AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &PropertySectionCell::OnButton));

    SetData(prop);
}

PropertySectionCell::~PropertySectionCell()
{
}

float32 PropertySectionCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT;
}

void PropertySectionCell::OnButton(BaseObject * object, void * userData, void * callerData)
{
    property->SetIsSectionOpened(!property->GetIsSectionOpened());
    propertyDelegate->OnPropertyChanged(property);
}


//*************** PropertyButtonCell **************
PropertyButtonCell::PropertyButtonCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
:   PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width;
    
    buttonEvent = prop->GetMessage(); 
	AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, buttonEvent);

    SetData(prop);
}

PropertyButtonCell::~PropertyButtonCell()
{
}

float32 PropertyButtonCell::GetHeightForWidth(float32 currentWidth)
{
    return ControlsFactory::BUTTON_HEIGHT;
}

void PropertyButtonCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);

    ControlsFactory::CustomizePropertyButtonCell(this);
    
    RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, buttonEvent);
    buttonEvent = prop->GetMessage(); 
    AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, buttonEvent);
}
