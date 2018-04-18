#pragma once

namespace DAVA
{
class SkeletonComponent;

struct SkeletonUtils final
{
    static void UpdateJointTransforms(SkeletonComponent* skeletonComponent);
};
}