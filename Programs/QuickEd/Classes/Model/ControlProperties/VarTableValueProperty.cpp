#include "VarTableValueProperty.h"

#include <UI/UIControl.h>

using namespace DAVA;

VarTableValueProperty::VarTableValueProperty(DAVA::BaseObject* object, const String& name, const Reflection& ref, const IntrospectionProperty* prototypeProperty)
    : IntrospectionProperty(object, nullptr, name, ref, prototypeProperty)
{
    if (prototypeProperty)
    {
        SetDefaultValue(prototypeProperty->GetValue());
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
        SetPropertyOverridden(propertyName, false);
        SetValue(Any(varTable));
    }
    else if (varTable.HasDefaultValues())
    {
        if (varTable.HasDefaultValue(propertyName))
        {
            varTable.SetPropertyValue(propertyName, varTable.GetDefaultValue(propertyName));
            SetPropertyOverridden(propertyName, false);
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
    SetPropertyOverridden(propertyName, true);
    SetValue(Any(varTable));

    UpdateRealProperties();
}

bool VarTableValueProperty::IsOverriddenLocally(const String& propertyName_)
{
    return IsSubPropertyOverridden(FastName(propertyName_));
}

void VarTableValueProperty::UpdateRealProperties()
{
    bool hasOverriden = false;
    Any vt = GetValue();
    DVASSERT(vt.CanGet<VarTable>());

    VarTable varTable = vt.Get<VarTable>();
    hasOverriden = HasAnySubPropertyOverridden();

    if (hasOverriden == false && GetPrototypeProperty() != nullptr)
    {
        ResetValue();
    }
    else
    {
        SetValue(GetValue());
    }
}

inline void VarTableValueProperty::SetValue(const DAVA::Any& newValue)
{
    DVASSERT(newValue.CanGet<VarTable>());
    VarTable varTable = newValue.Get<VarTable>();

    if (GetPrototypeProperty() != nullptr)
    {
        Any valueDefault = GetPrototypeProperty()->GetValue();
        DVASSERT(valueDefault.CanGet<VarTable>());
        VarTable varTableDefault = valueDefault.Get<VarTable>();

        SetSubOverriddenIfNotEqual(varTable, varTableDefault);
        ValueProperty::SetValue(Any(varTable));
        SetOverridden(HasAnySubPropertyOverridden());
    }
    else if (varTable.HasDefaultValues())
    {
        SetSubOverriddenIfNotEqualDefaultValues(varTable);
        ValueProperty::SetValue(Any(varTable));
        SetOverridden(HasAnySubPropertyOverridden());
    }
    else
    {
        propertyOverridden.clear();
        varTable.ForEachProperty([&](const FastName& name, const Any& value) {
            propertyOverridden.insert(name);
        });

        ValueProperty::SetValue(Any(varTable));
    }
}

void VarTableValueProperty::SetDefaultValue(const Any& newValue)
{
    DVASSERT(newValue.CanGet<VarTable>());
    VarTable varTable = newValue.Get<VarTable>();
    ValueProperty::SetDefaultValue(Any(varTable));
}

void VarTableValueProperty::ResetValue()
{
    Any value = GetValue();
    DVASSERT(value.CanGet<VarTable>());
    VarTable varTable = value.Get<VarTable>();
    propertyOverridden.clear();

    if (GetPrototypeProperty() != nullptr)
    {
        SetDefaultValue(GetPrototypeProperty()->GetValue());
    }
    else if (varTable.HasDefaultValues())
    {
        varTable.ResetToDefaultValues();
        SetDefaultValue(Any(varTable));
    }
    else
    {
        SetDefaultValue(Any(VarTable()));
    }
    IntrospectionProperty::ResetValue();
}

bool VarTableValueProperty::HasAnySubPropertyOverridden() const
{
    return !propertyOverridden.empty();
}

bool VarTableValueProperty::IsSubPropertyOverridden(const FastName& name) const
{
    return propertyOverridden.find(name) != propertyOverridden.end();
}

void VarTableValueProperty::SetPropertyOverridden(const FastName& name, bool value)
{
    if (value)
    {
        propertyOverridden.insert(name);
    }
    else
    {
        propertyOverridden.erase(name);
    }
}

void VarTableValueProperty::SetSubOverriddenIfNotEqual(VarTable& varTable, VarTable& varTableDefault)
{
    propertyOverridden.clear();
    varTable.ForEachProperty([&](const FastName& name, const Any& value) {
        if ((!varTableDefault.HasProperty(name)) || !varTable.CheckAndUpdateProperty(name, varTableDefault.GetPropertyValue(name)))
        {
            propertyOverridden.insert(name);
        }
    });
}

void VarTableValueProperty::SetSubOverriddenIfNotEqualDefaultValues(VarTable& varTable)
{
    propertyOverridden.clear();
    varTable.ForEachProperty([&](const FastName& name, const Any& value) {
        propertyOverridden.insert(name);
    });
    varTable.ForEachDefaultValue([&](const FastName& name, const Any& value) {
        if (varTable.CheckAndUpdateProperty(name, value))
        {
            propertyOverridden.erase(name);
        }
    });
}

void VarTableValueProperty::OverrideSubProperties(const VarTable& other)
{
    Any vt = GetValue();
    DVASSERT(vt.CanGet<VarTable>());
    VarTable varTable = vt.Get<VarTable>();

    other.ForEachProperty([&](const FastName& name, const Any& value) {
        propertyOverridden.insert(name);
        varTable.SetPropertyValue(name, value);
    });
    SetValue(varTable);
    SetForceOverride(true);
}

String VarTableValueProperty::GetNamesString(const String& sep) const
{
    Any vt = GetValue();
    DVASSERT(vt.CanGet<VarTable>());
    VarTable varTable = vt.Get<VarTable>();

    String str;
    varTable.ForEachProperty([&](const FastName& name, const Any& value) {
        if (str.empty() == false)
        {
            str += sep;
        }
        if (IsSubPropertyOverridden(name))
        {
            str += "*";
        }
        DVASSERT(!name.empty());
        str += name.c_str();
    });
    return str;
}