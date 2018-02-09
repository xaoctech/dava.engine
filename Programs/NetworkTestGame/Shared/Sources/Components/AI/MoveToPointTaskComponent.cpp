#include "MoveToPointTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(MoveToPointTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(MoveToPointTaskComponent)
{
    ReflectionRegistrator<MoveToPointTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* MoveToPointTaskComponent::Clone(Entity* toEntity)
{
    MoveToPointTaskComponent* component = new MoveToPointTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

MoveToPointTaskComponent::MoveToPointTaskComponent(const Vector3& targetPoint_, float precision_)
    :
    targetPoint(targetPoint_)
    , precision(precision_)
{
}
