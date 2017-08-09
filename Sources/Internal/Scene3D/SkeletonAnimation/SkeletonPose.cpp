#include "SkeletonPose.h"
#include "Math/MathHelpers.h"

namespace DAVA
{
SkeletonPose::SkeletonPose(uint32 jointCount)
{
    SetJointCount(jointCount);
}

SkeletonPose SkeletonPose::Add(const SkeletonPose& p0, const SkeletonPose& p1)
{
    uint32 maxJoints = Max(p0.GetJointsCount(), p1.GetJointsCount());
    SkeletonPose resultPose(maxJoints);
    for (uint32 j = 0; j < resultPose.GetJointsCount(); ++j)
    {
        const JointTransform transform0 = p0.GetJointTransform(j);
        const JointTransform transform1 = p1.GetJointTransform(j);
        resultPose.SetTransform(j, transform0.AppendTransform(transform1));
    }

    return resultPose;
}

SkeletonPose SkeletonPose::Sub(const SkeletonPose& p0, const SkeletonPose& p1)
{
    uint32 maxJoints = Max(p0.GetJointsCount(), p1.GetJointsCount());
    SkeletonPose resultPose(maxJoints);
    for (uint32 j = 0; j < resultPose.GetJointsCount(); ++j)
    {
        const JointTransform transform0 = p0.GetJointTransform(j);
        const JointTransform transform1 = p1.GetJointTransform(j);
        resultPose.SetTransform(j, transform0.AppendTransform(transform1.GetInverse()));
    }

    return resultPose;
}

SkeletonPose SkeletonPose::Lerp(const SkeletonPose& p0, const SkeletonPose& p1, float32 factor)
{
    uint32 maxJoints = Max(p0.GetJointsCount(), p1.GetJointsCount());
    SkeletonPose resultPose(maxJoints);
    for (uint32 j = 0; j < resultPose.GetJointsCount(); ++j)
    {
        const JointTransform transform0 = p0.GetJointTransform(j);
        const JointTransform transform1 = p1.GetJointTransform(j);
        resultPose.SetTransform(j, JointTransform::Lerp(transform0, transform1, factor));
    }

    return resultPose;
}

} //ns