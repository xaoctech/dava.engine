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

}

PropertyList::~PropertyList()
{
    ReleaseProperties();
    
    SafeRelease(propsList);
}

void PropertyList::AddTextProperty(const String &propertyName, const String &currentText, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_STRING);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetString(currentText);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddIntProperty(const String &propertyName, int32 currentIntValue, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_INTEGER);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetInt(currentIntValue);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddFloatProperty(const String &propertyName, float32 currentFloatValue, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_FLOAT);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetFloat(currentFloatValue);
    AddProperty(p, propertyName, propEditType);
}


void PropertyList::AddFilepathProperty(const String &propertyName, const String &currentFilepath, editableType propEditType)
{
}



void PropertyList::AddProperty(PropertyCellData *newProp, const String &propertyName, editableType propEditType)
{
    newProp->key = propertyName;
    newProp->isEditable = propEditType == PROPERTY_IS_EDITABLE;
    newProp->index = props.size();
    props.push_back(newProp);
    propsMap[propertyName] = newProp;
    propsList->Refresh();
}

void PropertyList::SetTextPropertyValue(const String &propertyName, const String &newText)
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

PropertyCellData *PropertyList::PropertyByName(const String &propertyName)
{
	Map<String, PropertyCellData*>::const_iterator it;
	it = propsMap.find(propertyName);
    
    DVASSERT(it != propsMap.end());
    
    return it->second;
}

void PropertyList::OnPropertyChanged(PropertyCellData *changedProperty)
{
    switch (changedProperty->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
            delegate->OnStringPropertyChanged(this, changedProperty->key, changedProperty->GetString());
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
            delegate->OnIntPropertyChanged(this, changedProperty->key, changedProperty->GetInt());
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
            delegate->OnFloatPropertyChanged(this, changedProperty->key, changedProperty->GetFloat());
            break;
    }
}



int32 PropertyList::ElementsCount(UIList *forList)
{
    return props.size();
}

UIListCell *PropertyList::CellAtIndex(UIList *forList, int32 index)
{
    PropertyCell *c = (PropertyCell *)forList->GetReusableCell(PropertyCell::GetTypeName(props[index]->cellType));
    if (!c) 
    {
        switch (props[index]->cellType) 
        {
            case PropertyCell::PROP_CELL_TEXT:
                c = new PropertyTextCell(this, props[index], size.x);
                break;
        }
    }
    else 
    {
        c->SetData(props[index]);
    }


    
    return c;
//                               //fill cell whith data
//                               c->serverName = GameServer::Instance()->totalServers[index].name + LocalizedString("'s game");
//                               c->SetStateText(UIControl::STATE_NORMAL, c->serverName, Vector2(c->GetStateBackground(UIControl::STATE_NORMAL)->GetSprite()->GetWidth() * 1.7 - 30, 0));
//                               c->connection = GameServer::Instance()->totalServers[index].connection;
//                               c->serverIndex = GameServer::Instance()->totalServers[index].index;
//                               
//                               return c;//returns cell
//                               //your application don't need to manage cells. UIList do all cells management.
//                               //you can create cells of your own types derived from the UIListCell
//                               Format("PropCellType%d", cellType)
}

int32 PropertyList::CellHeight(UIList *forList, int32 index)
{
    switch (props[index]->cellType) 
    {
        case PropertyCell::PROP_CELL_TEXT:
            return PropertyTextCell::GetHeightForWidth(size.x);
            break;
    }
    return 50;//todo: rework
}

void PropertyList::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
}

void PropertyList::AddPropertyByData(PropertyCellData *newProp)
{
    newProp->index = props.size();
    props.push_back(SafeRetain(newProp));
    propsMap[newProp->key] = newProp;
    propsList->Refresh();
}

void PropertyList::ReleaseProperties()
{
	propsMap.clear();
    
    for (int32 i = 0; i < props.size(); ++i)
    {
        SafeRelease(props[i]);
    }
    props.clear();
    
    propsList->Refresh();
}
