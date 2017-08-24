#include "Scene3D/Components/ParticleDragForceComponent.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ParticleDragForceComponent)
{
    ReflectionRegistrator<ParticleDragForceComponent>::Begin()
    .ConstructorByPointer()
    .Field("some var", &ParticleDragForceComponent::someFloat)[M::DisplayName("SomeVar")]
    .Field("force over life", &ParticleDragForceComponent::forceOverLife)[M::DisplayName("Force over life")]
    .End();
}

DAVA::Component* ParticleDragForceComponent::Clone(Entity* toEnity)
{
    ParticleDragForceComponent* newComponent = new ParticleDragForceComponent();
    newComponent->SetEntity(toEnity);
    return newComponent;
}
}