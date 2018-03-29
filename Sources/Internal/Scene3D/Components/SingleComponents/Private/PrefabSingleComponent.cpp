#include "Scene3D/Components/SingleComponents/PrefabSingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(PrefabSingleComponent)
{
    ReflectionRegistrator<PrefabSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}
} // namespace DAVA
