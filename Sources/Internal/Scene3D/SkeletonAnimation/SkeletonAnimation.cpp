#include "SkeletonAnimation.h"

#include "Animation/AnimationClip.h"
#include "Animation/AnimationTrack.h"

namespace DAVA
{
SkeletonAnimation::SkeletonAnimation(AnimationClip* clip, const UnorderedSet<uint32>& ignoreMask)
    : animationClip(SafeRetain(clip))
    , jointsIgnoreMask(ignoreMask)
{
    DVASSERT(animationClip);
}

SkeletonAnimation::~SkeletonAnimation()
{
    SafeRelease(animationClip);
}

void SkeletonAnimation::BindSkeleton(const SkeletonComponent* skeleton)
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
        if (jointsIgnoreMask.count(j) != 0)
            continue;

        const SkeletonComponent::Joint& joint = skeleton->GetJoint(j);

        for (uint32 t = 0; t < trackCount; ++t)
        {
            if (strcmp(animationClip->GetTrackUID(t), joint.uid.c_str()) == 0)
            {
                const AnimationTrack* track = animationClip->GetTrack(t);

                animationStates.emplace_back(AnimationTrack::State(track->GetChannelsCount()));
                track->Evaluate(0.f, &animationStates.back());

                boundTracks.emplace_back(std::make_pair(j, track));

                maxJointIndex = Max(maxJointIndex, j);
            }
        }
    }
}

void SkeletonAnimation::BindRootNode(const FastName& rootNodeID)
{
    rootTrack = nullptr;
    rootAnimationState = AnimationTrack::State();

    uint32 trackCount = animationClip->GetTrackCount();
    for (uint32 t = 0; t < trackCount; ++t)
    {
        if (strcmp(animationClip->GetTrackUID(t), rootNodeID.c_str()) == 0)
        {
            rootTrack = animationClip->GetTrack(t);

            rootAnimationState = AnimationTrack::State(rootTrack->GetChannelsCount());
            rootTrack->Evaluate(0.f, &rootAnimationState);
            break;
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
        rootTrack->Evaluate(localTime, &rootAnimationState);

        for (uint32 c = 0; c < rootTrack->GetChannelsCount(); ++c)
        {
            AnimationTrack::eChannelTarget target = rootTrack->GetChannelTarget(c);
            if (target == AnimationTrack::CHANNEL_TARGET_POSITION)
            {
                *offset = Vector3(rootTrack->GetStateValue(&rootAnimationState, c));
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