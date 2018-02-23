#include "Scene3D/Components/SplineComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SplineComponent::SplinePoint)
{
    ReflectionRegistrator<SplineComponent::SplinePoint>::Begin()
    .ConstructorByPointer()
    .Field("position", &SplineComponent::SplinePoint::position)
    .Field("width", &SplineComponent::SplinePoint::width)
    .Field("value", &SplineComponent::SplinePoint::value)
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(SplineComponent)
{
    ReflectionRegistrator<SplineComponent>::Begin()
    .ConstructorByPointer()
    .Field("controlPoints", &SplineComponent::controlPoints)
    .End();
}

SplineComponent::SplineComponent()
{
}

SplineComponent::~SplineComponent()
{
    for (SplinePoint* p : controlPoints)
    {
        delete p;
    }
}

Component* SplineComponent::Clone(Entity* toEntity)
{
    SplineComponent* newComponent = new SplineComponent();

    newComponent->controlPoints.reserve(controlPoints.size());
    for (SplinePoint* p : controlPoints)
    {
        SplinePoint* newPoint = new SplinePoint;
        *newPoint = *p;
        newComponent->controlPoints.push_back(newPoint);
    }

    return newComponent;
}
void SplineComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (archive != nullptr)
    {
        uint32 numControlPoints = static_cast<uint32>(controlPoints.size());
        archive->SetUInt32("splineComponent.pointsCount", numControlPoints);
        if (numControlPoints != 0)
        {
            ScopedPtr<KeyedArchive> pointArchive(new KeyedArchive);
            for (uint32 i = 0; i < numControlPoints; ++i)
            {
                SplinePoint* point = controlPoints[i];
                pointArchive->SetVector3("pos", point->position);
                pointArchive->SetFloat("val", point->value);
                pointArchive->SetFloat("width", point->width);
                archive->SetArchive(Format("splineComponent.point%u", i), pointArchive);
            }
        }
    }
}
void SplineComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive != nullptr)
    {
        uint32 numControlPoints = archive->GetUInt32("splineComponent.pointsCount");
        if (numControlPoints != 0)
        {
            {
                controlPoints.reserve(numControlPoints);
                for (uint32 i = 0; i < numControlPoints; ++i)
                {
                    KeyedArchive* pointArchive = archive->GetArchive(Format("splineComponent.point%u", i));
                    if (pointArchive != nullptr)
                    {
                        SplinePoint* point = new SplinePoint;
                        point->position = pointArchive->GetVector3("pos", point->position);
                        point->value = pointArchive->GetFloat("val", point->value);
                        point->width = pointArchive->GetFloat("width", point->width);
                        controlPoints.push_back(point);
                    }
                }
            }
        }
    }
    Component::Deserialize(archive, serializationContext);
}

const Vector<SplineComponent::SplinePoint*>& SplineComponent::GetControlPoints() const
{
    return controlPoints;
}
}