#include "VarTableValueProperty.h"

#include <UI/UIControl.h>
#include <UI/Properties/VarTable.h>

using namespace DAVA;

VarTableValueProperty::VarTableValueProperty(DAVA::BaseObject* object, const String& name, const Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType)
    : IntrospectionProperty(object, nullptr, name, ref, sourceProperty, copyType)
{
    if (sourceProperty)
    {
        SetDefaultValue(sourceProperty->GetValue());
    }
}

VarTableValueProperty::~VarTableValueProperty()
{
}

Any VarTableValueProperty::GetDefaultLocalValue(const String& propertyName_)
{
    FastName propertyName(propertyName_);
    if (GetPrototypeProperty() != nullptr)
    {
        const VarTableValueProperty* vtPrototype = DynamicTypeCheck<const VarTableValueProperty*>(GetPrototypeProperty());
        Any prototypePropertyValue = vtPrototype->GetLocalValue(propertyName_);
        return prototypePropertyValue;
    }

    Any value = GetValue();
    DVASSERT(value.CanGet<VarTable>());
    VarTable varTable = value.Get<VarTable>();
    if (varTable.HasDefaultValue(propertyName))
    {
        return varTable.GetDefaultValue(propertyName);
    }

    return GetLocalValue(propertyName_);
}

void VarTableValueProperty::ResetLocalValue(const String& propertyName_)
{
    Any value = GetValue();
    DVASSERT(value.CanGet<VarTable>());
    VarTable varTable = value.Get<VarTable>();

    FastName propertyName(propertyName_);
    if (GetPrototypeProperty() != nullptr)
    {
        const VarTableValueProperty* vtPrototype = DynamicTypeCheck<const VarTableValueProperty*>(GetPrototypeProperty());
        Any prototypePropertyValue = vtPrototype->GetLocalValue(propertyName_);

        varTable.SetPropertyValue(propertyName, prototypePropertyValue);
        varTable.SetPropertyOverridden(propertyName, false);
        SetValue(Any(varTable));
    }
    else if (varTable.HasDefaultValues())
    {
        if (varTable.HasDefaultValue(propertyName))
        {
            varTable.SetPropertyValue(propertyName, varTable.GetDefaultValue(propertyName));
            varTable.SetPropertyOverridden(propertyName, false);
            SetValue(Any(varTable));
        }
    }
    UpdateRealProperties();
}

Any VarTableValueProperty::GetLocalValue(const String& propertyName) const
{
    Any vt = GetValue();
    DVASSERT(vt.CanGet<VarTable>());

    VarTable varTable = vt.Get<VarTable>();
    return varTable.GetPropertyValue(FastName(propertyName));
}

void VarTableValueProperty::SetLocalValue(const String& propertyName_, const Any& value)
{
    Any vt = GetValue();
    DVASSERT(vt.CanGet<VarTable>());

    FastName propertyName(propertyName_);
    VarTable varTable = vt.Get<VarTable>();
    varTable.SetPropertyValue(propertyName, value);
    varTable.SetPropertyOverridden(propertyName, true);
    vt = Any(varTable);
    SetValue(vt);

    UpdateRealProperties();
}

bool VarTableValueProperty::IsOverriddenLocally(const String& propertyName_)
{
    Any vt = GetValue();
    DVASSERT(vt.CanGet<VarTable>());
    VarTable varTable = vt.Get<VarTable>();
    return varTable.IsPropertyOverridden(FastName(propertyName_));
}

void VarTableValueProperty::UpdateRealProperties()
{
    bool hasOverriden = false;
    Any vt = GetValue();
    DVASSERT(vt.CanGet<VarTable>());

    VarTable varTable = vt.Get<VarTable>();
    hasOverriden = varTable.HasAnyOverridden();

    if (hasOverriden == false && GetPrototypeProperty() != nullptr)
    {
        ResetValue();
    }
    else
    {
        SetValue(GetValue());
    }
}

void VarTableValueProperty::SetValue(const Any& newValue)
{
    DVASSERT(newValue.CanGet<VarTable>());
    VarTable varTable = newValue.Get<VarTable>();

    if (GetPrototypeProperty() != nullptr)
    {
        Any valueDefault = GetPrototypeProperty()->GetValue();
        DVASSERT(valueDefault.CanGet<VarTable>());
        VarTable varTableDefault = valueDefault.Get<VarTable>();

        varTable.MarkOverriddenValues(varTableDefault);
        ValueProperty::SetValue(Any(varTable));
        SetOverridden(varTable.HasAnyOverridden());
    }
    else if (varTable.HasDefaultValues())
    {
        varTable.MarkOverridden();
        ValueProperty::SetValue(Any(varTable));
        SetOverridden(varTable.HasAnyOverridden());
    }
    else
    {
        varTable.SetFlag(VarTable::OVERRIDDEN, true);
        ValueProperty::SetValue(Any(varTable));
    }
}

void VarTableValueProperty::SetDefaultValue(const Any& newValue)
{
    DVASSERT(newValue.CanGet<VarTable>());
    VarTable varTable = newValue.Get<VarTable>();
    varTable.ClearFlags();
    ValueProperty::SetDefaultValue(Any(varTable));
}

void VarTableValueProperty::ResetValue()
{
    Any value = GetValue();
    DVASSERT(value.CanGet<VarTable>());
    VarTable varTable = value.Get<VarTable>();

    if (GetPrototypeProperty() != nullptr)
    {
        SetDefaultValue(GetPrototypeProperty()->GetValue());
        IntrospectionProperty::ResetValue();
    }
    else if (varTable.HasDefaultValues())
    {
        varTable.ResetValuesToDefaults();
        SetDefaultValue(Any(varTable));
        IntrospectionProperty::ResetValue();
    }
    else
    {
        SetDefaultValue(Any(VarTable()));
        IntrospectionProperty::ResetValue();
    }
}