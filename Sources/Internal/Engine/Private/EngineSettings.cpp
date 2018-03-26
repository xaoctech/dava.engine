#include "EngineSettings_impl.h"
#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
void EngineSettingsVar::SetValue(Any v)
{
    DVASSERT(settings != nullptr);
    DVASSERT(v.GetType() == value.GetType());

    // TODO:
    // - apply validator from meta
    // - check range from meta
    // ..

    value = std::move(v);
    settings->varChanged.Emit(this);
}

void EngineSettingsVar::SetValueWithCast(Any v)
{
    if (v.GetType() == value.GetType())
    {
        SetValue(v);
    }
    else
    {
        Any cv = v.Cast(value.GetType());
        SetValue(std::move(cv));
    }
}

EngineSettingsVar* EngineSettings::RegisterVar(FastName name, Any defaultValue, String help, ReflectedMeta&& meta)
{
    EngineSettingsVar* var = nullptr;

    auto it = varsNameMap.find(name);
    if (it == varsNameMap.end())
    {
        var = new EngineSettingsVar();
        var->name = name;
        var->value = defaultValue;
        var->help = help;
        var->meta.reset(new ReflectedMeta(std::move(meta)));
        var->settings = this;

        vars.emplace_back(var);
        varsNameMap.emplace(name, vars.size() - 1);
        varRegistered.Emit(var);
    }
    else
    {
        EngineSettingsVar* var = GetVar(it->second);

        DVASSERT(nullptr != var);
        DVASSERT(var->help == help);
        DVASSERT(var->value.GetType() == defaultValue.GetType());
    }

    return var;
}

EngineSettingsVar* EngineSettings::GetVar(FastName name)
{
    EngineSettingsVar* ret = nullptr;

    auto it = varsNameMap.find(name);
    if (it != varsNameMap.end())
    {
        ret = vars[it->second].get();
    }

    return ret;
}

const Any& EngineSettings::GetValue(FastName name)
{
    static Any notExists;

    EngineSettingsVar* var = GetVar(name);
    if (nullptr != var)
    {
        return var->GetValue();
    }

    return notExists;
}

void EngineSettings::SetValue(FastName name, Any value)
{
    EngineSettingsVar* var = GetVar(name);
    if (nullptr != var)
    {
        var->SetValue(std::move(value));
    }
}

void EngineSettings::Save(String path)
{
    DVASSERT(false, "Not implemented");
}

void EngineSettings::Load(String path)
{
    DVASSERT(false, "Not implemented");
}

} // end namespace DAVA
