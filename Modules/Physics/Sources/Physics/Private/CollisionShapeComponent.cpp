#include "Physics/CollisionShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxShape.h>

namespace DAVA
{
void CollisionShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetFastName("shape.name", name);
    archive->SetMatrix4("shape.localPose", localPose);
}

void CollisionShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    name = archive->GetFastName("shape.name", FastName(""));
    localPose = archive->GetMatrix4("shape.localPose");
}

physx::PxShape* CollisionShapeComponent::GetPxShape() const
{
    return shape;
}

const DAVA::FastName& CollisionShapeComponent::GetName() const
{
    return name;
}

void CollisionShapeComponent::SetName(const FastName& name_)
{
    DVASSERT(name_.IsValid());
    name = name_;
    if (shape != nullptr)
    {
        shape->setName(name.c_str());
    }
}

const DAVA::Matrix4& CollisionShapeComponent::GetLocalPose() const
{
    return localPose;
}

void CollisionShapeComponent::SetLocalPose(const Matrix4& localPose_)
{
    localPose = localPose_;
    if (shape != nullptr)
    {
        shape->setLocalPose(physx::PxTransform(PhysicsMath::Matrix4ToPxMat44(localPose)));
    }
}

void CollisionShapeComponent::SetPxShape(physx::PxShape* shape_)
{
    DVASSERT(shape_ != nullptr);
    DVASSERT(shape == nullptr);
    shape = shape_;
    shape->userData = this;
    DVASSERT(name.IsValid());
    shape->setName(name.c_str());
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Matrix4ToPxMat44(localPose)));

#if defined(__DAVAENGINE_DEBUG__)
    CheckShapeType();
#endif
}

void CollisionShapeComponent::CopyFields(CollisionShapeComponent* component)
{
    component->name = name;
    component->localPose = localPose;
}

void CollisionShapeComponent::ReleasePxShape()
{
    DVASSERT(shape != nullptr);
    shape->release();
    shape = nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CollisionShapeComponent)
{
    ReflectionRegistrator<CollisionShapeComponent>::Begin()
    .Field("Name", &CollisionShapeComponent::GetName, &CollisionShapeComponent::SetName)
    .Field("Local pose", &CollisionShapeComponent::localPose)
    .End();
}

} // namespace DAVA