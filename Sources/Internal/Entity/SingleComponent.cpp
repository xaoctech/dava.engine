#include "SingleComponent.h"
#include <Reflection/ReflectionRegistrator.h>
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SingleComponent)
{
    ReflectionRegistrator<SingleComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(ClearableSingleComponent)
{
    ReflectionRegistrator<ClearableSingleComponent>::Begin()
    .End();
}

Component* SingleComponent::Clone(Entity* toEntity)
{
    DVASSERT(0, "SingleComponent is not cloneable");
    return nullptr;
}

ClearableSingleComponent::ClearableSingleComponent(Usage usage)
    : usage(usage)
{
}
}
