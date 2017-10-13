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

void MotionSystem::SetScene(Scene* scene)
{
    motionSingleComponent = (scene != nullptr) ? scene->motionSingleComponent : nullptr;
}

void MotionSystem::AddEntity(Entity* entity)
{
    DVASSERT(motionSingleComponent != nullptr);

    MotionComponent* motionComponent = GetMotionComponent(entity);
    motionSingleComponent->reloadConfig.push_back(motionComponent);
}

void MotionSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(activeComponents, GetMotionComponent(entity));
}

void MotionSystem::PrepareForRemove()
{
    activeComponents.clear();
}

void MotionSystem::ImmediateEvent(Component* component, uint32 event)
{
    DVASSERT(motionSingleComponent != nullptr);

    if (event == EventSystem::SKELETON_CONFIG_CHANGED)
    {
        MotionComponent* motionComponent = GetMotionComponent(component->GetEntity());
        motionSingleComponent->rebindAnimation.push_back(motionComponent);
    }
}

void MotionSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_MOTION_SYSTEM);
    DVASSERT(motionSingleComponent != nullptr);

    for (MotionComponent* motionComponent : motionSingleComponent->reloadConfig)
    {
        motionComponent->ReloadFromConfig();
        FindAndRemoveExchangingWithLast(activeComponents, motionComponent);
        motionSingleComponent->rebindAnimation.emplace_back(motionComponent);
    }

    for (MotionComponent* motionComponent : motionSingleComponent->rebindAnimation)
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

    motionSingleComponent->Clear();

    for (int32 i = int32(activeComponents.size()) - 1; i >= 0; --i)
    {
        MotionComponent* motionComponent = activeComponents[i];
        UpdateMotions(motionComponent, timeElapsed);
    }
}

void MotionSystem::UpdateMotions(MotionComponent* motionComponent, float32 dTime)
{
    DVASSERT(motionComponent);

    SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
    if (skeleton != nullptr)
    {
        SkeletonPose resultPose = skeleton->GetDefaultPose();
        uint32 motionCount = motionComponent->GetMotionsCount();
        for (uint32 m = 0; m < motionCount; ++m)
        {
            Motion* motion = motionComponent->GetMotion(m);

            motion->Update(dTime * motionComponent->playbackRate);

            for (auto& phaseEnd : motion->GetEndedPhases())
            {
                motionSingleComponent->animationPhaseEnd.emplace_back(motionComponent, MotionSingleComponent::AnimationPhaseInfo());

                MotionSingleComponent::AnimationPhaseInfo& phaseInfo = motionSingleComponent->animationPhaseEnd.back().second;
                phaseInfo.motionName = motion->GetName();
                phaseInfo.stateID = phaseEnd.first;
                phaseInfo.phaseID = phaseEnd.second;
            }

            const SkeletonPose& pose = motion->GetCurrentSkeletonPose();
            Motion::eMotionBlend blendMode = motion->GetBlendMode();
            switch (blendMode)
            {
            case Motion::BLEND_OVERRIDE:
                resultPose.Override(pose);
                motionComponent->rootOffsetDelta = motion->GetCurrentRootOffsetDelta();
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