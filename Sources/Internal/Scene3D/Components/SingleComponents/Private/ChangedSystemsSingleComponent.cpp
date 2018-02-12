#include "Scene3D/Components/SingleComponents/ChangedSystemsSingleComponent.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ChangedSystemsSingleComponent)
{
    ReflectionRegistrator<ChangedSystemsSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void ChangedSystemsSingleComponent::Clear()
{
    addedSystems.clear();
    removedSystems.clear();
}
}