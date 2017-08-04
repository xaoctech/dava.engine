#include "SkeletonAnimation.h"

#include "Animation/AnimationClip.h"
#include "Animation/AnimationTrack.h"

namespace DAVA
{
SkeletonAnimation::SkeletonAnimation(AnimationClip* clip)
    : animationClip(SafeRetain(clip))
{
    DVASSERT(animationClip);
}

SkeletonAnimation::~SkeletonAnimation()
{
    SafeRelease(animationClip);
}

void SkeletonAnimation::BindSkeleton(const SkeletonComponent* skeleton, SkeletonPose* outInitialPose)
{
    DVASSERT(skeleton);
    DVASSERT(animationClip);

    boundTracks.clear();
    animationStates.clear();
    maxJointIndex = 0;

    uint32 trackCount = animationClip->GetTrackCount();
    uint32 jointCount = skeleton->GetJointsCount();
    for (uint32 j = 0; j < jointCount; ++j)
    {
        const SkeletonComponent::Joint& joint = skeleton->GetJoint(j);

        for (uint32 t = 0; t < trackCount; ++t)
        {
            if (strcmp(animationClip->GetTrackUID(t), joint.uid.c_str()) == 0)
            {
                const AnimationTrack* track = animationClip->GetTrack(t);

                animationStates.emplace_back(AnimationTrack::State(track->GetChannelsCount()));
                track->Evaluate(0.f, &animationStates.back());

                if (outInitialPose)
                    outInitialPose->SetTransform(j, ConstructJointTransform(track, &animationStates.back())); //node index in pose equal bound track index

                boundTracks.emplace_back(std::make_pair(j, track));

                maxJointIndex = Max(maxJointIndex, j);
            }
        }
    }
}

void SkeletonAnimation::EvaluatePose(float32 phase, SkeletonPose* outPose, Vector3* offset)
{
    DVASSERT(outPose);
    outPose->SetJointCount(maxJointIndex + 1);

    uint32 boundTrackCount = uint32(boundTracks.size());
    for (uint32 t = 0; t < boundTrackCount; ++t)
    {
        uint32 jointIndex = boundTracks[t].first;
        const AnimationTrack* track = boundTracks[t].second;
        AnimationTrack::State* state = &animationStates[t];

        float32 localTime = phase * animationClip->GetDuration();
        track->Evaluate(localTime, state);

        JointTransform transform = ConstructJointTransform(track, state);
        outPose->SetTransform(jointIndex, transform);
    }
}

float32 SkeletonAnimation::GetPhaseDuration() const
{
    return animationClip->GetDuration();
}

JointTransform SkeletonAnimation::ConstructJointTransform(const AnimationTrack* track, const AnimationTrack::State* state)
{
    JointTransform transform;
    for (uint32 c = 0; c < track->GetChannelsCount(); ++c)
    {
        AnimationTrack::eChannelTarget target = track->GetChannelTarget(c);
        switch (target)
        {
        case DAVA::AnimationTrack::CHANNEL_TARGET_POSITION:
            transform.position = Vector3(track->GetStateValue(state, c));
            break;

        case DAVA::AnimationTrack::CHANNEL_TARGET_ORIENTATION:
            transform.orientation = Quaternion(track->GetStateValue(state, c));
            break;

        case DAVA::AnimationTrack::CHANNEL_TARGET_SCALE:
            transform.scale = *track->GetStateValue(state, c);
            break;

        default:
            break;
        }
    }

    return transform;
}

} //ns