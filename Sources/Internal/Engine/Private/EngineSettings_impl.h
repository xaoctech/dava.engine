#pragma once

#ifndef __DAVA_ENGINE_SETTINGS_IMPL__
#include "Engine/EngineSettings.h"
#endif

namespace DAVA
{
inline FastName EngineSettingsVar::GetName() const
{
    return name;
}

inline const Any& EngineSettingsVar::GetValue() const
{
    return value;
}

inline const String& EngineSettingsVar::GetHelp() const
{
    return help;
}

inline const ReflectedMeta* EngineSettingsVar::GetMeta() const
{
    return meta.get();
}

inline size_t EngineSettings::GetVarsCount() const
{
    return vars.size();
}

inline EngineSettingsVar* EngineSettings::GetVar(size_t index)
{
    if (index < vars.size())
    {
        return vars[index].get();
    }

    return nullptr;
}
} //ns DAVA
