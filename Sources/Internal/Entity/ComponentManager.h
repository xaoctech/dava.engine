#pragma once

#include "Base/Type.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    Track Components/UIComponents types.
    Before any Component/UIComponent is used, it must be added to both Reflection(through registering Reflection permanent name) and ComponentManager(through ComponentManager::RegisterComponent).

    For example:
    \code
        DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFlowLayoutComponent, "FlowLayout");
        GetEngineContext()->componentManager->RegisterComponent<UIFlowLayoutComponent>();
    \endcode

    Base engine Component/UIComponents are registered in ReflectionDeclaration.cpp

    After registration, Component/UIComponent can be created through Component/UIComponent::CreateByType(const Type* componentType).

    Component/UIComponent registration also introduces 'runtimeType' (just integer).
    'runtimeType' can be used for Component/UIComponents management along with its' Type. 'runtimeType' is typically used for optimization (for example, indeces in array). You cannot rely on actual 'runtimeType' value between launches of the application.
*/

class UIComponent;
class Component;

class ComponentManager
{
public:
    /** Create ComponentManager. */
    ComponentManager();

    /** Register new component of specified type 'T'. The behavior is undefined until 'T' is a Component/UIComponent subclass. */
    template <typename T>
    void RegisterComponent();

    void RegisterComponent(const Type* type);

    void RegisterAllDerivedSceneComponentsRecursively();

    uint32 GetCRC32HashOfReflectedSceneComponents();

    //just increment counter
    void RegisterFakeSceneComponent();

    /** Return total number of registered UIComponents. */
    uint32 GetUIComponentsCount() const;

    /** Return total number of registered Components. */
    uint32 GetSceneComponentsCount() const;

    /** Check if specified 'type' was registered as UIComponent. */
    bool IsUIComponent(const Type* type) const;

    /** Check if specified 'type' was registered as Scene Component. */
    bool IsSceneComponent(const Type* type) const;

    /** Return runtimeType for specified 'type'. The behavior is undefined until 'type' is registered in ComponentManager. */
    int32 GetRuntimeType(const Type* type) const;

    /** Return Type of Scene Component for specified 'runtimeType'. Return nullptr if 'runtimeType' was not registered in ComponentManager. */
    const Type* GetSceneTypeFromRuntimeType(int32 runtimeType) const;

    /** Return reference to sorted vector of registered UIComponents types. */
    const Vector<const Type*>& GetRegisteredUIComponents() const;

    /** Return reference to sorted vector of registered Scene Components types. */
    const Vector<const Type*>& GetRegisteredSceneComponents() const;

private:
    void RegisterSceneComponentRecursively(const Type* type);

    int32 runtimeUIComponentsCount = 0;
    Vector<const Type*> registeredUIComponents;

    int32 runtimeSceneComponentsCount = 0;
    Vector<const Type*> registeredSceneComponents;
    Vector<const Type*> sceneRuntimeTypeToType;

    int32 runtimeTypeIndex = -1;
    int32 componentType = -1;

    enum eComponentType : int32
    {
        UI_COMPONENT = 1,
        SCENE_COMPONENT
    };
};
}

#include "Entity/Private/ComponentManager_impl.h"