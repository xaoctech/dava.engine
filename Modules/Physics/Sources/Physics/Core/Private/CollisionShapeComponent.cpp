#include "Physics/Core/CollisionShapeComponent.h"
#include "PhysicsMath.h"
#include "Physics/PhysicsSystem.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxShape.h>

ENUM_DECLARE(DAVA::CollisionShapeComponent::JointSyncDirection)
{
    ENUM_ADD_DESCR(DAVA::CollisionShapeComponent::JointSyncDirection::FromPhysics, "From physics");
    ENUM_ADD_DESCR(DAVA::CollisionShapeComponent::JointSyncDirection::ToPhysics, "To physics");
}

namespace DAVA
{
namespace CollisionShapeComponentDetails
{
enum
{
    // additional flags of shape that are used in physx shader function for collision detection
    // values are mapped on physx::PxFilterData::word0
    // first bit of physx::PxFilterData::word0 signals is CCD enabled for shape of not
    CCD_FLAG = 1,
};
} // CollisionShapeComponentDetails

void CollisionShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetFastName("shape.name", name);
    archive->SetVector3("shape.localPosition", localPosition);
    archive->SetQuaternion("shape.localOrientation", localOrientation);
    archive->SetBool("shape.overrideMass", overrideMass);
    archive->SetFloat("shape.mass", mass);
    archive->SetUInt32("shape.typeMask", typeMask);
    archive->SetUInt32("shape.typeMaskToCollideWith", typeMaskToCollideWith);

    if (jointName.IsValid())
    {
        archive->SetFastName("shape.jointName", jointName);
    }

    archive->SetUInt32("shape.jointSyncDirection", static_cast<uint32>(jointSyncDirection));
    archive->SetVector3("shape.jointOffset", jointOffset);

    if (materialName.IsValid())
    {
        archive->SetFastName("shape.materialName", materialName);
    }

    archive->SetUInt32("shape.triggerMode", triggerMode);
}

void CollisionShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    name = archive->GetFastName("shape.name", FastName(""));
    localPosition = archive->GetVector3("shape.localPosition", localPosition);
    localOrientation = archive->GetQuaternion("shape.localOrientation", localOrientation);
    overrideMass = archive->GetBool("shape.overrideMass", overrideMass);
    mass = archive->GetFloat("shape.mass", mass);
    typeMask = archive->GetUInt32("shape.typeMask", typeMask);
    typeMaskToCollideWith = archive->GetUInt32("shape.typeMaskToCollideWith", typeMaskToCollideWith);
    materialName = archive->GetFastName("shape.materialName", materialName);
    triggerMode = archive->GetUInt32("shape.triggerMode", triggerMode);
    jointName = archive->GetFastName("shape.jointName", jointName);
    jointSyncDirection = static_cast<JointSyncDirection>(archive->GetUInt32("shape.jointSyncDirection", jointSyncDirection));
    jointOffset = archive->GetVector3("shape.jointOffset", jointOffset);
}

physx::PxShape* CollisionShapeComponent::GetPxShape() const
{
    return shape;
}

const FastName& CollisionShapeComponent::GetName() const
{
    return name;
}

void CollisionShapeComponent::SetName(const FastName& name_)
{
    DVASSERT(name_.IsValid());
    name = name_;
    ScheduleUpdate();
}

const Vector3& CollisionShapeComponent::GetLocalPosition() const
{
    return localPosition;
}

void CollisionShapeComponent::SetLocalPosition(const Vector3& value)
{
    localPosition = value;
    if (shape != nullptr)
    {
        shape->setLocalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(localPosition), PhysicsMath::QuaternionToPxQuat(localOrientation)));
    }
    else
    {
        ScheduleUpdate();
    }
}

const Quaternion& CollisionShapeComponent::GetLocalOrientation() const
{
    return localOrientation;
}

void CollisionShapeComponent::SetLocalOrientation(const Quaternion& value)
{
    localOrientation = value;
    if (shape != nullptr)
    {
        shape->setLocalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(localPosition), PhysicsMath::QuaternionToPxQuat(localOrientation)));
    }
    else
    {
        ScheduleUpdate();
    }
}

bool CollisionShapeComponent::GetOverrideMass() const
{
    return overrideMass;
}

void CollisionShapeComponent::SetOverrideMass(bool override)
{
    overrideMass = override;
    ScheduleUpdate();
}

float32 CollisionShapeComponent::GetMass() const
{
    return mass;
}

