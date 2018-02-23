#include "SingletonComponent.h"
#include <Reflection/ReflectionRegistrator.h>
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SingletonComponent)
{
    ReflectionRegistrator<SingletonComponent>::Begin()
    .End();
}

Component* SingletonComponent::Clone(Entity* toEntity)
{
    DVASSERT(0, "SingletonComponent is not cloneable");
    return nullptr;
}
}
