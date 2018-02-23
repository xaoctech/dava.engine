#include "ShootCooldownComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;

REGISTER_CLASS(ShootCooldownComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShootCooldownComponent)
{
    ReflectionRegistrator<ShootCooldownComponent>::Begin()[M::Replicable(M::Privacy::PRIVATE)]
    .Field("LastShootFrameId",
           &ShootCooldownComponent::GetLastShootFrameId,
           &ShootCooldownComponent::SetLastShootFrameId)[M::Replicable()]
    .ConstructorByPointer()
    .End();
}

ShootCooldownComponent::ShootCooldownComponent()
{
}

Component* ShootCooldownComponent::Clone(Entity* toEntity)
{
    ShootCooldownComponent* component = new ShootCooldownComponent();
    component->SetLastShootFrameId(GetLastShootFrameId());
    component->SetEntity(toEntity);
    return component;
}

uint32 ShootCooldownComponent::GetLastShootFrameId() const
{
    return lastShootFrameId;
}

void ShootCooldownComponent::SetLastShootFrameId(uint32 frameId)
{
    lastShootFrameId = frameId;
}

ShootCooldownComponent::~ShootCooldownComponent()
{
}