void CollisionShapeComponent::SetMass(float32 mass_)
{
    if (overrideMass)
    {
        mass = mass_;
        ScheduleUpdate();
    }
}

void CollisionShapeComponent::SetTypeMask(uint32 typeMask_)
{
    if (typeMask != typeMask_)
    {
        typeMask = typeMask_;

        if (shape != nullptr)
        {
            UpdateFilterData();
        }
        else
        {
            ScheduleUpdate();
        }
    }
}

uint32 CollisionShapeComponent::GetTypeMask() const
{
    return typeMask;
}

void CollisionShapeComponent::SetTypeMaskToCollideWith(uint32 typeMaskToCollideWith_)
{
    if (typeMaskToCollideWith != typeMaskToCollideWith_)
    {
        typeMaskToCollideWith = typeMaskToCollideWith_;

        if (shape != nullptr)
        {
            UpdateFilterData();
        }
        else
        {
            ScheduleUpdate();
        }
    }
}

uint32 CollisionShapeComponent::GetTypeMaskToCollideWith() const
{
    return typeMaskToCollideWith;
}

const FastName& CollisionShapeComponent::GetMaterialName() const
{
    return materialName;
}

void CollisionShapeComponent::SetMaterialName(const FastName& materialName_)
{
    if (materialName != materialName_)
    {
        materialName = materialName_;
        ScheduleUpdate();
    }
}

void CollisionShapeComponent::SetTriggerMode(bool isEnabled)
{
    if (isEnabled != triggerMode)
    {
        triggerMode = isEnabled;
        ScheduleUpdate();
    }
}

bool CollisionShapeComponent::GetTriggerMode() const
{
    return triggerMode;
}

const FastName& CollisionShapeComponent::GetJointName() const
{
    return jointName;
}

void CollisionShapeComponent::SetJointName(const FastName& value)
{
    if (jointName != value)
    {
        jointName = value;
        ScheduleUpdate();
    }
}

CollisionShapeComponent::JointSyncDirection CollisionShapeComponent::GetJointSyncDirection() const
{
    return jointSyncDirection;
}

void CollisionShapeComponent::SetJointSyncDirection(CollisionShapeComponent::JointSyncDirection value)
{
    jointSyncDirection = value;
}

Vector3 CollisionShapeComponent::GetJointOffset() const
{
    return jointOffset;
}

void CollisionShapeComponent::SetJointOffset(const Vector3& value)
{
    jointOffset = value;
}

CollisionShapeComponent* CollisionShapeComponent::GetComponent(const physx::PxShape* shape)
{
    DVASSERT(shape != nullptr);
    return reinterpret_cast<CollisionShapeComponent*>(shape->userData);
}

void CollisionShapeComponent::SetCCDEnabled(physx::PxShape* shape, bool isCCDActive)
{
    DVASSERT(shape != nullptr);
    physx::PxFilterData fd = shape->getSimulationFilterData();
    physx::PxU32 ccdFlag = static_cast<physx::PxU32>(CollisionShapeComponentDetails::CCD_FLAG);
    if (isCCDActive == true)
    {
        fd.word0 |= ccdFlag;
    }
    else
    {
        fd.word0 = fd.word0 & (~ccdFlag);
    }
    shape->setSimulationFilterData(fd);
}

bool CollisionShapeComponent::IsCCDEnabled(const physx::PxFilterData& filterData)
{
    using namespace CollisionShapeComponentDetails;
    return (filterData.word0 & CCD_FLAG) == CCD_FLAG;
}

void CollisionShapeComponent::SetPxShape(physx::PxShape* shape_)
{
    DVASSERT(shape_ != nullptr);
    DVASSERT(shape == nullptr);
    shape = shape_;
    shape->userData = this;
    DVASSERT(name.IsValid());
    shape->setName(name.c_str());
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(localPosition), PhysicsMath::QuaternionToPxQuat(localOrientation)));
    shape->acquireReference();

#if defined(__DAVAENGINE_DEBUG__)
    CheckShapeType();
#endif

    ScheduleUpdate();
}

void CollisionShapeComponent::CopyFieldsIntoClone(CollisionShapeComponent* component) const
{
    component->name = name;
    component->localPosition = localPosition;
    component->localOrientation = localOrientation;
    component->overrideMass = overrideMass;
    component->mass = mass;
    component->typeMask = typeMask;
    component->typeMaskToCollideWith = typeMaskToCollideWith;
    component->materialName = materialName;
    component->triggerMode = triggerMode;
    component->jointName = jointName;
    component->jointSyncDirection = jointSyncDirection;
    component->jointOffset = jointOffset;
    component->scale = scale;
}

