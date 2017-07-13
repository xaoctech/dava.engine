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
    void Advance(float32 dTime, Vector3* offset = nullptr);

    const SkeletonPose& GetSkeletonPose() const;

protected:
    SkeletonPose skeletonPose;
    Vector<const AnimationTrack*> boundTracks; //weak pointers
    Vector<AnimationTrack::State> animationStates;
};

} //ns