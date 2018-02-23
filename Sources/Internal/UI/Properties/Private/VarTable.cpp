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
  //Type::Instance<Color>(),
};

void VarTable::Clear()
{
    propeties.clear();
}

VarTable::VarMap& VarTable::GetProperties()
{
    return propeties;
}

const VarTable::VarMap& VarTable::GetProperties() const
{
    return propeties;
}

void VarTable::SetProperties(const VarTable::VarMap& value)
{
    propeties = value;
}

void VarTable::ResetValuesToDefaults()
{
    propeties.clear();
    for (auto& p : defaults)
    {
        SetPropertyValue(p.first, p.second);
    }
}

void VarTable::ClearDefaults()
{
    defaults.clear();
}

bool VarTable::HasDefaultValues() const
{
    return defaults.size() > 0;
}

bool VarTable::HasDefaultValue(const FastName& name) const
{
    return defaults.find(name) != defaults.end();
}

void VarTable::SetDefaultValue(const FastName& name, const Any& value)
{
    defaults[name] = value;
}

const Any& VarTable::GetDefaultValue(const FastName& name)
{
    return defaults[name];
}

void VarTable::RemoveDefault(const FastName& name)
{
    defaults.erase(name);
}

bool VarTable::HasAnyOverridden() const
{
    for (auto& v : propeties)
    {
        if (v.second.flags[Flags::OVERRIDDEN])
        {
            return true;
        }
    }
    return false;
}

bool VarTable::IsPropertyOverridden(const FastName& name) const
{
    auto it = propeties.find(name);
    if (it != propeties.end())
    {
        return it->second.flags[Flags::OVERRIDDEN];
    }
    return false;
}

void VarTable::SetPropertyOverridden(const FastName& name, bool value)
{
    propeties[name].flags[Flags::OVERRIDDEN] = value;
}

const Any& VarTable::GetPropertyValue(const FastName& name)
{
    return propeties[name].value;
}

void VarTable::SetPropertyValue(const FastName& name, const Any& value)
{
    propeties[name].value = value;
}

void VarTable::SetPropertyFlag(const FastName& name, Flags flag, bool value)
{
    auto& p = propeties[name];
    p.flags[flag] = value;
}

void VarTable::RemoveProperty(const FastName& name)
{
    propeties.erase(name);
}

bool VarTable::HasProperty(const FastName& name) const
{
    return propeties.find(name) != propeties.end();
}

String VarTable::GetNamesString(const String& sep) const
{
    String str;
    for (auto& v : propeties)
    {
        if (str.empty() == false)
        {
            str += sep;
        }
        if (v.second.flags[Flags::OVERRIDDEN])
        {
            str += "*";
        }
        DVASSERT(!v.first.empty());
        str += v.first.c_str();
    }
    return str;
}

void VarTable::AddAnyFromString(const String& name, const String& typeStr, const String& value)
{
    const ReflectedType* reflectedType = ReflectedTypeDB::GetByPermanentName(typeStr);
    DVASSERT(reflectedType);
    const Type* type = reflectedType->GetType();

    AddAnyFromString(name, type, value);
}

void VarTable::AddAnyFromString(const String& name, const Type* type, const String& value)
{
    DVASSERT(type);
    VarProperty& p = propeties[FastName(name)];
    p.value = ParseString(type, value);
}

void VarTable::ClearFlags()
{
    for (auto& p : propeties)
    {
        p.second.flags = 0;
    }
}

void VarTable::SetFlag(Flags flag, bool value)
{
    for (auto& p : propeties)
    {
        p.second.flags[flag] = value;
    }
}

void VarTable::AddPropertiesIfNotExists(const VarTable& varTablePrototype)
{
    for (auto& p : varTablePrototype.propeties)
    {
        if (propeties.find(p.first) == propeties.end())
        {
            VarProperty& property = propeties[p.first];
            property.value = p.second.value;
        }
    }
}

void VarTable::MarkOverriddenValues(const VarTable& varTablePrototype)
{
    SetFlag(Flags::OVERRIDDEN, true);
    for (auto& p : varTablePrototype.propeties)
    {
        auto propIt = propeties.find(p.first);
        if (propIt == propeties.end())
        {
            VarProperty& property = propeties[p.first];
            property.value = p.second.value;
            property.flags[Flags::OVERRIDDEN] = false;
        }
        else
        {
            VarProperty& property = propeties[p.first];
            if (property.value.GetType() != p.second.value.GetType())
            {
                property.value = p.second.value;
                property.flags[Flags::OVERRIDDEN] = false;
            }
            else if (property.value == p.second.value)
            {
                property.flags[Flags::OVERRIDDEN] = false;
            }
            else
            {
                property.flags[Flags::OVERRIDDEN] = true;
            }
        }
    }
}

void VarTable::MarkOverridden()
{
    SetFlag(Flags::OVERRIDDEN, true);
    for (auto& p : defaults)
    {
        auto propIt = propeties.find(p.first);
        if (propIt == propeties.end())
        {
            VarProperty& property = propeties[p.first];
            property.value = p.second;
            property.flags[Flags::OVERRIDDEN] = false;
        }
        else
        {
            VarProperty& property = propeties[p.first];
            if (property.value.GetType() != p.second.GetType())
            {
                property.value = p.second;
                property.flags[Flags::OVERRIDDEN] = false;
            }
            else if (property.value == p.second)
            {
                property.flags[Flags::OVERRIDDEN] = false;
            }
            else
            {
                property.flags[Flags::OVERRIDDEN] = true;
            }
        }
    }
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
}
