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
    DVASSERT(p0.GetJointsCount() == p1.GetJointsCount());

    SkeletonPose resultPose;
    resultPose.SetJointCount(p0.GetJointsCount());
    for (uint32 j = 0; j < resultPose.GetJointsCount(); ++j)
    {
        const JointTransform transform0 = p0.GetJointTransform(j);
        const JointTransform transform1 = p1.GetJointTransform(j);

        if (p0.IsJointTransformUsed(j) && p1.IsJointTransformUsed(j))
        {
            resultPose.SetTransform(j, transform0.AppendTransform(transform1));
        }
        else if (!p0.IsJointTransformUsed(j) && p1.IsJointTransformUsed(j))
        {
            resultPose.SetTransform(j, transform1);
        }
        else if (p0.IsJointTransformUsed(j) && !p1.IsJointTransformUsed(j))
        {
            resultPose.SetTransform(j, transform0);
        }
    }

    return resultPose;
}

SkeletonPose SkeletonPose::Sub(const SkeletonPose& p0, const SkeletonPose& p1)
{
    DVASSERT(false);
    return SkeletonPose();
}

SkeletonPose SkeletonPose::Lerp(const SkeletonPose& p0, const SkeletonPose& p1, float32 factor)
{
    uint32 maxJoints = Max(p0.GetJointsCount(), p1.GetJointsCount());
    SkeletonPose resultPose(maxJoints);
    for (uint32 j = 0; j < resultPose.GetJointsCount(); ++j)
    {
        const JointTransform transform0 = p0.GetJointTransform(j);
        const JointTransform transform1 = p1.GetJointTransform(j);

        if (p0.IsJointTransformUsed(j) && p1.IsJointTransformUsed(j))
        {
            JointTransform transform;
            transform.position.Lerp(transform0.position, transform1.position, factor);
            transform.orientation.Slerp(transform0.orientation, transform1.orientation, factor);
            transform.orientation.Normalize();
            transform.scale = DAVA::Lerp(transform0.scale, transform1.scale, factor);

            resultPose.SetTransform(j, transform);
        }
        else if (!p0.IsJointTransformUsed(j) && p1.IsJointTransformUsed(j))
        {
            resultPose.SetTransform(j, transform1);
        }
        else if (p0.IsJointTransformUsed(j) && !p1.IsJointTransformUsed(j))
        {
            resultPose.SetTransform(j, transform0);
        }
    }

    return resultPose;
}

} //ns