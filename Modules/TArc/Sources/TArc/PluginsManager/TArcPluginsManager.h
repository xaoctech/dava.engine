#pragma once

#include <PluginManager/Plugin.h>

#include <Base/Type.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
class TArcPlugin;
class TArcPluginManager
{
public:
    TArcPluginManager(const String& applicationName, const String& pluginsFolder);
    ~TArcPluginManager();

    void LoadPlugins(Vector<String>& errors);
    void UnloadPlugins();

    TArcPlugin* GetPlugin(const String& pluginName) const;
    Vector<TArcPlugin*> GetPluginsWithBaseType(const Type* t) const;

private:
    String applicationName;
    String pluginsFolder;

    struct PluginNode
    {
        Vector<TArcPlugin*> pluginInstances;
        PluginHandle handle;
        String libraryPath;
    };

    Vector<PluginNode> pluginsCollection;
};
} // namespace TArc
} // namespace DAVA