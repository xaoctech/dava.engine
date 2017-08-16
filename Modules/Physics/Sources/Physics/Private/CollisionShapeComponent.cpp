#include "Physics/CollisionShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxShape.h>

namespace DAVA
{
namespace CollisionShapeComponentDetail
{
enum
{
    CCD_FLAG = 1 << 29
};
} // namespace CollisionShapeComponentDetail

void CollisionShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetFastName("shape.name", name);
    archive->SetMatrix4("shape.localPose", localPose);
    archive->SetBool("shape.overrideMass", overrideMass);
    archive->SetFloat("shape.mass", mass);
}

void CollisionShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    name = archive->GetFastName("shape.name", FastName(""));
    localPose = archive->GetMatrix4("shape.localPose");
    overrideMass = archive->GetBool("shape.overrideMass", overrideMass);
    mass = archive->GetFloat("shape.mass", mass);
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
    SheduleUpdate();
}

const DAVA::Matrix4& CollisionShapeComponent::GetLocalPose() const
{
    return localPose;
}

void CollisionShapeComponent::SetLocalPose(const Matrix4& localPose_)
{
    localPose = localPose_;
    SheduleUpdate();
}

bool CollisionShapeComponent::GetOverrideMass() const
{
    return overrideMass;
}

void CollisionShapeComponent::SetOverrideMass(bool override)
{
    overrideMass = override;
    SheduleUpdate();
}

float32 CollisionShapeComponent::GetMass() const
{
    return mass;
}

void CollisionShapeComponent::SetMass(float32 mass_)
{
    if (overrideMass == true)
    {
        mass = mass_;
        SheduleUpdate();
    }
}

CollisionShapeComponent* CollisionShapeComponent::GetComponent(physx::PxShape* shape)
{
    DVASSERT(shape != nullptr);
    return reinterpret_cast<CollisionShapeComponent*>(shape->userData);
}

void CollisionShapeComponent::SetCCDActive(physx::PxShape* shape, bool isCCDActive)
{
    DVASSERT(shape != nullptr);
    physx::PxFilterData fd = shape->getSimulationFilterData();
    physx::PxU32 ccdFlag = static_cast<physx::PxU32>(CollisionShapeComponentDetail::CCD_FLAG);
    if (isCCDActive == true)
    {
        fd.word3 |= ccdFlag;
    }
    else
    {
        fd.word3 = fd.word3 & (~ccdFlag);
    }
    shape->setSimulationFilterData(fd);
}

bool CollisionShapeComponent::IsCCDActive(const physx::PxFilterData& filterData)
{
    return filterData.word3 & CollisionShapeComponentDetail::CCD_FLAG ? true : false;
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

    SheduleUpdate();
}

void CollisionShapeComponent::CopyFields(CollisionShapeComponent* component)
{
    component->name = name;
    component->localPose = localPose;
    component->overrideMass = overrideMass;
    component->mass = mass;
}

void CollisionShapeComponent::SheduleUpdate()
{
    if (shape != nullptr)
    {
        Entity* entity = GetEntity();
        DVASSERT(entity != nullptr);
        Scene* scene = entity->GetScene();
        DVASSERT(scene != nullptr);
        scene->physicsSystem->SheduleUpdate(this);
    }
}

void CollisionShapeComponent::UpdateLocalProperties()
{
    DVASSERT(shape != nullptr);
    shape->setName(name.c_str());
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Matrix4ToPxMat44(localPose)));
    if (overrideMass == false)
    {
        physx::PxMassProperties massProperties = physx::PxRigidBodyExt::computeMassPropertiesFromShapes(&shape, 1);
        mass = massProperties.mass;
    }
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
    .Field("Override mass", &CollisionShapeComponent::GetOverrideMass, &CollisionShapeComponent::SetOverrideMass)
    .Field("Mass", &CollisionShapeComponent::GetMass, &CollisionShapeComponent::SetMass)
    .End();
}

} // namespace DAVA