void CollisionShapeComponent::ScheduleUpdate()
{
    if (shape != nullptr)
    {
        Entity* entity = GetEntity();
        DVASSERT(entity != nullptr);
        Scene* scene = entity->GetScene();
        DVASSERT(scene != nullptr);
        scene->physicsSystem->ScheduleUpdate(this);
    }
}

void CollisionShapeComponent::UpdateFilterData()
{
    DVASSERT(shape != nullptr);

    // Setup word 0 to be the same for every shape since physx filters out shapes whose words do not intersect for some reason
    // See NpQueryShared.h, physx::applyFilterEquation function

    physx::PxFilterData filterData = shape->getSimulationFilterData();
    filterData.word0 |= (1 << 31);
    filterData.word1 = typeMask;
    filterData.word2 = typeMaskToCollideWith;
    // be careful and do not change first bit in filterData.word0 (CCD flag) as this flag is setting
    // directly from PhysicsSystem::UpdateComponents and should be synchronized with CCD flag of actor.
    shape->setSimulationFilterData(filterData);

    physx::PxFilterData queryFilterData = shape->getQueryFilterData();
    queryFilterData.word0 |= (1 << 31);
    queryFilterData.word1 = typeMask;
    queryFilterData.word2 = typeMaskToCollideWith;
    shape->setQueryFilterData(queryFilterData);
}

void CollisionShapeComponent::UpdateLocalProperties()
{
    DVASSERT(shape != nullptr);
    shape->setName(name.c_str());
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(localPosition), PhysicsMath::QuaternionToPxQuat(localOrientation)));
    if (overrideMass == false)
    {
        physx::PxMassProperties massProperties = physx::PxRigidBodyExt::computeMassPropertiesFromShapes(&shape, 1);
        mass = massProperties.mass;
    }

    UpdateFilterData();

    shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !triggerMode);
    shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, triggerMode);
}

void CollisionShapeComponent::ReleasePxShape()
{
    DVASSERT(shape != nullptr);
    shape->release();
    shape = nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CollisionShapeComponent)
{
    ReflectionRegistrator<CollisionShapeComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("Name", &CollisionShapeComponent::GetName, &CollisionShapeComponent::SetName)[M::Replicable()]
    .Field("material", &CollisionShapeComponent::GetMaterialName, &CollisionShapeComponent::SetMaterialName)[M::Replicable(), M::DisplayName("Material name")]
    .Field("Local position", &CollisionShapeComponent::GetLocalPosition, &CollisionShapeComponent::SetLocalPosition)[M::Replicable(), M::ComparePrecision(0.01f)]
    .Field("Local orientation", &CollisionShapeComponent::GetLocalOrientation, &CollisionShapeComponent::SetLocalOrientation)[M::Replicable(), M::ComparePrecision(0.01f)]
    .Field("OverrideMass", &CollisionShapeComponent::GetOverrideMass, &CollisionShapeComponent::SetOverrideMass)[M::Replicable()]
    .Field("Mass", &CollisionShapeComponent::GetMass, &CollisionShapeComponent::SetMass)[M::Replicable()]
    .Field("TypeMask", &CollisionShapeComponent::GetTypeMask, &CollisionShapeComponent::SetTypeMask)[M::Replicable(), M::DisplayName("Type")]
    .Field("TypeMaskToCollideWith", &CollisionShapeComponent::GetTypeMaskToCollideWith, &CollisionShapeComponent::SetTypeMaskToCollideWith)[M::Replicable(), M::DisplayName("Types to collide with")]
    .Field("Enable trigger mode", &CollisionShapeComponent::GetTriggerMode, &CollisionShapeComponent::SetTriggerMode)[M::Replicable()]
    .Field("Joint name", &CollisionShapeComponent::GetJointName, &CollisionShapeComponent::SetJointName)[M::Replicable()]
    .Field("Joint sync direction", &CollisionShapeComponent::GetJointSyncDirection, &CollisionShapeComponent::SetJointSyncDirection)[M::Replicable(), M::EnumT<CollisionShapeComponent::JointSyncDirection>()]
    .Field("Joint offset", &CollisionShapeComponent::GetJointOffset, &CollisionShapeComponent::SetJointOffset)[M::Replicable()]
    .End();
}

} // namespace DAVA
