#include "TArc/PluginsManager/TArcPluginsManager.h"

#include <Engine/EngineContext.h>
#include <PluginManager/PluginManager.h>

#include <FileSystem/FileList.h>
#include <Debug/DVAssert.h>

#define PLUGIN_MANAGER_STR_VALUE(name) #name

namespace DAVA
{
namespace TArc
{
TArcPluginManager::TArcPluginManager(const String& applicationName_, const String& pluginsFolder_)
    : applicationName(applicationName_)
    , pluginsFolder(pluginsFolder_)
{
}

TArcPluginManager::~TArcPluginManager()
{
    DVASSERT(pluginsCollection.empty() == true);
}

void TArcPluginManager::LoadPlugins(Vector<String>& errors)
{
#define REPORT_LOADING_ERROR(error)\
    errors.push_back(error);\
    continue

    UnorderedMap<String, size_t> pluginsMap;

    const EngineContext* ctx = GetEngineContext();
    Vector<FilePath> plugins = PluginManager::LookupPlugins(pluginsFolder, PluginManager::Auto);
    for (FilePath& filePath : plugins)
    {
        String absPath = filePath.GetAbsolutePathname();
        PluginHandle handle = OpenPlugin(absPath.c_str());
        if (handle == nullptr)
        {
            REPORT_LOADING_ERROR(Format("Couldn't load dynamic library %s", absPath.c_str()));
        }

        TCreatePluginFn createFn = LoadFunction<TCreatePluginFn>(handle, PLUGIN_MANAGER_STR_VALUE(CREATE_PLUGIN_FUNCTION_NAME));
        if (createFn == nullptr)
        {
            REPORT_LOADING_ERROR(Format("Dynamic library %s doesn't contains TArcPlugin", absPath.c_str()));
        }

        Vector<TArcPlugin*> plugins = createFn(ctx);
        for (TArcPlugin* pluginInstance : plugins)
        {
            if (pluginInstance == nullptr)
            {
                REPORT_LOADING_ERROR(Format("Dynamic librray %s can't create plugin", absPath.c_str()));
            }

            const TArcPlugin::PluginDescriptor& descriptor = pluginInstance->GetDescription();
            if (applicationName != descriptor.applicationName)
            {
                REPORT_LOADING_ERROR(Format("Plugin %s loaded from %s isn't matched by applicationName. Plugin's application name is %s, plugins manager is configured for %s",
                                            descriptor.pluginName.c_str(), absPath.c_str(), descriptor.applicationName.c_str(), applicationName.c_str()));
            }

            auto iter = pluginsMap.find(descriptor.pluginName);
            if (iter != pluginsMap.end())
            {
                String path = pluginsCollection[iter->second].libraryPath;
                REPORT_LOADING_ERROR(Format("Plugin's name conflict. Libraries %s and %s contains plugin with same name %s. The last one will be ignored",
                                            path.c_str(), absPath.c_str(), descriptor.pluginName));
            }

            PluginNode node;
            node.handle = handle;
            node.pluginInstance = pluginInstance;
            node.libraryPath = absPath;
            pluginsCollection.push_back(node);

            pluginsMap.emplace(descriptor.pluginName, pluginsCollection.size());
        }
    }

#undef REPORT_LOADING_ERROR
}

void TArcPluginManager::UnloadPlugins()
{
    for (PluginNode& node : pluginsCollection)
    {
        ClosePlugin(node.handle);
    }

    pluginsCollection.clear();
}

TArcPlugin* TArcPluginManager::GetPlugin(const String& pluginName) const
{
    for (const PluginNode& node : pluginsCollection)
    {
        if (node.pluginInstance->GetDescription().pluginName == pluginName)
        {
            return node.pluginInstance;
        }
    }

    return nullptr;
}

DAVA::Vector<TArcPlugin*> TArcPluginManager::GetPluginsWithBaseType(const Type* t) const
{
    Vector<TArcPlugin*> result;
    result.reserve(pluginsCollection.size());
    for (const PluginNode& node : pluginsCollection)
    {
        const ReflectedType* moduleType = node.pluginInstance->GetModuleType();
        const Type* type = moduleType->GetType();
        if (TypeInheritance::CanDownCast(type, t))
        {
            result.push_back(node.pluginInstance);
        }
    }

    return result;
}

} // namespace TArc
} // namespace DAVA