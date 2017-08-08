#pragma once

#include "Animation/AnimationTrack.h"
#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Math/AABBox3.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/SkeletonAnimation/JointTransform.h"
#include "Scene3D/SkeletonAnimation/SkeletonPose.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class AnimationClip;
class Entity;
class SkeletonSystem;
class SkeletonComponent : public Component
{
    friend class SkeletonSystem;

public:
    IMPLEMENT_COMPONENT_TYPE(SKELETON_COMPONENT);

    const static uint32 INVALID_JOINT_INDEX = 0xffffff; //same as INFO_PARENT_MASK

    struct Joint : public InspBase
    {
        uint32 parentIndex = INVALID_JOINT_INDEX;
        FastName name;
        FastName uid;
        AABBox3 bbox;

        Matrix4 bindTransform;
        Matrix4 bindTransformInv;

        bool operator==(const Joint& other) const;

        INTROSPECTION(Joint,
                      MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(uid, "UID", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(parentIndex, "Parent Index", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(bbox, "Bounding box", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(bindTransformInv, "invBindPos", I_SAVE | I_VIEW | I_EDIT)
                      );

        DAVA_VIRTUAL_REFLECTION(Joint, InspBase);
    };

    SkeletonComponent() = default;
    ~SkeletonComponent() = default;

    uint32 GetJointIndex(const FastName& uid) const;
    uint32 GetJointsCount() const;
    const Joint& GetJoint(uint32 jointIndex) const;

    void SetJoints(const Vector<Joint>& config);

    const JointTransform& GetJointTransform(uint32 jointIndex) const;
    const JointTransform& GetJointObjectSpaceTransform(uint32 jointIndex) const;

    SkeletonPose GetDefaultPose() const;
    void ApplyPose(const SkeletonPose& pose);
    void SetJointTransform(uint32 jointIndex, const JointTransform& transform);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    /*config time*/
    Vector<Joint> jointsArray;

    /*runtime*/
    const static uint32 INFO_PARENT_MASK = 0xffffff;
    const static uint32 INFO_FLAG_BASE = 0x1000000;
    const static uint32 FLAG_UPDATED_THIS_FRAME = INFO_FLAG_BASE << 0;
    const static uint32 FLAG_MARKED_FOR_UPDATED = INFO_FLAG_BASE << 1;

    Vector<uint32> jointInfo; //flags and parent
    //transforms info
    Vector<JointTransform> localSpaceTransforms;
    Vector<JointTransform> objectSpaceTransforms;
    Vector<JointTransform> finalTransforms;
    //bind pose
    Vector<JointTransform> inverseBindTransforms;
    //bounding boxes
    Vector<AABBox3> objectSpaceBoxes;

    Map<FastName, uint32> jointMap;

    uint32 startJoint = 0u; //first joint in the list that was updated this frame - cache this value to optimize processing
    bool configUpdated = true;
    bool drawSkeleton = false;

public:
    INTROSPECTION_EXTEND(SkeletonComponent, Component,
                         COLLECTION(jointsArray, "Root Joints", I_SAVE | I_VIEW | I_EDIT)
                         MEMBER(drawSkeleton, "Draw Skeleton", I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(SkeletonComponent, Component);

    friend class SkeletonSystem;
};

inline uint32 SkeletonComponent::GetJointIndex(const FastName& uid) const
{
    Map<FastName, uint32>::const_iterator it = jointMap.find(uid);
    if (jointMap.end() != it)
        return it->second;
    else
        return INVALID_JOINT_INDEX;
}

inline uint32 SkeletonComponent::GetJointsCount() const
{
    return uint32(jointsArray.size());
}

inline const SkeletonComponent::Joint& SkeletonComponent::GetJoint(uint32 jointIndex) const
{
    DVASSERT(jointIndex < GetJointsCount());
    return jointsArray[jointIndex];
}

inline const JointTransform& SkeletonComponent::GetJointTransform(uint32 jointIndex) const
{
    DVASSERT(jointIndex < GetJointsCount());
    return localSpaceTransforms[jointIndex];
}

inline const JointTransform& SkeletonComponent::GetJointObjectSpaceTransform(uint32 jointIndex) const
{
    DVASSERT(jointIndex < objectSpaceTransforms.size());
    return objectSpaceTransforms[jointIndex];
}

inline void SkeletonComponent::SetJointTransform(uint32 jointIndex, const JointTransform& transform)
{
    DVASSERT(jointIndex < GetJointsCount());

    jointInfo[jointIndex] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointIndex] = transform;
    startJoint = Min(startJoint, jointIndex);
}

template <>
bool AnyCompare<SkeletonComponent::Joint>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<SkeletonComponent::Joint>;

} //ns
