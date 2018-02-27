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

    prevPosition = curPosition;
    prevRotation = curRotation;
    prevScale = curScale;
}

void TransformInterpolationComponent::ApplyImmediately()
{
    immediately = true;
}

void TransformInterpolationComponent::SetNewTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
    switch (state)
    {
    case InterpolationState::ELASTIC:
    {
        Reset();
        break;
    }

    case InterpolationState::FIXED:
    {
        Reset();
        curPosition = position;
        curRotation = rotation;
        curScale = scale;
        if (!isInit)
        {
            isInit = true;
            Reset();
        }
        break;
    }

    default:
        break;
    }
}

} // namespace DAVA
