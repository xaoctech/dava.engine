#pragma once

#include <Entity/Component.h>
#include <Scene3D/Entity.h>
#include <Reflection/Reflection.h>

#include <Base/BaseTypes.h>

namespace physx
{
class PxActor;
} // namespace physx

namespace DAVA
{
class Entity;
class KeyedArchive;
class SerializationContext;
class PhysicsActor;

class PhysicsComponent : public Component
{
public:
    uint32 GetType() const override;
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    friend class PhysicsSystem;
    PhysicsActor* actor = nullptr;

    DAVA_VIRTUAL_REFLECTION(PhysicsComponent, Component);
};
} // namespace DAVA
