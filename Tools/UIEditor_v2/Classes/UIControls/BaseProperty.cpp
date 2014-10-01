//
//  BaseProperty.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 12.9.14.
//
//

#include "BaseProperty.h"

using namespace DAVA;

BaseProperty::BaseProperty() : parent(NULL)
{
    
}

BaseProperty::~BaseProperty()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        DVASSERT((*it)->parent == this);
        (*it)->Release();
    }
    children.clear();
}

BaseProperty *BaseProperty::GetParent() const
{
    return parent;
}

void BaseProperty::AddProperty(BaseProperty *property)
{
    DVASSERT(property->parent == NULL);
    children.push_back(property);
    property->parent = this;
}

int BaseProperty::GetCount() const
{
    return (int) children.size();
}

BaseProperty *BaseProperty::GetProperty(int index) const
{
    return children[index];
}

int BaseProperty::GetIndex(BaseProperty *property) const
{
    return find(children.begin(), children.end(), property) - children.begin();
}

DAVA::VariantType BaseProperty::GetValue() const
{
    return VariantType();
}

void BaseProperty::SetValue(const DAVA::VariantType &/*newValue*/)
{
     // Do nothing by default
}

const EnumMap *BaseProperty::GetEnumMap() const
{
    return NULL;
}

void BaseProperty::ResetValue()
{
    // Do nothing by default
}

bool BaseProperty::IsReplaced() const
{
    return false; // false by default
}
