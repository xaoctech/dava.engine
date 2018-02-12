#pragma once

#include "Base/FastName.h"
#include "Base/Type.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    Tracks Systems/UISystems types.
    When you need to add system from module, you should register your system in reflection.
    Systems are registered in scene in ascending order. Higher order means later execution of system process.
    Systems that do not have order meta for class are not executed.
 
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
 
     // In you system .cpp file
     DAVA_VIRTUAL_REFLECTION_IMPL(YourCustomSystem)
     {
     ReflectionRegistrator<YourCustomSystem>::Begin()[M::Tags("tag1", "tag2", ..., "tagn")]
     .ConstructorByPointer<Scene*>()
     .Method("ProcessFixed", &YourCustomSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 4.0f)]
     .Method("Process", &YourCustomSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::NORMAL, 2.0f)]
     .End();
     }
     \endcode
*/

class AnyFn;
class UISystem;
class System;

namespace Private
{
class EngineBackend;
}

class SystemManager
{
    friend class Private::EngineBackend; // For creation
public:
    struct SceneTagInfo
    {
        const Type* systemType;
        const Vector<FastName>* tags;
    };

    struct SceneProcessInfo
    {
        const Type* systemType;
        const AnyFn* method;
    };

    const Vector<SceneTagInfo>& GetRegisteredSceneSystems() const;
    const Vector<SceneTagInfo>& GetSystemsWithoutProcessMethods() const;

    const Vector<FastName>& GetTagsForSystem(const Type* systemType);

    const Vector<SceneProcessInfo>& GetProcessMethods() const;
    const Vector<SceneProcessInfo>& GetFixedProcessMethods() const;

    void RegisterAllDerivedSceneSystemsRecursively();

private:
    SystemManager() = default;

    Vector<SceneTagInfo> sceneSystems;
    Vector<SceneTagInfo> systemsWithoutProcessMethods;

    Vector<SceneProcessInfo> methodsToProcess;
    Vector<SceneProcessInfo> methodsToFixedProcess;
};
}
