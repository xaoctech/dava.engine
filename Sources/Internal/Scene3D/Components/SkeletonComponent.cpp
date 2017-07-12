#include "Animation2/AnimationClip.h"
#include "Animation2/AnimationTrack.h"
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

SkeletonComponent::JointTransform::JointTransform(const Vector3& _position, const Quaternion& _orientation, float32 _scale)
    : orientation(_orientation)
    , position(_position)
    , scale(_scale)
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

SkeletonComponent::SkeletonComponent()
{
}

SkeletonComponent::~SkeletonComponent()
{
    SafeDelete(animationClip);
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
        jointArch->SetUInt32("joint.parentIndex", joint.parentIndex);
        jointArch->SetUInt32("joint.targetIndex", joint.targetIndex);
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
        joint.parentIndex = jointArch->GetUInt32("joint.parentIndex", INVALID_JOINT_INDEX);
        joint.targetIndex = jointArch->GetUInt32("joint.targetIndex", INVALID_JOINT_INDEX);
        joint.bbox.min = jointArch->GetVector3("joint.bbox.min");
        joint.bbox.max = jointArch->GetVector3("joint.bbox.max");
        joint.bindTransform = jointArch->GetMatrix4("joint.bindPose");
        joint.bindTransformInv = jointArch->GetMatrix4("joint.invBindPose");
    }
}

const FilePath& SkeletonComponent::GetAnimationPath() const
{
    return animationPath;
}

void SkeletonComponent::SetAnimationPath(const FilePath& path)
{
    SafeDelete(animationClip);
    animationStates.clear();

    animationPath = path;
    if (!animationPath.IsEmpty())
    {
        animationClip = new AnimationClip();
        if (animationClip->Load(animationPath))
        {
            animationStates.resize(jointsArray.size());

            uint32 trackCount = animationClip->GetTrackCount();
            for (const Joint& j : jointsArray)
            {
                for (uint32 t = 0; t < trackCount; ++t)
                {
                    if (strcmp(animationClip->GetTrackUID(t), j.uid.c_str()) == 0)
                    {
                        const AnimationTrack* track = animationClip->GetTrack(t);
                        animationStates[j.targetIndex] = std::make_pair(track, AnimationTrack::State(track->GetChannelsCount()));

                        track->Reset(&animationStates[j.targetIndex].second);
                    }
                }
            }
        }
        else
        {
            SafeDelete(animationClip);
        }
    }
}

template <>
bool AnyCompare<SkeletonComponent::Joint>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<SkeletonComponent::Joint>() == v2.Get<SkeletonComponent::Joint>();
}
}