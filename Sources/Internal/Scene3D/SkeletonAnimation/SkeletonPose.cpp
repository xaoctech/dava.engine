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

        //if (p0.IsJointTransformUsed(j) && p1.IsJointTransformUsed(j))
        //{
        //    resultPose.SetTransform(j, transform0.AppendTransform(transform1));
        //}
        //else if (!p0.IsJointTransformUsed(j) && p1.IsJointTransformUsed(j))
        //{
        //    resultPose.SetTransform(j, transform1);
        //}
        //else if (p0.IsJointTransformUsed(j) && !p1.IsJointTransformUsed(j))
        //{
        //    resultPose.SetTransform(j, transform0);
        //}
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
        uint32 p0Flags = p0.GetJointTransformFlags(j);
        uint32 p1Flags = p1.GetJointTransformFlags(j);

        if ((p0Flags & FLAG_POSITION) && (p1Flags & FLAG_POSITION))
        {
            resultPose.SetPosition(j, DAVA::Lerp<Vector3>(transform0.position, transform1.position, factor));
        }
        else if (!(p0Flags & FLAG_POSITION) && (p1Flags & FLAG_POSITION))
        {
            resultPose.SetPosition(j, transform1.position);
        }
        else if ((p0Flags & FLAG_POSITION) && !(p1Flags & FLAG_POSITION))
        {
            resultPose.SetPosition(j, transform0.position);
        }

        if ((p0Flags & FLAG_ORIENTATION) && (p1Flags & FLAG_ORIENTATION))
        {
            Quaternion sLerp;
            sLerp.Slerp(transform0.orientation, transform1.orientation, factor);
            sLerp.Normalize();
            resultPose.SetOrientation(j, sLerp);
        }
        else if (!(p0Flags & FLAG_ORIENTATION) && (p1Flags & FLAG_ORIENTATION))
        {
            resultPose.SetOrientation(j, transform1.orientation);
        }
        else if ((p0Flags & FLAG_ORIENTATION) && !(p1Flags & FLAG_ORIENTATION))
        {
            resultPose.SetOrientation(j, transform0.orientation);
        }

        if ((p0Flags & FLAG_SCALE) && (p1Flags & FLAG_SCALE))
        {
            resultPose.SetScale(j, DAVA::Lerp<float32>(transform0.scale, transform1.scale, factor));
        }
        else if (!(p0Flags & FLAG_SCALE) && (p1Flags & FLAG_SCALE))
        {
            resultPose.SetScale(j, transform1.scale);
        }
        else if ((p0Flags & FLAG_SCALE) && !(p1Flags & FLAG_SCALE))
        {
            resultPose.SetScale(j, transform0.scale);
        }
    }

    return resultPose;
}

} //ns