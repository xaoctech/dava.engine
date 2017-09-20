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

                if (j == 0)
                {
                    rootTrack = track;
                    rootTrackStateIndex = uint32(animationStates.size());
                }

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

void SkeletonAnimation::EvaluatePose(float32 localTime, SkeletonPose* outPose)
{
    DVASSERT(outPose);
    outPose->SetJointCount(maxJointIndex + 1);

    uint32 boundTrackCount = uint32(boundTracks.size());
    for (uint32 t = 0; t < boundTrackCount; ++t)
    {
        uint32 jointIndex = boundTracks[t].first;
        const AnimationTrack* track = boundTracks[t].second;
        AnimationTrack::State* state = &animationStates[t];

        track->Evaluate(localTime, state);

        outPose->SetTransform(jointIndex, ConstructJointTransform(track, state));
    }
}

void SkeletonAnimation::EvaluateRootPosition(float32 localTime, Vector3* offset)
{
    if (rootTrack != nullptr)
    {
        rootTrack->Evaluate(localTime, &animationStates[rootTrackStateIndex]);

        for (uint32 c = 0; c < rootTrack->GetChannelsCount(); ++c)
        {
            AnimationTrack::eChannelTarget target = rootTrack->GetChannelTarget(c);
            if (target == AnimationTrack::CHANNEL_TARGET_POSITION)
            {
                *offset = Vector3(rootTrack->GetStateValue(&animationStates[rootTrackStateIndex], c));
                break;
            }
        }
    }
}

float32 SkeletonAnimation::GetDuration() const
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
        case AnimationTrack::CHANNEL_TARGET_POSITION:
            transform.SetPosition(Vector3(track->GetStateValue(state, c)));
            break;

        case AnimationTrack::CHANNEL_TARGET_ORIENTATION:
            transform.SetOrientation(Quaternion(track->GetStateValue(state, c)));
            break;

        case AnimationTrack::CHANNEL_TARGET_SCALE:
            transform.SetScale(*track->GetStateValue(state, c));
            break;

        default:
            break;
        }
    }

    return transform;
}

} //ns