#include "Physics/SphereShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>
#include <physx/geometry/PxSphereGeometry.h>

namespace DAVA
{

DAVA::Component* SphereShapeComponent::Clone(Entity* toEntity)
{
    SphereShapeComponent* result = new SphereShapeComponent();
    result->SetEntity(toEntity);

    result->radius = radius;
    CopyFields(result);

    return result;
}

void SphereShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Serialize(archive, serializationContext);
    archive->SetFloat("sphereShape.radius", radius);
}

void SphereShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Deserialize(archive, serializationContext);
    radius = archive->GetFloat("sphereShape.radius", radius);
}

float32 SphereShapeComponent::GetRadius() const
{
    return radius;
}

void SphereShapeComponent::SetRadius(float32 radius_)
{
    radius = radius_;
    SheduleUpdate();
}

#if defined(__DAVAENGINE_DEBUG__)
void SphereShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eSPHERE);
}
#endif

void SphereShapeComponent::UpdateLocalProperties()
{
    physx::PxShape* shape = GetPxShape();
    physx::PxSphereGeometry geom;
    shape->getSphereGeometry(geom);
    geom.radius = radius;
    shape->setGeometry(geom);

    CollisionShapeComponent::UpdateLocalProperties();
}

DAVA_VIRTUAL_REFLECTION_IMPL(SphereShapeComponent)
{
    ReflectionRegistrator<SphereShapeComponent>::Begin()
    .ConstructorByPointer()
    .Field("Radius", &SphereShapeComponent::GetRadius, &SphereShapeComponent::SetRadius)[M::Range(0.00001f, Any(), 1.0f)]
    .End();
}

} // namespace DAVA
