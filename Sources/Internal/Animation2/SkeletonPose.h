#pragma once

#include "Animation2/JointTransform.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class SkeletonPose
{
public:
    SkeletonPose(uint32 nodeCount = 0);

    void SetNodeCount(uint32 count);
    uint32 GetNodeCount() const;

    void AddNode(uint32 jointIndex, const JointTransform& transform);

    void SetJointIndex(uint32 nodeIndex, uint32 jointIndex);
    void SetTransform(uint32 nodeIndex, const JointTransform& transform);

    uint32 GetJointIndex(uint32 nodeIndex) const;
    const JointTransform& GetJointTransform(uint32 nodeIndex) const;

    void Add(const SkeletonPose& pose);

    static SkeletonPose Blend(const SkeletonPose& p0, const SkeletonPose& p1, float32 ratio);

private:
    Vector<std::pair<uint32, JointTransform>> nodes; // [jointIndex, jointTransform]
};

inline void SkeletonPose::SetNodeCount(uint32 count)
{
    nodes.resize(count);
}

inline uint32 SkeletonPose::GetNodeCount() const
{
    return uint32(nodes.size());
}

inline void SkeletonPose::SetJointIndex(uint32 nodeIndex, uint32 jointIndex)
{
    DVASSERT(nodeIndex < GetNodeCount());
    nodes[nodeIndex].first = jointIndex;
}

inline void SkeletonPose::AddNode(uint32 jointIndex, const JointTransform& transform)
{
    nodes.emplace_back(std::make_pair(jointIndex, transform));
}

inline void SkeletonPose::SetTransform(uint32 nodeIndex, const JointTransform& transform)
{
    DVASSERT(nodeIndex < GetNodeCount());
    nodes[nodeIndex].second = transform;
}

inline uint32 SkeletonPose::GetJointIndex(uint32 nodeIndex) const
{
    DVASSERT(nodeIndex < GetNodeCount());
    return nodes[nodeIndex].first;
}

inline const JointTransform& SkeletonPose::GetJointTransform(uint32 nodeIndex) const
{
    DVASSERT(nodeIndex < GetNodeCount());
    return nodes[nodeIndex].second;
}

} //ns