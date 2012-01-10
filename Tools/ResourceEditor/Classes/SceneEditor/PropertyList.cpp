/*
 *  PropertyList.cpp
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/13/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "PropertyList.h"

#include "PropertyCell.h"


PropertyList::PropertyList(const Rect &rect, PropertyListDelegate *propertiesDelegate)
:UIControl(rect)
{
    delegate = propertiesDelegate;
    background->SetDrawType(UIControlBackground::DRAW_FILL);
    background->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));

    propsList = new UIList(Rect(0,0, size.x, size.y), UIList::ORIENTATION_VERTICAL);
    propsList->SetDelegate(this);
    AddControl(propsList);
    currentSection = NULL;
}

PropertyList::~PropertyList()
{
    ReleaseProperties();
    
    SafeRelease(propsList);
}

void PropertyList::AddStringProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_STRING);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetString("");
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddIntProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_INTEGER);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetInt(0);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddFloatProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_FLOAT);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetFloat(0.f);
    AddProperty(p, propertyName, propEditType);
}


void PropertyList::AddFilepathProperty(const String &propertyName, const String &extensionFilter, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_STRING);
    p->cellType = PropertyCell::PROP_CELL_FILEPATH;
    p->SetString("/");
    p->SetExtensionFilter(extensionFilter);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddBoolProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_BOOL);
    p->cellType = PropertyCell::PROP_CELL_BOOL;
    p->SetBool(false);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddProperty(PropertyCellData *newProp, const String &propertyName, editableType propEditType)
{
    newProp->key = propertyName;
    newProp->isEditable = propEditType == PROPERTY_IS_EDITABLE;
    newProp->index = props.size();
    props.push_back(newProp);
    propsMap[propertyName] = newProp;
    if (currentSection)
    {
        currentSection->SetSectionElementsCount(currentSection->GetSectionElementsCount() + 1);
    }
    propsList->Refresh();
}

void PropertyList::SetStringPropertyValue(const String &propertyName, const String &newText)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetString(newText);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetIntPropertyValue(const String &propertyName, int32 newIntValue)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetInt(newIntValue);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetFloatPropertyValue(const String &propertyName, float32 newFloatValue)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetFloat(newFloatValue);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetFilepathPropertyValue(const String &propertyName, const String &currentFilepath)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetString(currentFilepath);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetBoolPropertyValue(const String &propertyName, bool newBoolValue)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetBool(newBoolValue);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}


const String &PropertyList::GetStringPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetString();   
}

int32 PropertyList::GetIntPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetInt();   
}

float32 PropertyList::GetFloatPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetFloat();   
}

const String &PropertyList::GetFilepathPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetString();   
}

bool PropertyList::GetBoolPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetBool();   
}

bool PropertyList::IsPropertyAvaliable(const String &propertyName)
{
	Map<String, PropertyCellData*>::const_iterator it;
	it = propsMap.find(propertyName);
    
    return it != propsMap.end();
}


PropertyCellData *PropertyList::PropertyByName(const String &propertyName)
{
	Map<String, PropertyCellData*>::const_iterator it;
	it = propsMap.find(propertyName);
    
    DVASSERT(it != propsMap.end());
    
    return it->second;
}

void PropertyList::OnPropertyChanged(PropertyCellData *changedProperty)
{
    //ADD Combobox
    switch (changedProperty->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
        {
            switch (changedProperty->cellType)
            {
                case PropertyCell::PROP_CELL_TEXT:
                    delegate->OnStringPropertyChanged(this, changedProperty->key, changedProperty->GetString());
                    break;
                case PropertyCell::PROP_CELL_FILEPATH:
                    delegate->OnFilepathPropertyChanged(this, changedProperty->key, changedProperty->GetString());
                    break;
            }
        }
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
            delegate->OnIntPropertyChanged(this, changedProperty->key, changedProperty->GetInt());
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
            delegate->OnFloatPropertyChanged(this, changedProperty->key, changedProperty->GetFloat());
            break;
        case PropertyCellData::PROP_VALUE_BOOL:
            delegate->OnBoolPropertyChanged(this, changedProperty->key, changedProperty->GetBool());
            break;
        case PropertyCellData::PROP_VALUE_COMBO_BOX:
            delegate->OnComboIndexChanged(this, changedProperty->key, changedProperty->GetItemIndex(), changedProperty->GetStringVector()[changedProperty->GetItemIndex()]);
            break;
        case PropertyCellData::PROP_VALUE_MATRIX4:
            delegate->OnMatrix4Changed(this, changedProperty->key, changedProperty->GetMatrix4());
            break;
        case PropertyCellData::PROP_VALUE_SECTION:
            propsList->Refresh();
            break;
    }
}



int32 PropertyList::ElementsCount(UIList *forList)
{
    int32 count = 0;
    for (int32 i = 0; i < props.size(); i++) 
    {
        if (props[i]->GetValueType() == PropertyCellData::PROP_VALUE_SECTION)
        {
            if (props[i]->GetIsSectionOpened())
            {
                count += props[i]->GetSectionElementsCount();
            }
            else 
            {
                count++;
            }

            i += props[i]->GetSectionElementsCount()-1;
        }
        else 
        {
            count++;
        }
    }
    return count;
}

int32 PropertyList::GetRealIndex(int32 index)
{
    int32 realIndex = 0;
    for (int32 i = 0; i <= index; i++) 
    {
        if (realIndex > 0 && props[realIndex - 1]->GetValueType() == PropertyCellData::PROP_VALUE_SECTION)
        {
            if (!props[realIndex - 1]->GetIsSectionOpened())
            {
                realIndex += props[realIndex - 1]->GetSectionElementsCount();
            }
            else 
            {
                realIndex++;
            }
        }
        else 
        {
            realIndex++;
        }
    }
    return realIndex - 1;
}

UIListCell *PropertyList::CellAtIndex(UIList *forList, int32 index)
{
    
    index = GetRealIndex(index);
    
    PropertyCell *c = (PropertyCell *)forList->GetReusableCell(PropertyCell::GetTypeName(props[index]->cellType));
    if (!c) 
    {
        switch (props[index]->cellType) 
        {
            case PropertyCell::PROP_CELL_TEXT:
                c = new PropertyTextCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_FILEPATH:
                c = new PropertyFilepathCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_BOOL:
                c = new PropertyBoolCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_COMBO:
                c = new PropertyComboboxCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_MATRIX4:
                c = new PropertyMatrix4Cell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_SECTION:
                c = new PropertySectionCell(this, props[index], size.x);
                break;
        }
    }
    else 
    {
        c->SetData(props[index]);
    }


    
    return c;
}

int32 PropertyList::CellHeight(UIList *forList, int32 index)
{
    index = GetRealIndex(index);
    switch (props[index]->cellType) 
    {
        case PropertyCell::PROP_CELL_TEXT:
            return PropertyTextCell::GetHeightForWidth(size.x);
            break;
        case PropertyCell::PROP_CELL_FILEPATH:
            return PropertyFilepathCell::GetHeightForWidth(size.x);
            break;
        case PropertyCell::PROP_CELL_BOOL:
            return PropertyBoolCell::GetHeightForWidth(size.x);
            break;
        case PropertyCell::PROP_CELL_COMBO:
            return PropertyComboboxCell::GetHeightForWidth(size.x);
            break;
        case PropertyCell::PROP_CELL_MATRIX4:
            return PropertyMatrix4Cell::GetHeightForWidth(size.x);
            break;
        case PropertyCell::PROP_CELL_SECTION:
            return PropertySectionCell::GetHeightForWidth(size.x);
            break;
    }
    return 50;//todo: rework
}

void PropertyList::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
//    PropertySectionHeaderCell *sectionHeader = dynamic_cast<PropertySectionHeaderCell *> (selectedCell);
//    if(sectionHeader)
//    {
//        sectionHeader->ToggleExpand();
//        forList->Refresh();
//    }
}

//void PropertyList::AddPropertyByData(PropertyCellData *newProp)
//{
//    newProp->index = props.size();
//    props.push_back(SafeRetain(newProp));
//    propsMap[newProp->key] = newProp;
//    propsList->Refresh();
//}

void PropertyList::ReleaseProperties()
{
	propsMap.clear();
    
    for (int32 i = 0; i < props.size(); ++i)
    {
        SafeRelease(props[i]);
    }
    props.clear();
    
    propsList->Refresh();
    currentSection = NULL;
}


void PropertyList::AddComboProperty(const String &propertyName, const Vector<String> &strings, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_COMBO_BOX);
    p->cellType = PropertyCell::PROP_CELL_COMBO;
    p->SetStringVector(strings);
    p->SetItemIndex(0);
    AddProperty(p, propertyName, propEditType);
}


void PropertyList::SetComboPropertyStrings(const String &propertyName, const Vector<String> &strings)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetStringVector(strings);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetComboPropertyIndex(const String &propertyName, int32 currentStringIndex)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetItemIndex(currentStringIndex);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

const String &PropertyList::GetComboPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetStringVector()[p->GetItemIndex()];
}

const int32 PropertyList::GetComboPropertyIndex(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetItemIndex();
}

void PropertyList::AddMatrix4Property(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_MATRIX4);
    p->cellType = PropertyCell::PROP_CELL_MATRIX4;
    p->SetMatrix4(Matrix4());
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::SetMatrix4PropertyValue(const String &propertyName, const Matrix4 &currentMatrix)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetMatrix4(currentMatrix);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

const Matrix4 & PropertyList::GetMatrix4PropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetMatrix4();
}

void PropertyList::AddSection(const String &sectionName, bool expanded)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_SECTION);
    p->cellType = PropertyCell::PROP_CELL_SECTION;
    p->SetIsSectionOpened(expanded);
    currentSection = p;
    AddProperty(p, sectionName, PROPERTY_IS_EDITABLE);
}

void PropertyList::SetSectionIsOpened(const String &sectionName, bool isOpened)
{
    PropertyCellData *p = PropertyByName(sectionName);
    p->SetIsSectionOpened(isOpened);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
    propsList->Refresh();
}

bool PropertyList::GetSectionIsOpened(const String &sectionName)
{
    PropertyCellData *p = PropertyByName(sectionName);
    return p->GetIsSectionOpened();
}



