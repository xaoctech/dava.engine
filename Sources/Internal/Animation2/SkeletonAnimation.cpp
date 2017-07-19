#include "AnimationClip.h"
#include "AnimationTrack.h"
#include "SkeletonAnimation.h"

namespace DAVA
{
void SkeletonAnimation::BindAnimation(const AnimationClip* clip, const SkeletonComponent* skeleton)
{
    DVASSERT(skeleton);

    boundTracks.clear();
    animationStates.clear();
    skeletonPose.SetNodeCount(0);

    if (clip)
    {
        uint32 trackCount = clip->GetTrackCount();

        uint32 jointCount = skeleton->GetJointsCount();
        for (uint32 j = 0; j < jointCount; ++j)
        {
            const SkeletonComponent::Joint& joint = skeleton->GetJoint(j);

            for (uint32 t = 0; t < trackCount; ++t)
            {
                if (strcmp(clip->GetTrackUID(t), joint.uid.c_str()) == 0)
                {
                    const AnimationTrack* track = clip->GetTrack(t);

                    animationStates.emplace_back(AnimationTrack::State(track->GetChannelsCount()));
                    track->Reset(&animationStates.back());
                    skeletonPose.AddNode(j, ConstructJointTransform(track, &animationStates.back())); //node index in pose equal bound track index

                    boundTracks.emplace_back(track);
                }
            }
        }
    }
}

void SkeletonAnimation::Advance(float32 dTime, Vector3* offset)
{
    uint32 boundTrackCount = uint32(boundTracks.size());
    for (uint32 t = 0; t < boundTrackCount; ++t)
    {
        const AnimationTrack* track = boundTracks[t];
        AnimationTrack::State* state = &animationStates[t];
        if (state)
        {
            track->Advance(dTime, state);

            JointTransform transform = ConstructJointTransform(track, state);
            skeletonPose.SetTransform(t, transform);
        }
    }
}

void SkeletonAnimation::Reset()
{
    uint32 boundTrackCount = uint32(boundTracks.size());
    for (uint32 t = 0; t < boundTrackCount; ++t)
    {
        const AnimationTrack* track = boundTracks[t];
        AnimationTrack::State* state = &animationStates[t];
        if (state)
        {
            track->Reset(state);

            JointTransform transform = ConstructJointTransform(track, state);
            skeletonPose.SetTransform(t, transform);
        }
    }
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

const SkeletonPose& SkeletonAnimation::GetSkeletonPose() const
{
    return skeletonPose;
}

} //ns