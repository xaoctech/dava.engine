#include "DamageComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(DamageComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(DamageComponent)
{
    ReflectionRegistrator<DamageComponent>::Begin()[M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .End();
}

DamageComponent::DamageComponent()
{
}

Component* DamageComponent::Clone(Entity* toEntity)
{
    DamageComponent* component = new DamageComponent();
    component->SetEntity(toEntity);
    return component;
}

uint8 DamageComponent::GetDamage() const
{
    return damage;
}

DamageComponent::~DamageComponent()
{
}
