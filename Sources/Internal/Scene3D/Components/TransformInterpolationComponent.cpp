#include "Scene3D/Components/TransformInterpolationComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(TransformInterpolationComponent)
{
    ReflectionRegistrator<TransformInterpolationComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::CantBeDeletedManualyComponent()]
    .ConstructorByPointer()
    .Field("time", &TransformInterpolationComponent::time)
    .End();
}

Component* TransformInterpolationComponent::Clone(Entity* toEntity)
{
    TransformInterpolationComponent* newComp = new TransformInterpolationComponent();
    newComp->time = time;
    newComp->elapsed = 0;
    return newComp;
}

void TransformInterpolationComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}

void TransformInterpolationComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}

void TransformInterpolationComponent::Reset()
{
    done = false;
    elapsed = 0;

    startPosition = curPosition;
    startRotation = curRotation;
    startScale = curScale;
}

void TransformInterpolationComponent::ApplyImmediately()
{
    immediately = true;
}
} // namespace DAVA
