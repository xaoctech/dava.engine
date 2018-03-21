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

namespace NetworkFactoryComponentDetail
{
struct ICallbackHolder
{
    virtual void Invoke(Component* component) = 0;
};

template <typename T>
struct CallbackHolder : ICallbackHolder
{
    using Callback = Function<void(T*)>;
    Callback callback;

    void Invoke(Component* component) override
    {
        callback(static_cast<T*>(component));
    };
};
}

#define SETUP_AFTER_INIT(fc, SetupCompType, compAlias, ...) \
    std::unique_ptr<NetworkFactoryComponentDetail::ICallbackHolder>& holder##SetupCompType = fc->componetTypeToOverrideData[Type::Instance<SetupCompType>()].callback; \
    if (!holder##SetupCompType) holder##SetupCompType.reset(new NetworkFactoryComponentDetail::CallbackHolder<SetupCompType>()); \
    auto* theHolder##SetupCompType = static_cast<NetworkFactoryComponentDetail::CallbackHolder<SetupCompType>*>(holder##SetupCompType.get()); \
    theHolder##SetupCompType->callback = [__VA_ARGS__](SetupCompType * compAlias)

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
    void SetInitialTransform(const Vector3& position, const Quaternion& rotation);

    Component* Clone(Entity* toEntity) override;

    NetworkPlayerID playerId;
    String name;
    struct FieldValue
    {
        FastName name;
        Any value;
    };

    struct OverrideFieldData
    {
        std::unique_ptr<NetworkFactoryComponentDetail::ICallbackHolder> callback;
        Vector<FieldValue> fieldValues;
    };

    UnorderedMap<const Type*, OverrideFieldData> componetTypeToOverrideData;

    template <typename T>
    void OverrideField(const String& path, const T& value);

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
    OverrideFieldData& overrideFieldData = componetTypeToOverrideData[splitData.compType];
    overrideFieldData.fieldValues.push_back({ splitData.fieldName, Any(value) });
}
}
