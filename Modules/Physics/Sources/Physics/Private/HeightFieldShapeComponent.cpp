#include "Physics/HeightFieldShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>

namespace DAVA
{
DAVA::Component* HeightFieldShapeComponent::Clone(Entity* toEntity)
{
    HeightFieldShapeComponent* result = new HeightFieldShapeComponent();
    result->SetEntity(toEntity);

    CopyFields(result);

    return result;
}

void HeightFieldShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Serialize(archive, serializationContext);
}

void HeightFieldShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Deserialize(archive, serializationContext);
}

#if defined(__DAVAENGINE_DEBUG__)
void HeightFieldShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eHEIGHTFIELD);
}
#endif

void HeightFieldShapeComponent::ReleasePxShape()
{
    physx::PxHeightFieldGeometry geom;
    GetPxShape()->getHeightFieldGeometry(geom);
    physx::PxHeightField* heightfield = geom.heightField;
    CollisionShapeComponent::ReleasePxShape();
    heightfield->release();
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightFieldShapeComponent)
{
    ReflectionRegistrator<HeightFieldShapeComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
