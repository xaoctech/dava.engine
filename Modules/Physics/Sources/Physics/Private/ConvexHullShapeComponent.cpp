#include "Physics/ConvexHullShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>

namespace DAVA
{
DAVA::Component* ConvexHullShapeComponent::Clone(Entity* toEntity)
{
    ConvexHullShapeComponent* result = new ConvexHullShapeComponent();
    result->SetEntity(toEntity);
    CopyFields(result);

    return result;
}

void ConvexHullShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Serialize(archive, serializationContext);
}

void ConvexHullShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Deserialize(archive, serializationContext);
}

#if defined(__DAVAENGINE_DEBUG__)
void ConvexHullShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eCONVEXMESH);
}
#endif

DAVA_VIRTUAL_REFLECTION_IMPL(ConvexHullShapeComponent)
{
    ReflectionRegistrator<ConvexHullShapeComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
