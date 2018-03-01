#include "CharacterVisibilityShapeComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(CharacterVisibilityShapeComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(CharacterVisibilityShapeComponent)
{
    ReflectionRegistrator<CharacterVisibilityShapeComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

CharacterVisibilityShapeComponent::CharacterVisibilityShapeComponent()
{
}

Component* CharacterVisibilityShapeComponent::Clone(Entity* toEntity)
{
    CharacterVisibilityShapeComponent* component = new CharacterVisibilityShapeComponent();
    component->SetEntity(toEntity);
    return component;
}
