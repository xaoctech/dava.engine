#include "Scene3D/Components/SplineComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SplineComponent)
{
    ReflectionRegistrator<SplineComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

SplineComponent::SplineComponent()
{
    controlPoints = { { { -3.5f, -43.5f, 0.0f }, 1.0f, 0.2f },
                      { { -16.5f, -34.2f, 0.0f }, 2.0f, 1.0f },
                      { { -17.0f, -23.0f, 0.0f }, 2.0f, 1.0f },
                      { { -17.0f, -11.8f, 0.0f }, 2.0f, 1.0f },
                      { { -15.2f, 9.8f, 0.0f }, 2.0f, 1.0f },
                      { { -2.8f, 25.1f, 0.0f }, 2.0f, 1.0f },
                      { { 12.8f, 29.3f, 0.0f }, 1.0f, 0.2f } };
}

Component* SplineComponent::Clone(Entity* toEntity)
{
    SplineComponent* newComponent = new SplineComponent();
    newComponent->controlPoints = controlPoints;
    return newComponent;
}
void SplineComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (NULL != archive)
    {
        uint32 numControlPoints = static_cast<uint32>(controlPoints.size());
        archive->SetUInt32("splineComponent.controlPointsCount", numControlPoints);
        if (numControlPoints != 0)
        {
            archive->SetByteArray("splineComponent.controlPoints", reinterpret_cast<uint8*>(&controlPoints.front()), sizeof(SplinePoint) * numControlPoints);
        }
    }
}
void SplineComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        uint32 numControlPoints = archive->GetUInt32("splineComponent.controlPointsCount");
        if (numControlPoints != 0)
        {
            const uint8* byteArray = archive->GetByteArray("splineComponent.controlPoints");
            DVASSERT(byteArray != nullptr);
            DVASSERT(archive->GetByteArraySize("splineComponent.controlPoints") == sizeof(SplinePoint) * numControlPoints);
            controlPoints.resize(numControlPoints);
            Memcpy(&controlPoints[0], byteArray, sizeof(SplinePoint) * numControlPoints);
        }
    }
    Component::Deserialize(archive, serializationContext);
}

const Vector<SplineComponent::SplinePoint>& SplineComponent::GetControlPoints() const
{
    return controlPoints;
}
}