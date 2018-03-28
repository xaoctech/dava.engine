#include "GameModes/Cubes/SmallCubeComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>

DAVA_VIRTUAL_REFLECTION_IMPL(SmallCubeComponent)
{
    using namespace DAVA;
    ReflectionRegistrator<SmallCubeComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* SmallCubeComponent::Clone(DAVA::Entity* toEntity)
{
    SmallCubeComponent* component = new SmallCubeComponent();
    component->SetEntity(toEntity);
    return component;
}
