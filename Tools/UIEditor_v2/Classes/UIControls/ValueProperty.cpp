//
//  ValueProperty.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#include "ValueProperty.h"

#include "SubValueProperty.h"

using namespace DAVA;

ValueProperty::ValueProperty(BaseObject *object, const InspMember *member) : object(NULL), member(member), replaced(false)
{
    this->object = SafeRetain(object);
    defaultValue = member->Value(object);
    
    if (defaultValue.GetType() == VariantType::TYPE_VECTOR2)
    {
        AddProperty(new SubValueProperty(0));
        AddProperty(new SubValueProperty(1));
    }
    if (defaultValue.GetType() == VariantType::TYPE_COLOR)
    {
        AddProperty(new SubValueProperty(0));
        AddProperty(new SubValueProperty(1));
        AddProperty(new SubValueProperty(2));
        AddProperty(new SubValueProperty(3));
    }
}

ValueProperty::~ValueProperty()
{
    SafeRelease(object);
}

String ValueProperty::GetName() const
{
    return member->Desc().text;
}

ValueProperty::ePropertyType ValueProperty::GetType() const
{
    if (member->Desc().type == InspDesc::T_ENUM)
        return TYPE_ENUM;
    else if (member->Desc().type == InspDesc::T_FLAGS)
        return TYPE_FLAGS;
    return TYPE_VARIANT;
}

VariantType ValueProperty::GetValue() const
{
    return member->Value(object);
}

void ValueProperty::SetValue(const DAVA::VariantType &newValue)
{
    replaced = true;
    member->SetValue(object, newValue);
}

const EnumMap *ValueProperty::GetEnumMap() const
{
    if (member->Desc().type == InspDesc::T_ENUM)
        return member->Desc().enumMap;
    else if (member->Desc().type == InspDesc::T_FLAGS)
        return member->Desc().enumMap;
    return NULL;
}

void ValueProperty::ResetValue()
{
    replaced = false;
    member->SetValue(object, defaultValue);
}

bool ValueProperty::IsReplaced() const
{
    return replaced;
}

String ValueProperty::GetSubValueName(int index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
            return index == 0 ? "X" : "Y";

        case VariantType::TYPE_COLOR:
        {
            if (index == 0)
                return "Red";
            else if (index == 1)
                return "Green";
            else if (index == 2)
                return "Blue";
            else
                return "Alpha";
        }
            
        default:
        {
            DVASSERT(false);
            return "???";
        }
    }
}

VariantType ValueProperty::GetSubValue(int index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
            return index == 0 ? VariantType(GetValue().AsVector2().x) : VariantType(GetValue().AsVector2().y);

        case VariantType::TYPE_COLOR:
        {
            if (index == 0)
                return VariantType(GetValue().AsColor().r);
            else if (index == 1)
                return VariantType(GetValue().AsColor().g);
            else if (index == 2)
                return VariantType(GetValue().AsColor().b);
            else
                return VariantType(GetValue().AsColor().a);
        }
        default:
            DVASSERT(false);
            return VariantType();
    }
}

void ValueProperty::SetSubValue(int index, const DAVA::VariantType &newValue)
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
        {
            Vector2 val = GetValue().AsVector2();
            if (index == 0)
                val.x = newValue.AsFloat();
            else
                val.y = newValue.AsFloat();
            
            SetValue(VariantType(val));
            break;
        }
            
        case VariantType::TYPE_COLOR:
        {
            Color val = GetValue().AsColor();
            if (index == 0)
                val.r = newValue.AsFloat();
            else if (index == 1)
                val.g = newValue.AsFloat();
            else if (index == 2)
                val.b = newValue.AsFloat();
            else
                val.a = newValue.AsFloat();
            
            SetValue(VariantType(val));
            break;
        }
            
        default:
            DVASSERT(false);
            break;
    }
}
