#pragma once

#include "Base/BaseTypes.h"
#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
class AnimationClip;

class SkeletonAnimation
{
public:
    SkeletonAnimation() = default;

    void BindAnimation(const AnimationClip* animationClip, const SkeletonComponent* skeleton, SkeletonPose* outInitialPose = nullptr);
    void EvaluatePose(SkeletonPose* outPose, float32 time, Vector3* offset = nullptr);

protected:
    static JointTransform ConstructJointTransform(const AnimationTrack* track, const AnimationTrack::State* state);

    Vector<std::pair<uint32, const AnimationTrack*>> boundTracks; //[jointIndex, track weak pointer]
    Vector<AnimationTrack::State> animationStates;
};

} //ns