#pragma once

#include "Animation2/AnimationTrack.h"
#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Math/AABBox3.h"

namespace DAVA
{
class AnimationClip;
class Entity;
class SkeletonSystem;
class SkeletonPose;
class SkeletonComponent : public Component
{
    friend class SkeletonSystem;

public:
    IMPLEMENT_COMPONENT_TYPE(SKELETON_COMPONENT);

    const static uint32 INVALID_JOINT_INDEX = 0xfff; //same as INFO_PARENT_MASK
    const static uint32 MAX_TARGET_JOINTS = 64; //same as in shader

    struct JointTransform
    {
        JointTransform() = default;
        JointTransform(const Vector3& position, const Quaternion& orientation, float32 scale = 1.f);

        Quaternion orientation;
        Vector3 position;
        float32 scale = 1.f;

        void Construct(const Matrix4& transform);
        JointTransform AppendTransform(const JointTransform& transform) const;
        JointTransform GetInverse() const;
        Vector3 TransformPoint(const Vector3& inVec) const;
        AABBox3 TransformAABBox(const AABBox3& bbox) const;
    };

    struct Joint : public InspBase
    {
        uint32 parentIndex = INVALID_JOINT_INDEX;
        uint32 targetIndex = INVALID_JOINT_INDEX;
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

    SkeletonComponent();
    ~SkeletonComponent();

    uint32 GetJointIndex(const FastName& uid) const;
    uint32 GetJointsCount() const;
    const Joint& GetJoint(uint32 i);

    void SetJoints(const Vector<Joint>& config);

    const JointTransform& GetJointTransform(uint32 jointIndex) const;
    const JointTransform& GetJointObjectSpaceTransform(uint32 jointIndex) const;

    void SetJointTransform(uint32 jointIndex, const JointTransform& transform);

    void ApplyPose(const SkeletonPose* pose);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void Rebuild();

private:
    const FilePath& GetAnimationPath() const;
    void SetAnimationPath(const FilePath& path);

    /*config time*/
    Vector<Joint> jointsArray;

    /*runtime*/
    const static uint32 INFO_PARENT_MASK = 0xfff;
    const static uint32 INFO_TARGET_SHIFT = 12;
    const static uint32 INFO_FLAG_BASE = 0x1000000;
    const static uint32 FLAG_UPDATED_THIS_FRAME = INFO_FLAG_BASE << 0;
    const static uint32 FLAG_MARKED_FOR_UPDATED = INFO_FLAG_BASE << 1;

    uint32 targetJointsCount = 0u; //amount of joints bound to skinnedMesh
    Vector<uint32> jointInfo; //flags and parent
    //transforms info
    Vector<JointTransform> localSpaceTransforms;
    Vector<JointTransform> objectSpaceTransforms;
    //bind pose
    Vector<JointTransform> inverseBindTransforms;
    //bounding boxes for bone
    Vector<AABBox3> jointSpaceBoxes;
    Vector<AABBox3> objectSpaceBoxes;

    Vector<Vector4> resultPositions; //stores final results
    Vector<Vector4> resultQuaternions;

    Map<FastName, uint32> jointMap;

    AnimationClip* animationClip = nullptr;
    Vector<std::pair<const AnimationTrack*, AnimationTrack::State>> animationStates;
    FilePath animationPath;

    uint32 startJoint = 0u; //first joint in the list that was updated this frame - cache this value to optimize processing
    bool configUpdated = true;
    bool drawSkeleton = false;

public:
    INTROSPECTION_EXTEND(SkeletonComponent, Component,
                         COLLECTION(jointsArray, "Root Joints", I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("animation", "animation", GetAnimationPath, SetAnimationPath, I_SAVE | I_VIEW | I_EDIT)
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
        return uint32(INVALID_JOINT_INDEX);
}

inline uint32 SkeletonComponent::GetJointsCount() const
{
    return uint32(jointsArray.size());
}

inline const SkeletonComponent::Joint& SkeletonComponent::GetJoint(uint32 i)
{
    return jointsArray[i];
}

inline void SkeletonComponent::SetJoints(const Vector<Joint>& config)
{
    jointsArray = config;
    configUpdated = true;
}

inline const SkeletonComponent::JointTransform& SkeletonComponent::GetJointTransform(uint32 jointIndex) const
{
    DVASSERT(jointIndex < GetJointsCount());
    return localSpaceTransforms[jointIndex];
}

inline const SkeletonComponent::JointTransform& SkeletonComponent::GetJointObjectSpaceTransform(uint32 jointIndex) const
{
    DVASSERT(jointIndex < objectSpaceTransforms.size());
    return objectSpaceTransforms[jointIndex];
}

inline void SkeletonComponent::SetJointTransform(uint32 jointIndex, const SkeletonComponent::JointTransform& transform)
{
    DVASSERT(jointIndex < GetJointsCount());

    jointInfo[jointIndex] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointIndex] = transform;
    startJoint = Min(startJoint, jointIndex);
}

inline void SkeletonComponent::Rebuild()
{
    configUpdated = true;
}

inline Vector3 SkeletonComponent::JointTransform::TransformPoint(const Vector3& inVec) const
{
    return position + orientation.ApplyToVectorFast(inVec) * scale;
}

inline AABBox3 SkeletonComponent::JointTransform::TransformAABBox(const AABBox3& bbox) const
{
    const Vector3& min = bbox.min;
    const Vector3& max = bbox.max;

    AABBox3 res;
    res.AddPoint(TransformPoint(min));
    res.AddPoint(TransformPoint(max));
    res.AddPoint(TransformPoint(Vector3(min.x, min.y, max.z)));
    res.AddPoint(TransformPoint(Vector3(min.x, max.y, min.z)));
    res.AddPoint(TransformPoint(Vector3(min.x, max.y, max.z)));
    res.AddPoint(TransformPoint(Vector3(max.x, min.y, min.z)));
    res.AddPoint(TransformPoint(Vector3(max.x, min.y, max.z)));
    res.AddPoint(TransformPoint(Vector3(max.x, max.y, min.z)));

    return res;
}

inline void SkeletonComponent::JointTransform::Construct(const Matrix4& transform)
{
    Vector3 scale3;
    transform.Decomposition(position, scale3, orientation);
    scale = scale3.x;
}

inline SkeletonComponent::JointTransform SkeletonComponent::JointTransform::AppendTransform(const JointTransform& transform) const
{
    JointTransform res;
    res.position = TransformPoint(transform.position);
    res.orientation = orientation * transform.orientation;
    res.scale = scale * transform.scale;
    return res;
}

inline SkeletonComponent::JointTransform SkeletonComponent::JointTransform::GetInverse() const
{
    JointTransform res;
    res.scale = 1.0f / scale;
    res.orientation = orientation;
    res.orientation.Inverse();
    res.position = -res.orientation.ApplyToVectorFast(position) * res.scale;

    return res;
}

template <>
bool AnyCompare<SkeletonComponent::Joint>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<SkeletonComponent::Joint>;

} //ns
