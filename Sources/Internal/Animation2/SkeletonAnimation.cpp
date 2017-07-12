#include "AnimationClip.h"
#include "AnimationTrack.h"
#include "SkeletonAnimation.h"

namespace DAVA
{
void SkeletonAnimation::BindAnimation(const AnimationClip* clip, const SkeletonComponent* skeleton)
{
    DVASSERT(skeleton);

    animationStates.clear();
    skeletonPose.nodes.clear();
    animationClip = clip;

    if (animationClip)
    {
        uint32 trackCount = animationClip->GetTrackCount();
        skeletonPose.nodes.resize(trackCount);
        animationStates.resize(trackCount);

        uint32 jointCount = skeleton->GetJointsCount();
        for (uint32 j = 0; j < jointCount; ++j)
        {
            const SkeletonComponent::Joint& joint = skeleton->GetJoint(j);

            for (uint32 t = 0; t < trackCount; ++t)
            {
                if (strcmp(animationClip->GetTrackUID(t), joint.uid.c_str()) == 0)
                {
                    skeletonPose.nodes[t].jointIndex = j;

                    const AnimationTrack* track = animationClip->GetTrack(t);
                    animationStates[t] = AnimationTrack::State(track->GetChannelsCount());

                    track->Reset(&animationStates[t]);
                }
            }
        }
    }
}

void SkeletonAnimation::Advance(float32 dTime, Vector3* offset)
{
    if (animationClip)
    {
        for (uint32 t = 0; t < animationClip->GetTrackCount(); ++t)
        {
            const AnimationTrack* track = animationClip->GetTrack(t);
            AnimationTrack::State* state = &animationStates[t];
            if (track)
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
}

const SkeletonComponent::Pose& SkeletonAnimation::GetSkeletonPose() const
{
    return skeletonPose;
}

} //ns