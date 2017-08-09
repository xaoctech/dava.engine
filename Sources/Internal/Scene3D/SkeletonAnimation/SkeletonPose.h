#pragma once

#include "Base/BaseTypes.h"
#include "Scene3D/SkeletonAnimation/JointTransform.h"

namespace DAVA
{
class SkeletonPose
{
public:
    enum eFlag : uint8
    {
        FLAG_POSITION = 1 << 0,
        FLAG_ORIENTATION = 1 << 1,
        FLAG_SCALE = 1 << 2
    };

    SkeletonPose(uint32 jointCount = 0);

    void SetJointCount(uint32 jointCount);
    uint32 GetJointsCount() const;

    void Reset();

    void SetTransform(uint32 jointIndex, const JointTransform& transform);

    void SetPosition(uint32 jointIndex, const Vector3& position);
    void SetOrientation(uint32 jointIndex, const Quaternion& orientation);
    void SetScale(uint32 jointIndex, float32 scale);

    const JointTransform& GetJointTransform(uint32 jointIndex) const;
    uint8 GetJointTransformFlags(uint32 jointIndex) const;

    static SkeletonPose Add(const SkeletonPose& p0, const SkeletonPose& p1);
    static SkeletonPose Sub(const SkeletonPose& p0, const SkeletonPose& p1);
    static SkeletonPose Lerp(const SkeletonPose& p0, const SkeletonPose& p1, float32 factor);

private:
    Vector<JointTransform> jointTransforms;
    Vector<uint8> jointTransformFlags;
};

inline void SkeletonPose::SetJointCount(uint32 jointCount)
{
    jointTransforms.resize(jointCount, JointTransform());
    jointTransformFlags.resize(jointCount, 0);
}

inline uint32 SkeletonPose::GetJointsCount() const
{
    return uint32(jointTransforms.size());
}

inline void SkeletonPose::Reset()
{
    std::fill(jointTransformFlags.begin(), jointTransformFlags.end(), 0);
}

inline void SkeletonPose::SetTransform(uint32 jointIndex, const JointTransform& transform)
{
    if (GetJointsCount() <= jointIndex)
        SetJointCount(jointIndex + 1);

    jointTransforms[jointIndex] = transform;
    jointTransformFlags[jointIndex] = FLAG_POSITION | FLAG_ORIENTATION | FLAG_SCALE;
}

inline void SkeletonPose::SetPosition(uint32 jointIndex, const Vector3& position)
{
    if (GetJointsCount() <= jointIndex)
        SetJointCount(jointIndex + 1);

    jointTransforms[jointIndex].position = position;
    jointTransformFlags[jointIndex] |= FLAG_POSITION;
}

inline void SkeletonPose::SetOrientation(uint32 jointIndex, const Quaternion& orientation)
{
    if (GetJointsCount() <= jointIndex)
        SetJointCount(jointIndex + 1);

    jointTransforms[jointIndex].orientation = orientation;
    jointTransformFlags[jointIndex] |= FLAG_ORIENTATION;
}

inline void SkeletonPose::SetScale(uint32 jointIndex, float32 scale)
{
    if (GetJointsCount() <= jointIndex)
        SetJointCount(jointIndex + 1);

    jointTransforms[jointIndex].scale = scale;
    jointTransformFlags[jointIndex] |= FLAG_SCALE;
}

inline const JointTransform& SkeletonPose::GetJointTransform(uint32 jointIndex) const
{
    DVASSERT(jointIndex < GetJointsCount());
    return jointTransforms[jointIndex];
}

inline uint8 SkeletonPose::GetJointTransformFlags(uint32 jointIndex) const
{
    DVASSERT(jointIndex < GetJointsCount());
    return jointTransformFlags[jointIndex];
}

} //ns