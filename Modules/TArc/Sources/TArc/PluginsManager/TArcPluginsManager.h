#pragma once

#include <Base/BaseTypes.h>
#include <PluginManager/Plugin.h>

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
        TArcPlugin* pluginInstance = nullptr;
        PluginHandle handle;
        String libraryPath;
    };

    Vector<PluginNode> pluginsCollection;
};
} // namespace TArc
} // namespace DAVA