
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
    void ForEachProperty(const Function<void(const FastName& name, const Any& value)>& f) const;
    void ForEachDefaultValue(const Function<void(const FastName& name, const Any& value)>& f) const;

    void ClearProperties();
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
    void ResetToDefaultValues();

    void Insert(const VarTable& other, bool overwriteValues = true);
    bool CheckAndUpdateProperty(const FastName& propertyName, const Any& otherValue);

    bool operator==(const VarTable& other) const;

    static Any ParseString(const Type* type, const String& str);
    static String AnyToString(const Any& val);

    static const Vector<const Type*> SUPPORTED_TYPES;

private:
    using VarMap = Map<FastName, Any>;

    VarMap defaultValues;
    VarMap properties;
};

template <>
bool AnyCompare<VarTable>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<VarTable>;
}
