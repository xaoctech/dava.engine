#pragma once

#include <Base/BaseTypes.h>
#include <Entity/Component.h>
#include <Entity/ComponentMask.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class NetworkPredictComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(NetworkPredictComponent, Component);

public:
    NetworkPredictComponent() = default;
    NetworkPredictComponent(ComponentMask predictionMask);

    template <typename T>
    void SetForPrediction();
    void SetForPrediction(const Type* componentType);

    const ComponentMask& GetPredictionMask() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    ComponentMask predictionMask;
};

template <typename T>
void NetworkPredictComponent::SetForPrediction()
{
    SetForPrediction(Type::Instance<T>());
}
} // namespace DAVA
