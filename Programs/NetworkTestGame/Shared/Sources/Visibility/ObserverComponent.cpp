#include "ObserverComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(ObserverComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ObserverComponent)
{
    ReflectionRegistrator<ObserverComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

ObserverComponent::ObserverComponent()
    : visibilityByObservableId(MAX_OBSERVABLES_COUNT)
{
}

Component* ObserverComponent::Clone(Entity* toEntity)
{
    ObserverComponent* component = new ObserverComponent();
    component->SetEntity(toEntity);
    return component;
}

void ObserverComponent::ClearCache()
{
    updatedObservablesIds.clear();
}

void ObserverComponent::Reset()
{
    Memset(visibilityByObservableId.data(), visibilityByObservableId.size(), 0);
    updatedObservablesIds.clear();
}
