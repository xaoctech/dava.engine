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
#include "ColorPicker.h"
#include "PVRConverter.h"
#include "UISliderWithText.h"
#include "HintManager.h"

#pragma mark --PropertyCell 
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
//    keyName->SetText(StringToWString(prop->key) + L" : ");
    keyName->SetText(LocalizedString(StringToWString(prop->key)));
}

String PropertyCell::GetTypeName(int cellType)
{
    return Format("PropCellType%d", cellType);
}


#pragma mark --PropertyTextCell 
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

#pragma mark --PropertyBoolCell 
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


#pragma mark --PropertyFilepathCell 
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
    pathTextContainer->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &PropertyFilepathCell::OnHint));
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
    
    moveCounter = 0;
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

void PropertyFilepathCell::WillAppear()
{
    moveCounter = 0;
    
    UIListCell::WillAppear();
}

void PropertyFilepathCell::Input(UIEvent *currentInput)
{
    if(currentInput->phase == UIEvent::PHASE_MOVE)
    {
        ++moveCounter;
        Logger::Debug("move");
    }
    else 
    {
        moveCounter = 0;
        Logger::Debug("not_move");
    }
    
    UIListCell::Input(currentInput);
}

void PropertyFilepathCell::OnHint(BaseObject * object, void * userData, void * callerData)
{
    HintManager::Instance()->ShowHint(pathText->GetText(), this->GetRect(true));
}

void PropertyFilepathCell::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    String extension = FileSystem::GetExtension(pathToFile);
    if(0 == CompareStrings(".pvr", extension))
    {
        PVRConverter::Instance()->ConvertPvrToPng(pathToFile);
    }
    
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


#pragma mark --PropertyComboboxCell 
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

#pragma mark --PropertyMatrix4Cell 
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


#pragma mark --PropertySectionCell 
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


#pragma mark --PropertyButtonCell 
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

#pragma mark --PropertyColorCell 
PropertyColorCell::PropertyColorCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
:   PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width/2;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);

    colorPicker = new ColorPicker(this);
    
    colorPreview = new UIControl(Rect(keyName->size.x, 0, GetHeightForWidth(width), GetHeightForWidth(width)));
    colorPreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    AddControl(colorPreview);
    
    UIButton *colorButton = ControlsFactory::CreateButton(Rect(keyName->size.x + GetHeightForWidth(width), 0,
                                                               keyName->size.x - GetHeightForWidth(width), GetHeightForWidth(width))
                                                          , LocalizedString(L"property.changecolor"));
    colorButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &PropertyColorCell::OnButtonPressed));
    AddControl(colorButton);
    SafeRelease(colorButton);
    
    SetData(prop);
}

PropertyColorCell::~PropertyColorCell()
{
    SafeRelease(colorPreview);
    SafeRelease(colorPreview);
}

float32 PropertyColorCell::GetHeightForWidth(float32 currentWidth)
{
    return ControlsFactory::BUTTON_HEIGHT;
}

void PropertyColorCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    Color c = prop->GetColor();
    colorPreview->GetBackground()->SetColor(Color(c.r, c.g, c.b, 1.0f));
}

void PropertyColorCell::OnButtonPressed(BaseObject * owner, void * userData, void * callerData)
{
    colorPicker->SetColor(property->GetColor());
    colorPicker->Show();
}

void PropertyColorCell::ColorPickerDone(const Color &newColor)
{
    property->SetColor(newColor);
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
}


#pragma mark --PropertySubsectionCell 
PropertySubsectionCell::PropertySubsectionCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
:   PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    ControlsFactory::CustomizePropertySubsectionCell(this);
    keyName->size.x = width;
    
    SetData(prop);
}

PropertySubsectionCell::~PropertySubsectionCell()
{
}

float32 PropertySubsectionCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT;
}


