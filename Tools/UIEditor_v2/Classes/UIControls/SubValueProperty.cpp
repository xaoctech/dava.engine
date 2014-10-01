//
//  SubValueProperty.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#include "SubValueProperty.h"

#include "ValueProperty.h"

using namespace DAVA;

SubValueProperty::SubValueProperty(int index) : index(index)
{
    
}

SubValueProperty::~SubValueProperty()
{
    
}

String SubValueProperty::GetName() const
{
    return GetValueProperty()->GetSubValueName(index);
}

SubValueProperty::ePropertyType SubValueProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType SubValueProperty::GetValue() const
{
    return GetValueProperty()->GetSubValue(index);
}

void SubValueProperty::SetValue(const DAVA::VariantType &newValue)
{
    GetValueProperty()->SetSubValue(index, newValue);
}

bool SubValueProperty::IsReplaced() const
{
    return GetValueProperty()->IsReplaced();
}

ValueProperty *SubValueProperty::GetValueProperty() const
{
    return DynamicTypeCheck<ValueProperty*>(GetParent());
}
