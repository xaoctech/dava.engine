#include "Physics/CapsuleShapeComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>
#include <physx/geometry/PxCapsuleGeometry.h>

namespace DAVA
{
uint32 CapsuleShapeComponent::GetType() const
{
    return Component::CAPSULE_SHAPE_COMPONENT;
}

Component* CapsuleShapeComponent::Clone(Entity* toEntity)
{
    CapsuleShapeComponent* result = new CapsuleShapeComponent();
    result->SetEntity(toEntity);

    result->radius = radius;
    result->halfHeight = halfHeight;
    CopyFields(result);

    return result;
}

void CapsuleShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Serialize(archive, serializationContext);
    archive->SetFloat("capsuleShape.radius", radius);
    archive->SetFloat("capsuleShape.halfSize", halfHeight);
}

void CapsuleShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Deserialize(archive, serializationContext);
    radius = archive->GetFloat("capsuleShape.radius", radius);
    halfHeight = archive->GetFloat("capsuleShape.halfSize", halfHeight);
}

float32 CapsuleShapeComponent::GetRadius() const
{
    return radius;
}

void CapsuleShapeComponent::SetRadius(float32 r)
{
    radius = r;

    physx::PxShape* shape = GetPxShape();
    if (shape != nullptr)
    {
        physx::PxCapsuleGeometry geom(radius, halfHeight);
        shape->setGeometry(geom);
    }
}

DAVA::float32 CapsuleShapeComponent::GetHalfHeight() const
{
    return halfHeight;
}

void CapsuleShapeComponent::SetHalfHeight(float32 halfHeight_)
{
    halfHeight = halfHeight_;

    physx::PxShape* shape = GetPxShape();
    if (shape != nullptr)
    {
        physx::PxCapsuleGeometry geom(radius, halfHeight);
        shape->setGeometry(geom);
    }
}

#if defined(__DAVAENGINE_DEBUG__)
void CapsuleShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eCAPSULE);
}
#endif

DAVA_VIRTUAL_REFLECTION_IMPL(CapsuleShapeComponent)
{
    ReflectionRegistrator<CapsuleShapeComponent>::Begin()
    .ConstructorByPointer()
    .Field("Radius", &CapsuleShapeComponent::GetRadius, &CapsuleShapeComponent::SetRadius)[M::Range(0.00001f, Any(), 1.0f)]
    .Field("Half Height", &CapsuleShapeComponent::GetHalfHeight, &CapsuleShapeComponent::SetHalfHeight)[M::Range(0.00001f, Any(), 1.0f)]
    .End();
}

} // namespace DAVA