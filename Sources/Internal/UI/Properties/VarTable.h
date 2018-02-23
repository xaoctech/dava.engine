
#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/FastName.h"

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
    struct VarProperty
    {
        Any value;
        FlagsType flags = 0;
    };
    using VarMap = Map<FastName, VarProperty>;
    using DefaultsMap = Map<FastName, Any>;

    void Clear();
    VarMap& GetProperties();

    const VarMap& GetProperties() const;
    void SetProperties(const VarMap& value);

    void ResetValuesToDefaults();
    void ClearDefaults();
    bool HasDefaultValues() const;
    bool HasDefaultValue(const FastName& name) const;
    void SetDefaultValue(const FastName& name, const Any& value);
    const Any& GetDefaultValue(const FastName& name);
    void RemoveDefault(const FastName& name);

    bool HasAnyOverridden() const;

    const Any& GetPropertyValue(const FastName& name);
    void SetPropertyValue(const FastName& name, const Any& value);
    void RemoveProperty(const FastName& name);
    bool HasProperty(const FastName& name) const;
    void SetPropertyFlag(const FastName& name, Flags flag, bool value);
    bool IsPropertyOverridden(const FastName& name) const;
    void SetPropertyOverridden(const FastName& name, bool value);

    void ClearFlags();
    void SetFlag(Flags flag, bool value);

    void AddPropertiesIfNotExists(const VarTable& other);
    void MarkOverriddenValues(const VarTable& other);
    void MarkOverridden();

    String GetNamesString(const String& sep = ", ") const;

    void AddAnyFromString(const String& name, const String& typeStr, const String& value);
    void AddAnyFromString(const String& name, const Type* type, const String& value);

    static Any ParseString(const Type* type, const String& str);
    static String AnyToString(const Any& val);

    static const Vector<const Type*> SUPPORTED_TYPES;

private:
    VarMap propeties;
    DefaultsMap defaults;
};
}
