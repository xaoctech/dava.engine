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

    GetScene()->motionSingleComponent->rebindAnimation.push_back(motionComponent);
}

void MotionSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(activeMotions, GetMotionComponent(entity));
}

void MotionSystem::PrepareForRemove()
{
    activeMotions.clear();
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
    for (MotionComponent* motionComponent : msc->rebindAnimation)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        if (skeleton)
        {
            MotionComponent::SimpleMotion* motion = motionComponent->simpleMotion;

            motion->BindSkeleton(skeleton);
            skeleton->ApplyPose(motion->GetAnimation()->GetSkeletonPose());
        }
    }

    for (MotionComponent* motionComponent : msc->stopAnimation)
    {
        MotionComponent::SimpleMotion* motion = motionComponent->simpleMotion;

        motion->Stop();
        FindAndRemoveExchangingWithLast(activeMotions, motionComponent);

        SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        if (skeleton)
        {
            skeleton->ApplyPose(motion->GetAnimation()->GetSkeletonPose());
        }
    }

    for (MotionComponent* motionComponent : msc->startAnimation)
    {
        if (motionComponent->simpleMotion->IsPlaying())
            continue;

        motionComponent->simpleMotion->Start();
        triggeredEvents.emplace_back(motionComponent, MotionComponent::EVENT_SINGLE_ANIMATION_STARTED);

        activeMotions.emplace_back(motionComponent);
    }

    for (int32 i = int32(activeMotions.size()) - 1; i >= 0; --i)
    {
        MotionComponent* motionComponent = activeMotions[i];
        MotionComponent::SimpleMotion* motion = motionComponent->simpleMotion;
        DVASSERT(motion->IsPlaying());

        motion->Update(timeElapsed);
        if (motion->IsFinished())
        {
            motion->Stop();
            triggeredEvents.emplace_back(motionComponent, MotionComponent::EVENT_SINGLE_ANIMATION_ENDED);

            RemoveExchangingWithLast(activeMotions, i);
        }

        SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        if (skeleton)
        {
            skeleton->ApplyPose(motion->GetAnimation()->GetSkeletonPose());
        }
    }

    msc->Clear();
    msc->eventTriggered = triggeredEvents;
}
}