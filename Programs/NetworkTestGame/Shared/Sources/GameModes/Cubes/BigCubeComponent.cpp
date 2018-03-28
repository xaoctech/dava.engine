#include "GameModes/Cubes/BigCubeComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>

DAVA_VIRTUAL_REFLECTION_IMPL(BigCubeComponent)
{
    using namespace DAVA;
    ReflectionRegistrator<BigCubeComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* BigCubeComponent::Clone(DAVA::Entity* toEntity)
{
    BigCubeComponent* component = new BigCubeComponent();
    component->playerId = playerId;
    component->SetEntity(toEntity);
    return component;
}