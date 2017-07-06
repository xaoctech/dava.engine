#ifndef __DAVAENGINE_SKELETON_COMPONENT_H__
#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Math/AABBox3.h"

namespace DAVA
{
class Entity;
class SkeletonSystem;
class SkeletonComponent : public Component
{
    friend class SkeletonSystem;

public:
    IMPLEMENT_COMPONENT_TYPE(SKELETON_COMPONENT);

    const static uint16 INVALID_JOINT_INDEX = 0xff; //same as INFO_PARENT_MASK
    const static uint16 MAX_TARGET_JOINTS = 64; //same as in shader

    struct JointTransform
    {
        Quaternion orientation;

        Vector3 position;
        float32 scale;

        void Construct(const Matrix4& transform);
        JointTransform AppendTransform(const JointTransform& transform) const;
        JointTransform GetInverse() const;
        Vector3 TransformPoint(const Vector3& inVec) const;
        AABBox3 TransformAABBox(const AABBox3& bbox) const;
    };

    struct JointConfig : public InspBase
    {
        JointConfig() = default;
        JointConfig(int32 parentIndex, int32 targetId, const FastName& name, const FastName& uid, const Vector3& position, const Quaternion& orientation, float32 scale, const AABBox3& bbox, const Matrix4& _invBindPose);

        int32 parentIndex = INVALID_JOINT_INDEX;
        int32 targetId = INVALID_JOINT_INDEX;
        FastName name;
        FastName uid;
        //TODO: *Skinning* remove local transforms from JointConfig. Keep bindTransformInv only
        Quaternion orientation;
        Vector3 position;
        float32 scale = 1.f;
        AABBox3 bbox;
        Matrix4 bindTransformInv;

        bool operator==(const JointConfig& other) const;

        INTROSPECTION(JointConfig,
                      MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(uid, "UID", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(parentIndex, "Parent Index", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(bbox, "Bounding box", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(bindTransformInv, "invBindPos", I_SAVE | I_VIEW | I_EDIT)
                      );

        DAVA_VIRTUAL_REFLECTION(JointConfig, InspBase);
    };

    SkeletonComponent() = default;

    void RebuildFromConfig();
    void SetConfigJoints(const Vector<JointConfig>& config);

    uint32 GetConfigJointsCount();
    const JointConfig& GetConfigJoint(uint32 i);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetJointPosition(uint16 jointId, const Vector3& position);
    void SetJointOrientation(uint16 jointId, const Quaternion& orientation);
    void SetJointScale(uint16 jointId, float32 scale);

    uint16 GetJointId(const FastName& name) const;
    uint16 GetJointsCount() const;

    const FastName& GetJointName(uint16 jointId) const;
    const JointTransform& GetObjectSpaceTransform(uint16 jointId) const;

private:
    /*config time*/
    Vector<JointConfig> configJoints;

    /*runtime*/
    const static uint32 INFO_PARENT_MASK = 0xff;
    const static uint32 INFO_TARGET_SHIFT = 8;
    const static uint32 INFO_FLAG_BASE = 0x10000;
    const static uint32 FLAG_UPDATED_THIS_FRAME = INFO_FLAG_BASE << 0;
    const static uint32 FLAG_MARKED_FOR_UPDATED = INFO_FLAG_BASE << 1;

    uint16 jointsCount = 0u;
    uint16 targetJointsCount = 0u; //amount of joints bound to skinnedMesh
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

    Map<FastName, uint16> jointMap;

    uint16 startJoint = 0u; //first joint in the list that was updated this frame - cache this value to optimize processing
    bool configUpdated = true;
    bool drawSkeleton = false;

public:
    INTROSPECTION_EXTEND(SkeletonComponent, Component,
                         COLLECTION(configJoints, "Root Joints", I_SAVE | I_VIEW | I_EDIT)
                         MEMBER(drawSkeleton, "Draw Skeleton", I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(SkeletonComponent, Component);

    friend class SkeletonSystem;
};

inline void SkeletonComponent::SetJointPosition(uint16 jointId, const Vector3& position)
{
    DVASSERT(jointId < GetJointsCount());
    jointInfo[jointId] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointId].position = position;
    startJoint = Min(startJoint, jointId);
}

inline void SkeletonComponent::SetJointOrientation(uint16 jointId, const Quaternion& orientation)
{
    DVASSERT(jointId < GetJointsCount());
    jointInfo[jointId] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointId].orientation = orientation;
    startJoint = Min(startJoint, jointId);
}

inline void SkeletonComponent::SetJointScale(uint16 jointId, float32 scale)
{
    DVASSERT(jointId < GetJointsCount());
    jointInfo[jointId] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointId].scale = scale;
    startJoint = Min(startJoint, jointId);
}

inline uint16 SkeletonComponent::GetJointId(const FastName& name) const
{
    Map<FastName, uint16>::const_iterator it = jointMap.find(name);
    if (jointMap.end() != it)
        return it->second;
    else
        return INVALID_JOINT_INDEX;
}

inline uint16 SkeletonComponent::GetJointsCount() const
{
    return jointsCount;
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
bool AnyCompare<SkeletonComponent::JointConfig>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<SkeletonComponent::JointConfig>;

} //ns

#endif