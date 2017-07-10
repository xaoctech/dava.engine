#include "Motion.h"

void Motion::Reset()
{
}

Motion::NodeID Motion::AddAnimation(const char* clipName)
{
    return InvalidNodeID;
}

Motion::NodeID Motion::AddBlend2()
{
    return InvalidNodeID;
}

void Motion::LinkNodes(NodeID parent, NodeID child)
{
}

void Motion::Update(float dt_s, DAVA::Vector3 offset)
{
}

void Motion::GetPose(Skeleton::Pose* pose)
{
}

void Motion::EnsureFlatten()
{
}
