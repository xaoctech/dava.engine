#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/SkeletonAnimation/SkeletonAnimation.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/MotionSystem.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

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

    Vector<std::pair<MotionComponent*, FastName>> triggeredEvents;

    MotionSingleComponent* msc = GetScene()->motionSingleComponent;
    for (MotionComponent* motionComponent : msc->reloadConfig)
    {
        motionComponent->ReloadFromConfig();
        msc->rebindAnimation.emplace_back(motionComponent);
    }

    for (MotionComponent* motionComponent : msc->rebindAnimation)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        DVASSERT(skeleton);

        if (motionComponent->GetMotionsCount() > 0)
        {
            Motion* motion = motionComponent->GetMotion(0);
            motion->BindSkeleton(skeleton);
            skeleton->ApplyPose(motion->GetCurrentSkeletonPose());

            activeComponents.emplace_back(motionComponent);
        }
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
    msc->eventTriggered = triggeredEvents;
}

void MotionSystem::UpdateMotions(MotionComponent* motionComponent, float32 dTime)
{
    DVASSERT(motionComponent);
    DVASSERT(motionComponent->GetMotionsCount() > 0);
    Motion* motion = motionComponent->GetMotion(0);

    //debug
    //////////////////////////////////////////////////////////////////////////
    uint32 iParam = motion->GetParameterIndex(FastName("speed"));
    motion->SetParameter(iParam, motionComponent->workSpeedParameter);
    //////////////////////////////////////////////////////////////////////////

    motion->Update(dTime);

    SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
    DVASSERT(skeleton);
    skeleton->ApplyPose(motion->GetCurrentSkeletonPose());
}
}