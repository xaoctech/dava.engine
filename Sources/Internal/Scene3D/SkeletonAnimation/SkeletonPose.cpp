#include "SkeletonPose.h"

namespace DAVA
{
SkeletonPose::SkeletonPose(uint32 nodeCount)
{
    SetNodeCount(nodeCount);
}

void SkeletonPose::Add(const SkeletonPose& pose)
{
    //TODO
}

SkeletonPose SkeletonPose::Blend(const SkeletonPose& p0, const SkeletonPose& p1, float32 ratio)
{
    //TODO
    return p0;
}

} //ns