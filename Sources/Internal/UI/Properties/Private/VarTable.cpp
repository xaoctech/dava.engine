#include "UI/Properties/VarTable.h"

#include "Utils/StringUtils.h"
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Math/Color.h"

namespace DAVA
{
const Vector<const Type*> VarTable::SUPPORTED_TYPES =
{
  Type::Instance<bool>(),
  Type::Instance<int32>(),
  Type::Instance<uint32>(),
  Type::Instance<int64>(),
  Type::Instance<uint64>(),
  Type::Instance<float32>(),
  Type::Instance<FastName>(),
  Type::Instance<String>(),
  Type::Instance<WideString>(),
  //Type::Instance<FilePath>(),
  Type::Instance<Vector2>(),
  Type::Instance<Vector3>(),
  Type::Instance<Vector4>(),
  Type::Instance<Color>(),
};

void VarTable::ClearProperties()
{
    properties.clear();
}

void VarTable::ForEachProperty(const Function<void(const FastName& name, const Any& value)>& f) const
{
    for (auto& p : properties)
    {
        f(p.first, p.second);
    }
}

void VarTable::ForEachDefaultValue(const Function<void(const FastName& name, const Any& value)>& f) const
{
    for (auto& p : defaultValues)
    {
        if (!p.second.IsEmpty())
        {
            f(p.first, p.second);
        }
    }
}

Any VarTable::GetPropertyValue(const FastName& name) const
{
    auto it = properties.find(name);
    DVASSERT(it != properties.end(), "Property not found!");
    return it != properties.end() ? it->second : Any();
}

void VarTable::SetPropertyValue(const FastName& name, const Any& value)
{
    properties[name] = value;
}

void VarTable::RemoveProperty(const FastName& name)
{
    properties.erase(name);
    defaultValues.erase(name);
}

bool VarTable::HasProperty(const FastName& name) const
{
    return properties.find(name) != properties.end();
}

void VarTable::ResetToDefaultValues()
{
    properties.clear();
    ForEachDefaultValue([&](const FastName& name, const Any& value) {
        SetPropertyValue(name, value);
    });
}

void VarTable::ClearDefaultValues()
{
    defaultValues.clear();
}

bool VarTable::HasDefaultValues() const
{
    return defaultValues.end() != std::find_if(defaultValues.begin(), defaultValues.end(), [](const auto& pair) { return !pair.second.IsEmpty(); });
}

bool VarTable::HasDefaultValue(const FastName& name) const
{
    auto it = defaultValues.find(name);
    return it != defaultValues.end() && !it->second.IsEmpty();
}

void VarTable::SetDefaultValue(const FastName& name, const Any& value)
{
    defaultValues[name] = value;
}

Any VarTable::GetDefaultValue(const FastName& name) const
{
    auto it = defaultValues.find(name);
    if (it == defaultValues.end())
    {
        DVASSERT(it != defaultValues.end(), "Property not found!");
    }
    return it != defaultValues.end() ? it->second : Any();
}

void VarTable::RemoveDefaultValue(const FastName& name)
{
    defaultValues.erase(name);
}

void VarTable::Insert(const VarTable& other, bool overwriteValues)
{
    other.ForEachProperty([&](const FastName& name, const Any& value) {
        if (overwriteValues || !HasProperty(name))
        {
            SetPropertyValue(name, value);
        }
    });
}

bool VarTable::CheckAndUpdateProperty(const FastName& propertyName, const Any& referenceValue)
{
    if (!HasProperty(propertyName))
    {
        SetPropertyValue(propertyName, referenceValue);
        return true;
    }
    else
    {
        const Any& value = GetPropertyValue(propertyName);
        if (value.GetType() != referenceValue.GetType())
        {
            SetPropertyValue(propertyName, referenceValue);
            return true;
        }
        else if (value == referenceValue)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool VarTable::operator==(const VarTable& other) const
{
    return properties == other.properties;
}

Any VarTable::ParseString(const Type* type, const String& str)
{
    if (type == Type::Instance<bool>())
        return Any(str != "0" && str != "false");
    else if (type == Type::Instance<int32>())
        return Any(std::atoi(str.c_str()));
    else if (type == Type::Instance<uint32>())
        return Any(static_cast<uint32>(std::atoll(str.c_str())));
    else if (type == Type::Instance<int64>())
        return Any(static_cast<int64>(std::atoll(str.c_str())));
    else if (type == Type::Instance<uint64>())
        return Any(static_cast<uint64>(std::atoll(str.c_str())));
    else if (type == Type::Instance<float32>())
        return Any(static_cast<float>(std::atof(str.c_str())));
    else if (type == Type::Instance<FastName>())
        return Any(FastName(str));
    else if (type == Type::Instance<String>())
        return Any(str);
    else if (type == Type::Instance<WideString>())
        return Any(UTF8Utils::EncodeToWideString(str));
    else if (type == Type::Instance<Vector2>())
    {
        Vector2 v;
        sscanf(str.c_str(), "%f %f", &v.x, &v.y);
        return Any(v);
    }
    else if (type == Type::Instance<Vector3>())
    {
        Vector3 v;
        sscanf(str.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
        return Any(v);
    }
    else if (type == Type::Instance<Vector4>())
    {
        Vector4 v;
        sscanf(str.c_str(), "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
        return Any(v);
    }
    else if (type == Type::Instance<Color>())
    {
        Color v;
        sscanf(str.c_str(), "%f %f %f %f", &v.r, &v.g, &v.b, &v.a);
        return Any(v);
    }
    // else if (type == Type::Instance<Rect>())
    //     return Any(AsRect());
    else if (type == Type::Instance<FilePath>())
        return Any(FilePath(str));

    return Any(str);
}

String VarTable::AnyToString(const Any& val)
{
    if (val.CanGet<int32>())
    {
        return Format("%d", val.Get<int32>());
    }
    else if (val.CanGet<uint32>())
    {
        return Format("%d", val.Get<uint32>());
    }
    else if (val.CanGet<uint64>())
    {
        return Format("%ld", val.Get<uint64>());
    }
    else if (val.CanGet<int64>())
    {
        return Format("%ldL", val.Get<int64>());
    }
    else if (val.CanGet<float32>())
    {
        return Format("%f", val.Get<float32>());
    }
    else if (val.CanGet<String>())
    {
        return val.Get<String>();
    }
    else if (val.CanGet<WideString>())
    {
        return UTF8Utils::EncodeToUTF8(val.Get<WideString>());
    }
    else if (val.CanGet<FastName>())
    {
        const FastName& fastName = val.Get<FastName>();
        if (fastName.IsValid())
        {
            return fastName.c_str();
        }
        else
        {
            return "";
        }
    }
    else if (val.CanGet<FilePath>())
    {
        return val.Get<FilePath>().GetStringValue();
    }
    else if (val.CanGet<Vector2>())
    {
        Vector2 v = val.Get<Vector2>();
        return Format("%f %f", v.x, v.y);
    }
    else if (val.CanGet<Vector3>())
    {
        Vector3 v = val.Get<Vector3>();
        return Format("%f %f %f", v.x, v.y, v.z);
    }
    else if (val.CanGet<Vector4>())
    {
        Vector4 v = val.Get<Vector4>();
        return Format("%f %f %f %f", v.x, v.y, v.z, v.w);
    }
    else if (val.CanGet<Color>())
    {
        Color v = val.Get<Color>();
        return Format("%f %f %f %f", v.r, v.g, v.b, v.a);
    }
    else if (val.CanGet<bool>())
    {
        return val.Get<bool>() ? "true" : "false";
    }

    DVASSERT(false);
    return String("");
}

template <>
bool AnyCompare<VarTable>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<VarTable>() == v2.Get<VarTable>();
}
}
