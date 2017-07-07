#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
REGISTER_CLASS(SkeletonComponent)

DAVA_VIRTUAL_REFLECTION_IMPL(SkeletonComponent::Joint)
{
    ReflectionRegistrator<SkeletonComponent::Joint>::Begin()
    .Field("name", &SkeletonComponent::Joint::name)[M::DisplayName("Name")]
    .Field("uid", &SkeletonComponent::Joint::uid)[M::DisplayName("UID")]
    .Field("bbox", &SkeletonComponent::Joint::bbox)[M::DisplayName("Bounding box")]
    .Field("bindTransformInv", &SkeletonComponent::Joint::bbox)[M::DisplayName("Bind Transform Inverse")]
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(SkeletonComponent)
{
    ReflectionRegistrator<SkeletonComponent>::Begin()
    .ConstructorByPointer()
    .Field("joints", &SkeletonComponent::jointsArray)[M::DisplayName("Joints")]
    .End();
}

SkeletonComponent::Joint::Joint(int32 _parentIndex, int32 _targetId, const FastName& _name, const FastName& _uid, const AABBox3& _bbox, const Matrix4& _bindPose, const Matrix4& _invBindPose)
    : parentIndex(_parentIndex)
    , targetIndex(_targetId)
    , name(_name)
    , uid(_uid)
    , bbox(_bbox)
    , bindTransform(_bindPose)
    , bindTransformInv(_invBindPose)
{
}

bool SkeletonComponent::Joint::operator==(const Joint& other) const
{
    return parentIndex == other.parentIndex &&
    targetIndex == other.targetIndex &&
    name == other.name &&
    uid == other.uid &&
    bindTransformInv == other.bindTransformInv;
    bindTransform == other.bindTransform&&
                     bbox == other.bbox;
}

void SkeletonComponent::ApplyPose(const SkeletonPose* pose)
{

}

Component* SkeletonComponent::Clone(Entity* toEntity)
{
    SkeletonComponent* newComponent = new SkeletonComponent();
    newComponent->SetEntity(toEntity);
    newComponent->jointsArray = jointsArray;
    return newComponent;
}
void SkeletonComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetUInt32("skeletoncomponent.jointsCount", static_cast<uint32>(jointsArray.size()));
    ScopedPtr<KeyedArchive> jointsArch(new KeyedArchive());
    for (size_t i = 0, sz = jointsArray.size(); i < sz; ++i)
    {
        const Joint& joint = jointsArray[i];
        ScopedPtr<KeyedArchive> jointArch(new KeyedArchive());
        jointArch->SetFastName("joint.name", joint.name);
        jointArch->SetFastName("joint.uid", joint.uid);
        jointArch->SetInt32("joint.parentIndex", joint.parentIndex);
        jointArch->SetInt32("joint.targetId", joint.targetIndex);
        jointArch->SetVector3("joint.bbox.min", joint.bbox.min);
        jointArch->SetVector3("joint.bbox.max", joint.bbox.max);
        jointArch->SetMatrix4("joint.bindPose", joint.bindTransform);
        jointArch->SetMatrix4("joint.invBindPose", joint.bindTransformInv);

        jointsArch->SetArchive(KeyedArchive::GenKeyFromIndex(static_cast<int32>(i)), jointArch);
    }

    archive->SetArchive("skeletoncomponent.joints", jointsArch);
}
void SkeletonComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    uint32 jointsCount = archive->GetUInt32("skeletoncomponent.jointsCount", static_cast<uint32>(jointsArray.size()));
    jointsArray.resize(jointsCount);
    KeyedArchive* jointsArch = archive->GetArchive("skeletoncomponent.joints");
    for (uint32 i = 0; i < jointsCount; ++i)
    {
        Joint& joint = jointsArray[i];
        KeyedArchive* jointArch = jointsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
        joint.name = jointArch->GetFastName("joint.name");
        joint.uid = jointArch->GetFastName("joint.uid");
        joint.parentIndex = jointArch->GetInt32("joint.parentIndex");
        joint.targetIndex = jointArch->GetInt32("joint.targetId");
        joint.bbox.min = jointArch->GetVector3("joint.bbox.min");
        joint.bbox.max = jointArch->GetVector3("joint.bbox.max");
        joint.bindTransform = jointArch->GetMatrix4("joint.bindPose");
        joint.bindTransformInv = jointArch->GetMatrix4("joint.invBindPose");
    }
}

template <>
bool AnyCompare<SkeletonComponent::Joint>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<SkeletonComponent::Joint>() == v2.Get<SkeletonComponent::Joint>();
}
}