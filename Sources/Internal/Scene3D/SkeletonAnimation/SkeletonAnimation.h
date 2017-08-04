#pragma once

#include "Base/BaseTypes.h"
#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
class AnimationClip;

class SkeletonAnimation
{
public:
    SkeletonAnimation(AnimationClip* animationClip);
    ~SkeletonAnimation();

    void BindSkeleton(const SkeletonComponent* skeleton, SkeletonPose* outInitialPose = nullptr);
    void EvaluatePose(float32 phase, SkeletonPose* outPose, Vector3* offset = nullptr);
    float32 GetPhaseDuration() const;

protected:
    static JointTransform ConstructJointTransform(const AnimationTrack* track, const AnimationTrack::State* state);

    Vector<std::pair<uint32, const AnimationTrack*>> boundTracks; //[jointIndex, track]
    Vector<AnimationTrack::State> animationStates;

    AnimationClip* animationClip = nullptr;

    uint32 maxJointIndex = 0;
};

} //ns