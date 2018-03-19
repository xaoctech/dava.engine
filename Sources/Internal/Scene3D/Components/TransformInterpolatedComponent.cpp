#include "Scene3D/Components/TransformInterpolatedComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(TransformInterpolatedComponent)
{
    ReflectionRegistrator<TransformInterpolatedComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* TransformInterpolatedComponent::Clone(Entity* toEntity)
{
    TransformInterpolatedComponent* newComp = new TransformInterpolatedComponent();
    return newComp;
}

} // namespace DAVA
