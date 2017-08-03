#pragma once

#include "Base/BaseTypes.h"
#include "Scene3D/SkeletonAnimation/JointTransform.h"

namespace DAVA
{
class SkeletonPose
{
public:
    SkeletonPose(uint32 jointCount = 0);

    void SetJointCount(uint32 jointCount);
    uint32 GetJointsCount() const;

    void Reset();

    void SetTransform(uint32 jointIndex, const JointTransform& transform);
    const JointTransform& GetJointTransform(uint32 jointIndex) const;
    bool IsJointTransformUsed(uint32 jointIndex) const;

    static SkeletonPose Add(const SkeletonPose& p0, const SkeletonPose& p1);
    static SkeletonPose Sub(const SkeletonPose& p0, const SkeletonPose& p1);
    static SkeletonPose Lerp(const SkeletonPose& p0, const SkeletonPose& p1, float32 factor);

private:
    Vector<JointTransform> jointTransforms;
    Vector<bool> jointTransformUsed;
};

inline void SkeletonPose::SetJointCount(uint32 jointCount)
{
    jointTransforms.resize(jointCount, JointTransform());
    jointTransformUsed.resize(jointCount, false);
}

inline uint32 SkeletonPose::GetJointsCount() const
{
    return uint32(jointTransforms.size());
}

inline void SkeletonPose::Reset()
{
    std::fill(jointTransformUsed.begin(), jointTransformUsed.end(), false);
}

inline void SkeletonPose::SetTransform(uint32 jointIndex, const JointTransform& transform)
{
    if (GetJointsCount() <= jointIndex)
        SetJointCount(jointIndex + 1);

    jointTransforms[jointIndex] = transform;
    jointTransformUsed[jointIndex] = true;
}

inline const JointTransform& SkeletonPose::GetJointTransform(uint32 jointIndex) const
{
    DVASSERT(jointIndex < GetJointsCount());
    return jointTransforms[jointIndex];
}

inline bool SkeletonPose::IsJointTransformUsed(uint32 jointIndex) const
{
    DVASSERT(jointIndex < GetJointsCount());
    return jointTransformUsed[jointIndex];
}

} //ns