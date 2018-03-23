#include "Scene3D/Components/SingleComponents/RenderObjectSingleComponent.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(RenderObjectSingleComponent)
{
    ReflectionRegistrator<RenderObjectSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
