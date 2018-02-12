#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/VTSingleComponent.h"
#include "Scene3D/Components/VTDecalComponent.h"
#include "Scene3D/Components/SplineComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/Utils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VTSingleComponent)
{
    ReflectionRegistrator<VTSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void VTSingleComponent::Clear()
{
    vtDecalChanged.clear();
    vtSplineChanged.clear();
}

void VTSingleComponent::EraseEntity(Entity* entity)
{
    if (entity->GetComponentCount<VTDecalComponent>())
    {
        FindAndRemoveExchangingWithLast(vtDecalChanged, entity);
    }
    if (entity->GetComponentCount<SplineComponent>())
    {
        FindAndRemoveExchangingWithLast(vtSplineChanged, entity);
    }
}
}
