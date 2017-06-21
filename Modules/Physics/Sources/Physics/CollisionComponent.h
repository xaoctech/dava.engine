#pragma once

#include <Entity/Component.h>
#include <Scene3D/Entity.h>

#include <Reflection/Reflection.h>

namespace physx
{
class PxShape;
} // namespace physx

namespace DAVA
{
class CollisionComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(COLLISION_COMPONENT);
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    physx::PxShape* GetPxShape() const;

private:
    physx::PxShape* shape = nullptr;

    friend class PhysicsSystem;
    void SetPxShape(physx::PxShape* shape);
    void ReleasePxShape();

    DAVA_VIRTUAL_REFLECTION(CollisionComponent, Component);
};
} // namespace DAVA