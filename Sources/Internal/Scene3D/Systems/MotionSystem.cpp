#include "MotionSystem.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/SkeletonAnimation/Motion.h"

namespace DAVA
{
MotionSystem::MotionSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

MotionSystem::~MotionSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

void MotionSystem::AddEntity(Entity* entity)
{
    MotionComponent* motionComponent = GetMotionComponent(entity);

    GetScene()->motionSingleComponent->reloadConfig.push_back(motionComponent);
}

void MotionSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(activeComponents, GetMotionComponent(entity));
}

void MotionSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::SKELETON_CONFIG_CHANGED)
    {
        MotionComponent* motionComponent = GetMotionComponent(component->GetEntity());
        GetScene()->motionSingleComponent->rebindAnimation.push_back(motionComponent);
    }
}

void MotionSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_MOTION_SYSTEM);

    Vector<std::pair<MotionComponent*, MotionSingleComponent::AnimationPhaseInfo>> endedPhases;

    MotionSingleComponent* msc = GetScene()->motionSingleComponent;
    for (MotionComponent* motionComponent : msc->reloadConfig)
    {
        motionComponent->ReloadFromConfig();
        FindAndRemoveExchangingWithLast(activeComponents, motionComponent);
        msc->rebindAnimation.emplace_back(motionComponent);
    }

    for (MotionComponent* motionComponent : msc->rebindAnimation)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        DVASSERT(skeleton);

        uint32 motionCount = motionComponent->GetMotionsCount();
        for (uint32 m = 0; m < motionCount; ++m)
        {
            Motion* motion = motionComponent->GetMotion(m);
            motion->BindSkeleton(skeleton);
        }
        skeleton->ApplyPose(skeleton->GetDefaultPose());

        FindAndRemoveExchangingWithLast(activeComponents, motionComponent);
        activeComponents.emplace_back(motionComponent);
    }

    for (MotionComponent* motionComponent : msc->stopAnimation)
    {
        //MotionComponent::SimpleMotion* motion = motionComponent->simpleMotion;

        //motion->Stop();
        //FindAndRemoveExchangingWithLast(activeComponents, motionComponent);

        //SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        //if (skeleton)
        //{
        //    skeleton->ApplyPose(&motion->GetSkeletonPose());
        //}
    }

    for (MotionComponent* motionComponent : msc->startAnimation)
    {
        //motionComponent->simpleMotion->Start();
        //triggeredEvents.emplace_back(motionComponent, MotionComponent::EVENT_SINGLE_ANIMATION_STARTED);

        //activeComponents.emplace_back(motionComponent);
    }

    for (int32 i = int32(activeComponents.size()) - 1; i >= 0; --i)
    {
        MotionComponent* motionComponent = activeComponents[i];
        UpdateMotions(motionComponent, timeElapsed);

        //DVASSERT(motion->IsPlaying());

        //if (motion->IsFinished())
        //{
        //    motion->Stop();
        //    triggeredEvents.emplace_back(motionComponent, MotionComponent::EVENT_SINGLE_ANIMATION_ENDED);

        //    RemoveExchangingWithLast(activeComponents, i);
        //}
    }

    msc->Clear();
    msc->animationPhaseEnd = endedPhases;
}

void MotionSystem::UpdateMotions(MotionComponent* motionComponent, float32 dTime)
{
    DVASSERT(motionComponent);

    SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
    if (skeleton != nullptr)
    {
        SkeletonPose resultPose;
        uint32 motionCount = motionComponent->GetMotionsCount();
        for (uint32 m = 0; m < motionCount; ++m)
        {
            Motion* motion = motionComponent->GetMotion(m);
            motion->Update(dTime * motionComponent->playbackRate);

            const SkeletonPose& pose = motion->GetCurrentSkeletonPose();
            Motion::eMotionBlend blendMode = motion->GetBlendMode();
            switch (blendMode)
            {
            case Motion::BLEND_OVERRIDE:
                resultPose.Override(pose);
                break;
            case Motion::BLEND_ADD:
                resultPose.Add(pose);
                break;
            case Motion::BLEND_DIFF:
                resultPose.Diff(pose);
                break;
            case Motion::BLEND_LERP:
                resultPose.Lerp(pose, 0.5f /*TODO: *Skinning* motion-blend param*/);
                break;
            default:
                break;
            }
        }

        skeleton->ApplyPose(resultPose);
    }
}
}