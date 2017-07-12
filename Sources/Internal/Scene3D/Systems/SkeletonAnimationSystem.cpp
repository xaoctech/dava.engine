#include "Animation2/AnimationClip.h"
#include "Animation2/AnimationTrack.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/SkeletonAnimationComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/SkeletonAnimationSystem.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
SkeletonAnimationSystem::SkeletonAnimationSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

SkeletonAnimationSystem::~SkeletonAnimationSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

void SkeletonAnimationSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);

    SkeletonComponent* skeleton = GetSkeletonComponent(entity);
    SkeletonAnimationComponent* animation = GetSkeletonAnimationComponent(entity);

    BindAnimation(animation, skeleton);
}

void SkeletonAnimationSystem::RemoveEntity(Entity* entity)
{
    Vector<Entity*>::iterator found = std::find(entities.begin(), entities.end(), entity);
    DVASSERT(found != entities.end());

    if (found != entities.end())
    {
        *found = entities.back();
        entities.pop_back();
    }
}

void SkeletonAnimationSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::SKELETON_CONFIG_CHANGED)
    {
        BindAnimation(GetSkeletonAnimationComponent(component->GetEntity()), static_cast<SkeletonComponent*>(component));
    }
}

void SkeletonAnimationSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_SKELETON_ANIMATION_SYSTEM);

    for (Entity*& e : entities)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(e);
        SkeletonAnimationComponent* animation = GetSkeletonAnimationComponent(e);

        DVASSERT(skeleton);
        DVASSERT(animation);

        if (animation->animationChanged)
        {
            BindAnimation(animation, skeleton);
            animation->animationChanged = false;
        }

        if (animation->animationClip)
        {
            for (uint32 jointIndex = 0; jointIndex < skeleton->GetJointsCount(); ++jointIndex)
            {
                const AnimationTrack* track = animation->animationStates[jointIndex].first;
                AnimationTrack::State* state = &animation->animationStates[jointIndex].second;
                if (track)
                {
                    track->Advance(timeElapsed, state);

                    SkeletonComponent::JointTransform transform;
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

                    skeleton->SetJointTransform(jointIndex, transform);
                }
            }
        }
    }
}

void SkeletonAnimationSystem::BindAnimation(SkeletonAnimationComponent* animation, SkeletonComponent* skeleton)
{
    DVASSERT(animation);
    DVASSERT(skeleton);

    if (animation->animationClip)
    {
        uint32 jointCount = skeleton->GetJointsCount();
        animation->animationStates.resize(jointCount);

        uint32 trackCount = animation->animationClip->GetTrackCount();
        for (uint32 j = 0; j < jointCount; ++j)
        {
            const SkeletonComponent::Joint& joint = skeleton->GetJoint(j);

            for (uint32 t = 0; t < trackCount; ++t)
            {
                if (strcmp(animation->animationClip->GetTrackUID(t), joint.uid.c_str()) == 0)
                {
                    const AnimationTrack* track = animation->animationClip->GetTrack(t);
                    animation->animationStates[j] = std::make_pair(track, AnimationTrack::State(track->GetChannelsCount()));

                    track->Reset(&animation->animationStates[j].second);
                }
            }
        }
    }
}
}