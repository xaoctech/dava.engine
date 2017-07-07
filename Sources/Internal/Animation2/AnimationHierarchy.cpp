#include "AnimationHierarchy.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"

Skeleton::Skeleton()
{
}

bool Skeleton::Load(const char* fileName)
{
    return false;
}

unsigned Skeleton::NodeCount() const
{
    return nodeCount;
}

const char* Skeleton::NodeId(unsigned node_i) const
{
    DVASSERT(node_i < nodeCount);
    return nodeId[node_i];
}
const DAVA::Matrix4& Skeleton::NodeBindMatrix(unsigned node_i) const
{
    DVASSERT(node_i < nodeCount);
    return bindMatrix[node_i];
}
const DAVA::Matrix4& Skeleton::NodeInvBindMatrix(unsigned node_i) const
{
    DVASSERT(node_i < nodeCount);
    return invBindMatrix[node_i];
}

void Skeleton::ApplyPose(const Skeleton::Pose& pose)
{
}

void Skeleton::ComputeBoneWeights()
{
}

Skeleton::Pose::Pose()
    : nodeCount(0)
    ,
    transform(nullptr)
    ,
    rotation(nullptr)
    ,
    translation(nullptr)
{
}

void Skeleton::Pose::SetNodeTransform(unsigned node_i, DAVA::Quaternion q, DAVA::Vector3 t)
{
}

const DAVA::Matrix4* Skeleton::Pose::NodeTransforms() const
{
    return transform;
}

void Skeleton::Pose::Blend(Skeleton::Pose* dst, const Skeleton::Pose& src1, const Skeleton::Pose& src2, float ratio)
{
}

void Skeleton::Pose::Add(Skeleton::Pose* dst, const Skeleton::Pose& src)
{
}
