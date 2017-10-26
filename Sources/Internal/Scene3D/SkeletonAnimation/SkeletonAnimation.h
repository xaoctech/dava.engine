#pragma once

#include "Base/BaseTypes.h"
#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
class AnimationClip;

class SkeletonAnimation
{
public:
    SkeletonAnimation(AnimationClip* animationClip, const UnorderedSet<uint32>& ignoreMask = UnorderedSet<uint32>());
    ~SkeletonAnimation();

    void BindSkeleton(const SkeletonComponent* skeleton);
    void BindRootNode(const FastName& rootNodeID);

    void EvaluatePose(float32 localTime, SkeletonPose* outPose);
    void EvaluateRootPosition(float32 localTime, Vector3* offset);

    float32 GetDuration() const;

protected:
    static JointTransform ConstructJointTransform(const AnimationTrack* track, const AnimationTrack::State* state);

    Vector<std::pair<uint32, const AnimationTrack*>> boundTracks; //[jointIndex, track]
    Vector<AnimationTrack::State> animationStates;

    const AnimationTrack* rootTrack = nullptr;
    AnimationTrack::State rootAnimationState;

    AnimationClip* animationClip = nullptr;

    Vector<float32> syncPoints;
    UnorderedSet<uint32> jointsIgnoreMask;
    uint32 maxJointIndex = 0;
};

} //ns