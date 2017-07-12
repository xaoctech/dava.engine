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
    skeletonPose.nodes.clear();

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
                    boundTracks.emplace_back(track);

                    skeletonPose.nodes.emplace_back();
                    skeletonPose.nodes.back().jointIndex = j;

                    animationStates.emplace_back(AnimationTrack::State(track->GetChannelsCount()));
                    track->Reset(&animationStates.back());
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

            for (uint32 c = 0; c < track->GetChannelsCount(); ++c)
            {
                AnimationTrack::eChannelTarget target = track->GetChannelTarget(c);
                switch (target)
                {
                case DAVA::AnimationTrack::CHANNEL_TARGET_POSITION:
                    skeletonPose.nodes[t].transform.position = Vector3(track->GetStateValue(state, c));
                    break;

                case DAVA::AnimationTrack::CHANNEL_TARGET_ORIENTATION:
                    skeletonPose.nodes[t].transform.orientation = Quaternion(track->GetStateValue(state, c));
                    break;

                case DAVA::AnimationTrack::CHANNEL_TARGET_SCALE:
                    skeletonPose.nodes[t].transform.scale = *track->GetStateValue(state, c);
                    break;

                default:
                    break;
                }
            }
        }
    }
}

const SkeletonComponent::Pose& SkeletonAnimation::GetSkeletonPose() const
{
    return skeletonPose;
}

} //ns