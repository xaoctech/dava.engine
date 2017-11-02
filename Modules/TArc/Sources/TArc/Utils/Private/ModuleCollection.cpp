#include "TArc/Utils/ModuleCollection.h"

namespace DAVA
{
void ModuleCollection::AddGuiModule(const TypeCreateFn& type)
{
    guiModules.push_back(type);
}

void ModuleCollection::AddConsoleModule(const TypeCreateFn& type, const String& command)
{
    consoleModules.emplace_back(type, command);
}

Vector<const ReflectedType*> ModuleCollection::GetGuiModules() const
{
    Vector<const ReflectedType*> types;
    for (const TypeCreateFn& fn : guiModules)
    {
        types.push_back(fn());
    }

    return types;
}

Vector<std::pair<const ReflectedType*, String>> ModuleCollection::GetConsoleModules() const
{
    Vector<std::pair<const ReflectedType*, String>> types;
    for (const auto& modulePair : consoleModules)
    {
        types.emplace_back(modulePair.first(), modulePair.second);
    }

    return types;
}
} // namespace DAVA
