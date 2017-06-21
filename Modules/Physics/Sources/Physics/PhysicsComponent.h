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

class PhysicsComponent : public Component
{
public:
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    physx::PxActor* GetPxActor() const;

protected:
#if defined(__DAVAENGINE_DEBUG__)
    virtual void ValidateActorType() const = 0;
#endif

    physx::PxActor* actor = nullptr;

private:
    friend class PhysicsSystem;
    void SetPxActor(physx::PxActor* actor);
    void ReleasePxActor();

    DAVA_VIRTUAL_REFLECTION(PhysicsComponent, Component);
};
} // namespace DAVA
