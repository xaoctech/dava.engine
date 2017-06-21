#include "Physics/DynamicBodyComponent.h"
#include "Physics/PhysicsModule.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>

#include <ModuleManager/ModuleManager.h>
#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxRigidDynamic.h>

namespace DAVA
{
Component* DynamicBodyComponent::Clone(Entity* toEntity)
{
    DynamicBodyComponent* result = new DynamicBodyComponent();
    result->SetEntity(toEntity);

    if (actor != nullptr)
    {
        Physics* physics = GetEngineContext()->moduleManager->GetModule<Physics>();
        result->actor = physics->ClonePxActor(actor, result);
    }

    return result;
}

void DynamicBodyComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Serialize(archive, serializationContext);
}

void DynamicBodyComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Deserialize(archive, serializationContext);
}

#if defined(__DAVAENGINE_DEBUG__)
void DynamicBodyComponent::ValidateActorType() const
{
    DVASSERT(actor->is<physx::PxRigidDynamic>());
}
#endif

DAVA_VIRTUAL_REFLECTION_IMPL(DynamicBodyComponent)
{
    ReflectionRegistrator<DynamicBodyComponent>::Begin()
    .ConstructorByPointer()
    .End();
}
} // namespace DAVA