#pragma once

#include "Physics/PhysicsComponent.h"

namespace DAVA
{
class DynamicBodyComponent : public PhysicsComponent
{
public:
    IMPLEMENT_COMPONENT_TYPE(DYNAMIC_BODY_COMPONENT);
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void ValidateActorType() const override;
#endif

    DAVA_VIRTUAL_REFLECTION(DynamicBodyComponent, PhysicsComponent);
};
} // namespace DAVA
