
#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/FastName.h"
#include "Functional/Function.h"

namespace DAVA
{
class VarTable
{
public:
    using FlagsType = Bitset<8>;
    enum Flags
    {
        OVERRIDDEN = 0,
    };
    struct VarMeta
    {
        FlagsType flags = 0;
        Any defaultValue;
    };

    using VarMap = Map<FastName, Any>;
    using MetaMap = Map<FastName, VarMeta>;

    void ClearProperties();

    void ForEachProperty(const Function<void(const FastName& name, const Any& value)>& f) const;

    bool HasProperty(const FastName& name) const;
    Any GetPropertyValue(const FastName& name) const;
    void SetPropertyValue(const FastName& name, const Any& value);
    void RemoveProperty(const FastName& name);

    bool HasDefaultValues() const;
    bool HasDefaultValue(const FastName& name) const;
    Any GetDefaultValue(const FastName& name) const;
    void SetDefaultValue(const FastName& name, const Any& value);
    void RemoveDefaultValue(const FastName& name);
    void ClearDefaultValues();

    void ResetAllPropertiesToDefaultValues();

    void ClearFlags();
    void SetFlag(Flags flag, bool value);
    void SetPropertyFlag(const FastName& name, Flags flag, bool value);
    bool GetPropertyFlag(const FastName& name, Flags flag, bool defaultValue = false) const;

    bool HasAnyPropertyOverridden() const;
    bool IsPropertyOverridden(const FastName& name) const;
    void SetPropertyOverridden(const FastName& name, bool value);

    void AddPropertiesIfNotExists(const VarTable& other);
    void SetOverriddenIfNotEqual(const VarTable& other);
    void SetOverriddenIfNotEqualDefaultValues();
    void SetPropertyOverriddenFlagIfNotEqual(const FastName& propertyName, const Any& otherValue);

    String GetNamesString(const String& sep = ", ") const;

    static Any ParseString(const Type* type, const String& str);
    static String AnyToString(const Any& val);

    static const Vector<const Type*> SUPPORTED_TYPES;

private:
    MetaMap metaMap;
    VarMap properties;
};
}
