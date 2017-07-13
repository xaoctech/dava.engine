#pragma once

#include <PluginManager/Plugin.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class EngineContext;
namespace TArc
{
class TArcPlugin
{
public:
    struct PluginDescriptor
    {
        String applicationName;
        String pluginName;
        String shortDescription;
        String fullDescription;
        int32 majorVersion;
        int32 minorVersion;
    };

    TArcPlugin(const EngineContext* context);
    virtual ~TArcPlugin() = default;

    virtual const ReflectedType* GetModuleType() const = 0;
    virtual const PluginDescriptor& GetDescription() const = 0;
};

template <typename T>
class TypedTArcPlugin : public TArcPlugin
{
public:
    TypedTArcPlugin(const EngineContext* context, const TArcPlugin& descriptor)
        : TArcPlugin(context)
        , descr(descriptor)
    {
    }

    const ReflectedType* GetModuleType() const override
    {
        return ReflectedTypeDB::Get<T>();
    }

    const PluginDescriptor& GetDescription() const override
    {
        return descr;
    }

private:
    PluginDescriptor descr;
};
} // namespace TArc
} // namespace DAVA

#define CREATE_PLUGIN_FUNCTION_NAME CreatePlugin
#define DESTROY_PLUGIN_FUNCTION_NAME DestroyPlugin

using TCreatePluginFn = DAVA::Vector<DAVA::TArc::TArcPlugin*> (*)(const DAVA::EngineContext* context);
using TDestroyPluginFn = void (*)(DAVA::TArc::TArcPlugin* plugin);

#define START_PLUGINS_DECLARATION()\
    PLUGIN_FUNCTION_EXPORT void DESTROY_PLUGIN_FUNCTION_NAME(DAVA::TArc::TArcPlugin* plugin) \
    { \
        delete plugin; \
    }\
    PLUGIN_FUNCTION_EXPORT DAVA::Vector<DAVA::TArc::TArcPlugin*> CREATE_PLUGIN_FUNCTION_NAME(const DAVA::EngineContext* context) \
    { \
        DAVA::Vector<DAVA::TArc::TArcPlugin*> plugins;

#define DECLARE_PLUGIN(moduleType, appName, pluginName, shortDescr, fullDescr, majorVersion, minorVersion)\
    { \
        DAVA::TArc::TArcPlugin::PluginDescriptor descr; \
        descr.applicationName = appName; \
        descr.pluginName = pluginName; \
        descr.shortDescription = shortDescr; \
        descr.fullDescription = fullDescr; \
        descr.majorVersion = majorVersion; \
        descr.minorVersion = minorVersion; \
        plugins.push_back(new DAVA::TArc::TypedTArcPlugin<moduleType>(context, descr)); \
    }

#define END_PLUGINS_DECLARATION()\
        return plugins; \
    }
