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
    else if (defaultValue.GetType() == VariantType::TYPE_COLOR)
    {
        AddProperty(new SubValueProperty(0));
        AddProperty(new SubValueProperty(1));
        AddProperty(new SubValueProperty(2));
        AddProperty(new SubValueProperty(3));
    }
    else if (defaultValue.GetType() == VariantType::TYPE_INT32 && member->Desc().type == InspDesc::T_FLAGS)
    {
        const EnumMap *map = member->Desc().enumMap;
        for (int32 i = 0; i < (int32) map->GetCount(); i++)
            AddProperty(new SubValueProperty(i));
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
            
        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int val = 0;
                map->GetValue(index, val);
                return map->ToString(val);
            }
            else
            {
                DVASSERT(false);
                return "???";
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
            {
                DVASSERT(index >= 0 && index < 2);
                return VariantType(GetValue().AsVector2().data[index]);
            }

        case VariantType::TYPE_COLOR:
        {
            DVASSERT(index >= 0 && index < 4);
            return VariantType(GetValue().AsColor().color[index]);
        }

        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int val = 0;
                map->GetValue(index, val);
                return VariantType((GetValue().AsInt32() & val) != 0);
            }
            else
            {
                DVASSERT(false);
                return VariantType();
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
            
        case VariantType::TYPE_INT32:
            if (member->Desc().type == InspDesc::T_FLAGS)
            {
                const EnumMap *map = member->Desc().enumMap;
                int32 value = GetValue().AsInt32();

                int val = 0;
                map->GetValue(index, val);
                if (newValue.AsBool())
                    SetValue(VariantType(value | val));
                else
                    SetValue(VariantType(value & (~val)));
            }
            else
            {
                DVASSERT(false);
            }
            break;
            
        default:
            DVASSERT(false);
            break;
    }
}
