#pragma once

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
     ReflectionRegistrator<YourCustomSystem>::Begin()[M::Order(50)]
     .ConstructorByPointer<Scene*>()
     .End();
     }
     \endcode
*/

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
    /** Return const reference to sorted vector of registered SceneSystem types. */
    const Vector<std::pair<const Type*, int32>>& GetRegisteredSceneSystems() const;

private:
    SystemManager() = default;

    void PreregisterAllDerivedSceneSystemsRecursively();

    Vector<std::pair<const Type*, int32>> sceneSystems;
};
}
