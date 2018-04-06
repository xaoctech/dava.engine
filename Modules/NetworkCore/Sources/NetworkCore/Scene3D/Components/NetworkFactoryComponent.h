#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Functional/Function.h"
#include "NetworkCore/NetworkFactoryUtils.h"

namespace DAVA
{
class Entity;
class Component;

using CallbackWithCast = Function<void(Component*)>;
class NetworkFactoryComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(NetworkFactoryComponent, Component);

public:
    struct InitialTransform
    {
        Vector3 position;
        Quaternion rotation;
    };

    std::unique_ptr<InitialTransform> initialTransformPtr;
    void SetInitialTransform(const Vector3& position, const Quaternion& rotation, float32 scale = 1.0f);

    Component* Clone(Entity* toEntity) override;

    NetworkID replicationId;
    String name;
    float32 scale = 1.0f;

    struct FieldValue
    {
        FastName name;
        Any value;
    };

    struct OverrideFieldData
    {
        CallbackWithCast callbackWithCast = {};
        Vector<FieldValue> fieldValues;
    };

    UnorderedMap<const Type*, OverrideFieldData> componentTypeToOverrideData = {};
    Function<void(Entity*)> overrideEntityDataCallback = {};

    template <typename T>
    void OverrideField(const String& path, const T& value);

    template <typename T>
    void SetupAfterInit(Function<void(T*)> callback);
    void SetupAfterInit(Function<void(Entity*)> callback);

private:
    struct ComponentField
    {
        const Type* compType;
        FastName fieldName;
    };

    static UnorderedMap<FastName, ComponentField> componentFieldsCache;
    static const ComponentField& ParsePath(const String& path);
};

template <typename T>
void NetworkFactoryComponent::OverrideField(const String& path, const T& value)
{
    const ComponentField& splitData = ParsePath(path);
    OverrideFieldData& overrideFieldData = componentTypeToOverrideData[splitData.compType];
    overrideFieldData.fieldValues.push_back({ splitData.fieldName, Any(value) });
};

template <typename T>
void NetworkFactoryComponent::SetupAfterInit(Function<void(T*)> callback)
{
    OverrideFieldData& overrideFieldData = componentTypeToOverrideData[Type::Instance<T>()];
    overrideFieldData.callbackWithCast = [callback](Component* component)
    {
        callback(static_cast<T*>(component));
    };
};
}
