#include "ObservableIdComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(ObservableIdComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ObservableIdComponent)
{
    ReflectionRegistrator<ObservableIdComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

ObservableIdComponent::ObservableIdComponent()
{
}

Component* ObservableIdComponent::Clone(Entity* toEntity)
{
    ObservableIdComponent* component = new ObservableIdComponent();
    component->SetEntity(toEntity);
    return component;
}
