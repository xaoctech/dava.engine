#include "Physics/DynamicBodyComponent.h"
#include "Physics/PhysicsModule.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>

#include <ModuleManager/ModuleManager.h>
#include <Reflection/ReflectionRegistrator.h>

#include <Base/GlobalEnum.h>

#include <physx/PxRigidDynamic.h>

ENUM_DECLARE(DAVA::DynamicBodyComponent::eLockFlags)
{
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::LinearX, "Linear X");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::LinearY, "Linear Y");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::LinearZ, "Linear Z");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::AngularX, "Angular X");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::AngularY, "Angular Y");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::AngularZ, "Angular Z");
}

namespace DAVA
{
uint32 DynamicBodyComponent::GetType() const
{
    return DYNAMIC_BODY_COMPONENT;
}

Component* DynamicBodyComponent::Clone(Entity* toEntity)
{
    DynamicBodyComponent* result = new DynamicBodyComponent();
    result->SetEntity(toEntity);
    CopyFields(result);
    result->linearDamping = linearDamping;
    result->angularDamping = angularDamping;
    result->maxAngularVelocity = maxAngularVelocity;
    result->lockFlags = lockFlags;

    return result;
}

void DynamicBodyComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Serialize(archive, serializationContext);
    archive->SetFloat("dynamicBody.linearDamping", linearDamping);
    archive->SetFloat("dynamicBody.angularDamping", angularDamping);
    archive->SetFloat("dynamicBody.maxAngularVelocity", maxAngularVelocity);
    archive->SetUInt32("dynamicBody.lockFlags", static_cast<uint32>(lockFlags));
}

void DynamicBodyComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Deserialize(archive, serializationContext);
    linearDamping = archive->GetFloat("dynamicBody.linearDamping", linearDamping);
    angularDamping = archive->GetFloat("dynamicBody.angularDamping", angularDamping);
    maxAngularVelocity = archive->GetFloat("dynamicBody.maxAngularVelocity", maxAngularVelocity);
    lockFlags = static_cast<eLockFlags>(archive->GetUInt32("dynamicBody.lockFlags", static_cast<uint32>(lockFlags)));
}

float32 DynamicBodyComponent::GetLinearDamping() const
{
    return linearDamping;
}

void DynamicBodyComponent::SetLinearDamping(float32 damping)
{
    linearDamping = damping;
    physx::PxRigidDynamic* actor = GetPxActor()->is<physx::PxRigidDynamic>();
    if (actor != nullptr)
    {
        actor->setLinearDamping(linearDamping);
    }
}

float32 DynamicBodyComponent::GetAngularDamping() const
{
    return angularDamping;
}

void DynamicBodyComponent::SetAngularDamping(float32 damping)
{
    angularDamping = damping;
    physx::PxRigidDynamic* actor = GetPxActor()->is<physx::PxRigidDynamic>();
    if (actor != nullptr)
    {
        actor->setAngularDamping(angularDamping);
    }
}

float32 DynamicBodyComponent::GetMaxAngularVelocity() const
{
    return maxAngularVelocity;
}

void DynamicBodyComponent::SetMaxAngularVelocity(float32 velocity)
{
    maxAngularVelocity = velocity;
    physx::PxRigidDynamic* actor = GetPxActor()->is<physx::PxRigidDynamic>();
    if (actor != nullptr)
    {
        actor->setMaxAngularVelocity(angularDamping);
    }
}

DynamicBodyComponent::eLockFlags DynamicBodyComponent::GetLockFlags() const
{
    return lockFlags;
}

void DynamicBodyComponent::SetLockFlags(eLockFlags lockFlags_)
{
    lockFlags = lockFlags_;
    physx::PxRigidDynamic* actor = GetPxActor()->is<physx::PxRigidDynamic>();
    if (actor != nullptr)
    {
        physx::PxRigidDynamicLockFlags flags(static_cast<physx::PxRigidDynamicLockFlag::Enum>(lockFlags));
        actor->setRigidDynamicLockFlags(flags);
    }
}

#if defined(__DAVAENGINE_DEBUG__)
void DynamicBodyComponent::CheckActorType() const
{
    DVASSERT(GetPxActor()->is<physx::PxRigidDynamic>());
}
#endif

void DynamicBodyComponent::SetPxActor(physx::PxActor* actor)
{
    PhysicsComponent::SetPxActor(actor);
    physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
    DVASSERT(dynamicActor != nullptr);

    dynamicActor->setLinearDamping(linearDamping);
    dynamicActor->setAngularDamping(angularDamping);
    dynamicActor->setMaxAngularVelocity(maxAngularVelocity);
    dynamicActor->setRigidDynamicLockFlags(physx::PxRigidDynamicLockFlags(static_cast<physx::PxRigidDynamicLockFlag::Enum>(lockFlags)));
}

DAVA_VIRTUAL_REFLECTION_IMPL(DynamicBodyComponent)
{
    ReflectionRegistrator<DynamicBodyComponent>::Begin()
    .ConstructorByPointer()
    .Field("Linear damping", &DynamicBodyComponent::GetLinearDamping, &DynamicBodyComponent::SetLinearDamping)[M::Range(0, Any(), 1.0f), M::Group("Damping")]
    .Field("Angular damping", &DynamicBodyComponent::GetAngularDamping, &DynamicBodyComponent::SetAngularDamping)[M::Range(0, Any(), 1.0f), M::Group("Damping")]
    .Field("Max angular velocity", &DynamicBodyComponent::GetMaxAngularVelocity, &DynamicBodyComponent::SetMaxAngularVelocity)[M::Range(0, Any(), 1.0f)]
    .Field("Lock flags", &DynamicBodyComponent::GetLockFlags, &DynamicBodyComponent::SetLockFlags)[M::FlagsT<DynamicBodyComponent::eLockFlags>()]
    .End();
}
} // namespace DAVA