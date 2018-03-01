#include "SimpleVisibilityShapeComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(SimpleVisibilityShapeComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(SimpleVisibilityShapeComponent)
{
    ReflectionRegistrator<SimpleVisibilityShapeComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

SimpleVisibilityShapeComponent::SimpleVisibilityShapeComponent()
{
}

Component* SimpleVisibilityShapeComponent::Clone(Entity* toEntity)
{
    SimpleVisibilityShapeComponent* component = new SimpleVisibilityShapeComponent();
    component->SetEntity(toEntity);
    return component;
}
