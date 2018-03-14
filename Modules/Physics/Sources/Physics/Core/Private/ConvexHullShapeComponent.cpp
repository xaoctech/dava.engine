#include "Physics/Core/ConvexHullShapeComponent.h"
#include "PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>

namespace DAVA
{
Component* ConvexHullShapeComponent::Clone(Entity* toEntity)
{
    ConvexHullShapeComponent* result = new ConvexHullShapeComponent();
    CopyFieldsIntoClone(result);
    result->SetEntity(toEntity);

    return result;
}

#if defined(__DAVAENGINE_DEBUG__)
void ConvexHullShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eCONVEXMESH);
}
#endif

void ConvexHullShapeComponent::UpdateLocalProperties()
{
    physx::PxShape* shape = GetPxShape();
    DVASSERT(shape != nullptr);
    physx::PxConvexMeshGeometry geom;
    shape->getConvexMeshGeometry(geom);
    geom.scale.scale = PhysicsMath::Vector3ToPxVec3(scale);
    shape->setGeometry(geom);

    CollisionShapeComponent::UpdateLocalProperties();
}

DAVA_VIRTUAL_REFLECTION_IMPL(ConvexHullShapeComponent)
{
    ReflectionRegistrator<ConvexHullShapeComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
