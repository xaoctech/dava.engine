#include "Physics/MeshShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>

namespace DAVA
{
DAVA::Component* MeshShapeComponent::Clone(Entity* toEntity)
{
    MeshShapeComponent* result = new MeshShapeComponent();
    result->SetEntity(toEntity);

    CopyFields(result);

    return result;
}

void MeshShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Serialize(archive, serializationContext);
}

void MeshShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Deserialize(archive, serializationContext);
}

#if defined(__DAVAENGINE_DEBUG__)
void MeshShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eTRIANGLEMESH);
}
#endif

DAVA_VIRTUAL_REFLECTION_IMPL(MeshShapeComponent)
{
    ReflectionRegistrator<MeshShapeComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
