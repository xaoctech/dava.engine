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

    void BindAnimation(const AnimationClip* animationClip, const SkeletonComponent* skeleton);
    void EvaluatePose(float32 time, Vector3* offset = nullptr);

    const SkeletonPose& GetSkeletonPose() const;

protected:
    static JointTransform ConstructJointTransform(const AnimationTrack* track, const AnimationTrack::State* state);

    SkeletonPose skeletonPose;
    Vector<const AnimationTrack*> boundTracks; //weak pointers
    Vector<AnimationTrack::State> animationStates;
};

} //ns