#pragma mark --PropertySliderCell 
PropertySliderCell::PropertySliderCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)
:   PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
    keyName->size.x = width;
    keyName->size.y = GetHeightForWidth(width)/2;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_LEFT);
    
    float32 textWidth = 50;
    minValue = new UIStaticText(Rect(0, keyName->size.y, textWidth, keyName->size.y));
    minValue->SetFont(ControlsFactory::GetFontLight());
    minValue->SetAlign(ALIGN_VCENTER|ALIGN_RIGHT);
    AddControl(minValue);

    maxValue = new UIStaticText(Rect(width - textWidth, keyName->size.y, textWidth, keyName->size.y));
    maxValue->SetFont(ControlsFactory::GetFontLight());
    maxValue->SetAlign(ALIGN_VCENTER|ALIGN_LEFT);
    AddControl(maxValue);

    slider = new UISliderWithText(Rect(textWidth, keyName->size.y, width - 2*textWidth, keyName->size.y));
    slider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &PropertySliderCell::OnValueChanged));
    slider->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    slider->SetMinDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMinLeftRightStretchCap(5);
    slider->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    slider->SetMaxDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMaxLeftRightStretchCap(5);
    slider->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    AddControl(slider);

    SetData(prop);
}

PropertySliderCell::~PropertySliderCell()
{
    SafeRelease(minValue);
    SafeRelease(maxValue);
    SafeRelease(slider);
}

float32 PropertySliderCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT * 2;
}

void PropertySliderCell::OnValueChanged(DAVA::BaseObject *owner, void *userData, void *callerData)
{
    property->SetSliderValue(slider->GetValue());
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
}

void PropertySliderCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    
    minValue->SetText(Format(L"%f", prop->GetSliderMinValue()));
    maxValue->SetText(Format(L"%f", prop->GetSliderMaxValue()));
    
    slider->SetMinMaxValue(prop->GetSliderMinValue(), prop->GetSliderMaxValue());
    slider->SetValue(prop->GetSliderValue());
}


#pragma mark --PropertyTexturePreviewCell 
PropertyTexturePreviewCell::PropertyTexturePreviewCell(PropertyCellDelegate *propDelegate, PropertyCellData *prop, float32 width)   
    :   PropertyCell(propDelegate, Rect(0, 0, width, GetHeightForWidth(width)), prop)
{
//    keyName->size.x = width/KEY_NAME_DEVIDER;
    keyName->size.x = GetHeightForWidth(width) / 2;
    keyName->size.y = GetHeightForWidth(width) / 2;
    keyName->SetAlign(ALIGN_VCENTER|ALIGN_LEFT);
    keyName->SetVisible(false, false);
    
    float32 checkBoxWidth = GetHeightForWidth(width)/2;
    checkBox = new UICheckBox("~res:/Gfx/UI/chekBox", Rect(0, keyName->size.y, checkBoxWidth, checkBoxWidth));
    checkBox->SetDelegate(this);
    AddControl(checkBox);

    previewControl = new UIControl(Rect(keyName->size.x, 0, width - keyName->size.x, GetHeightForWidth(width)));
    previewControl->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &PropertyTexturePreviewCell::OnClick));
    AddControl(previewControl);
    
    SetData(prop);
}

PropertyTexturePreviewCell::~PropertyTexturePreviewCell()
{
    SafeRelease(previewControl);
    SafeRelease(checkBox);
}

void PropertyTexturePreviewCell::SetData(PropertyCellData *prop)
{
    PropertyCell::SetData(prop);
    
    switch (prop->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_TEXTUREPREVIEW:
        {
            checkBox->SetChecked(prop->GetBool(), false);
            Texture *tex = prop->GetTexture();
            
            if(tex)
            {
                uint32 width = DAVA::Min(tex->width, (uint32)previewControl->GetSize().x);
                uint32 height = DAVA::Min(tex->height, (uint32)previewControl->GetSize().y);
                Sprite *previewSprite = Sprite::CreateFromTexture(tex, 0, 0, width, height);
                
                previewControl->SetSprite(previewSprite, 0);
                
                SafeRelease(previewSprite);
            }
            
            
            break;
        }
            
        default:
            break;
    }
}

float32 PropertyTexturePreviewCell::GetHeightForWidth(float32 currentWidth)
{
    return CELL_HEIGHT * 2;
}

void PropertyTexturePreviewCell::ValueChanged(bool newValue)
{
    property->SetBool(newValue);
    SetData(property);
    propertyDelegate->OnPropertyChanged(property);
}


void PropertyTexturePreviewCell::OnClick(DAVA::BaseObject *owner, void *userData, void *callerData)
{
    bool checked = checkBox->Checked();
    checkBox->SetChecked(!checked, true);
}
