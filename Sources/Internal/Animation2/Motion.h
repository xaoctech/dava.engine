#pragma once

#include "AnimationHierarchy.h"
#include "Math/Vector.h"

class Motion
{
public:
    typedef int NodeID;

    void Reset();
    NodeID AddAnimation(const char* clipName);
    NodeID AddBlend2();
    void LinkNodes(NodeID parent, NodeID child);

    void Update(float dt_s, DAVA::Vector3 offset);

    void GetPose(Skeleton::Pose* pose);

    static const NodeID InvalidNodeID = (NodeID)(-1);

private:
    void EnsureFlatten();

    struct Node
    {
        enum Type
        {
            NODE_POSE,
            NODE_ANIMATION,
            NODE_BLEND2
        };

        Type type;
    };
};
