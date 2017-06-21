#include "Physics/CollisionComponent.h"

#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxShape.h>

namespace DAVA
{

Component* CollisionComponent::Clone(Entity* toEntity)
{
    CollisionComponent* result = new CollisionComponent();
    result->SetEntity(toEntity);

    return result;
}

void CollisionComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void CollisionComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

physx::PxShape* CollisionComponent::GetPxShape() const
{
    return shape;
}

void CollisionComponent::SetPxShape(physx::PxShape* shape_)
{
    DVASSERT(shape_ != nullptr);
    DVASSERT(shape == nullptr);
    shape = shape_;
    shape->userData = this;
}

void CollisionComponent::ReleasePxShape()
{
    DVASSERT(shape != nullptr);
    shape->release();
    shape = nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CollisionComponent)
{
    ReflectionRegistrator<CollisionComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA