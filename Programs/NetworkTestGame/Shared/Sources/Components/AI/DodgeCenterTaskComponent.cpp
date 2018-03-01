#include "DodgeCenterTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(DodgeCenterTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(DodgeCenterTaskComponent)
{
    ReflectionRegistrator<DodgeCenterTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* DodgeCenterTaskComponent::Clone(Entity* toEntity)
{
    DodgeCenterTaskComponent* component = new DodgeCenterTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

DodgeCenterTaskComponent::DodgeCenterTaskComponent(bool movingRight_)
    : movingRight(movingRight_)
{
}
