#pragma once

#include "Entity/SceneSystem.h"
#include "Entity/SystemProcessInfo.h"

namespace DAVA
{
/**
    Tracks SceneSystems types.

    When you need to register a scene system, you should register its reflection and provide a permanent name for it.

    If reflection registration is done before engine init, scene system will be automatically registered in SystemManager,
    otherwise call to SystemManager::RegisterSystem is required.

    Systems that do not have tags (M::SystemTags) will not be registered in SystemManager.

    If your system requires process, you should specify process info meta (M::SystemProcessInfo) for system methods.
 
    Example:
     \code
     // You need to register permanent name of your class in module init function.
     void YourCustomModule::Init()
     {
 
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(YouCustomSystem);
     }
 
     // In your system .h file
     class YourCustomSystem : public SceneSystem
     {
     public:
         DAVA_VIRTUAL_REFLECTION(YourCustomSystem, SceneSystem);
     };
 
     // In your system .cpp file
     DAVA_VIRTUAL_REFLECTION_IMPL(YourCustomSystem)
     {
     ReflectionRegistrator<YourCustomSystem>::Begin()[M::SystemTags("tag1", FastName("tag2"), ..., "tagn")]
     .ConstructorByPointer<Scene*>()
     .Method("ProcessFixed", &YourCustomSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::Gameplay, SPI::Type::Fixed, 4.0f)]
     .Method("Process", &YourCustomSystem::Process)[M::SystemProcessInfo(SPI::Group::Gameplay, SPI::Type::Normal, 2.0f)]
     .End();
     }
     \endcode
*/

class AnyFn;
class FastTags;
class SystemProcessInfo;
class ReflectedStructure;

namespace Private
{
class EngineBackend;
}

class SystemManager final
{
public:
    struct SystemProcess final
    {
        SystemProcess(const AnyFn& process, const SystemProcessInfo& info)
            : process(process)
            , info(info)
        {
        }

        const AnyFn& process;
        const SystemProcessInfo& info;
    };

    struct SystemInfo final
    {
        const FastTags* tags = nullptr;
        Vector<SystemProcess> processMethods;
    };

    /** Register system with type `T`. `SystemTags` meta is required for type `T`. */
    template <typename T>
    bool RegisterSystem();

    /** Register system with Type `systemType`. `SystemTags` meta is required for Type `systemType`. */
    bool RegisterSystem(const Type* systemType);

    /** Return `true` if system with type `T` is registered. */
    template <typename T>
    bool IsSystemRegistered() const;

    /** Return `true` if system with Type `systemType` is registered. */
    bool IsSystemRegistered(const Type* systemType) const;

    /** Return list of all registered systems. */
    const Vector<const Type*>& GetSystems() const;

    /**
        Return registered systems with given `tags`.
        If `exactMatch` is `false`, system's tags will be checked as subset of `tags`.
        If `exactMatch` is `true`, system's tags will be checked as exact match of `tags`.
    */
    Vector<const Type*> GetSystems(const FastTags& tags, bool exactMatch = false) const;

    /** Return info for registered system of type `T`. */
    template <typename T>
    const SystemInfo* GetSystemInfo() const;

    /** Return info for registered system of Type `systemType`. */
    const SystemInfo* GetSystemInfo(const Type* systemType) const;

    /** Print sorted (by type, group & order) process methods info for `systemsTypes`. */
    void PrintSystemsSortedProcessMethodsInfo(const Vector<const Type*>& systemsTypes) const;

    /* Emitted when new system is registered in SystemManager. */
    Signal<const Type*, const SystemInfo*> systemRegistered;

private:
    SystemManager() = default;

    bool RegisterSystem(const Type& systemType, bool warningsAsAsserts);

    void PreregisterAllDerivedSceneSystemsRecursively();

    bool VerifySystemConstructor(const Vector<std::unique_ptr<AnyFn>>& systemConstructors) const;

    bool VerifyProcessMethods(const Vector<SystemProcess>& processMethods) const;

    SystemInfo CollectSystemInfo(const ReflectedStructure& reflectedStructure) const;

    Vector<const Type*> systems;
    UnorderedMap<const Type*, SystemInfo> systemsInfo;
    mutable UnorderedSet<SystemProcessInfo> systemProcessesVerificationCache;

    friend class Private::EngineBackend; // For creation
};

} // namespace DAVA

#include "Entity/Private/SystemManager_impl.h"
