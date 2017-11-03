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
#include "Scene3D/SkeletonAnimation/SimpleMotion.h"

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
    motionSingleComponent->reloadMotion.push_back(motionComponent);
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
        motionSingleComponent->rebindSkeleton.push_back(motionComponent);
    }
}

void MotionSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_MOTION_SYSTEM);
    DVASSERT(motionSingleComponent != nullptr);

    for (MotionComponent* motionComponent : motionSingleComponent->reloadMotion)
    {
        motionComponent->ReloadFromFile();
        FindAndRemoveExchangingWithLast(activeComponents, motionComponent);
        motionSingleComponent->rebindSkeleton.emplace_back(motionComponent);
    }

    for (MotionComponent* motionComponent : motionSingleComponent->rebindSkeleton)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        DVASSERT(skeleton);

        uint32 motionCount = motionComponent->GetMotionsCount();
        for (uint32 m = 0; m < motionCount; ++m)
        {
            Motion* motion = motionComponent->GetMotion(m);
            motion->BindSkeleton(skeleton);
        }

        FindAndRemoveExchangingWithLast(activeComponents, motionComponent);
        activeComponents.emplace_back(motionComponent);

        SkeletonPose defaultPose = skeleton->GetDefaultPose();
        SimpleMotion* simpleMotion = motionComponent->simpleMotion;
        if (simpleMotion != nullptr)
        {
            simpleMotion->BindSkeleton(skeleton);
            simpleMotion->EvaluatePose(&defaultPose);
        }
        skeleton->ApplyPose(defaultPose);
    }

    for (MotionComponent* motionComponent : motionSingleComponent->stopSimpleMotion)
    {
        if (motionComponent->simpleMotion != nullptr)
            motionComponent->simpleMotion->Stop();
    }

    for (MotionComponent* motionComponent : motionSingleComponent->startSimpleMotion)
    {
        SimpleMotion* simpleMotion = motionComponent->simpleMotion;
        if (simpleMotion != nullptr)
        {
            if (simpleMotion->IsPlaying())
                continue;

            simpleMotion->SetRepeatsCount(motionComponent->simpleMotionRepeatsCount);
            simpleMotion->Start();
        }
    }

    motionSingleComponent->Clear();

    for (MotionComponent* component : activeComponents)
    {
        UpdateMotions(component, timeElapsed);
    }
}

void MotionSystem::UpdateMotions(MotionComponent* motionComponent, float32 dTime)
{
    DVASSERT(motionComponent);

    SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
    if (skeleton != nullptr && (motionComponent->GetMotionsCount() != 0 || (motionComponent->simpleMotion != nullptr && motionComponent->simpleMotion->IsPlaying())))
    {
        dTime *= motionComponent->playbackRate;
        SkeletonPose resultPose = skeleton->GetDefaultPose();

        uint32 motionCount = motionComponent->GetMotionsCount();
        for (uint32 m = 0; m < motionCount; ++m)
        {
            Motion* motion = motionComponent->GetMotion(m);

            motion->Update(dTime);

            for (auto& phaseEnd : motion->GetEndedPhases())
            {
                if (phaseEnd.second == Motion::STATE_ANIMATION_END_MARKER)
                    motionSingleComponent->animationEnd.insert(MotionSingleComponent::AnimationInfo(motionComponent, motion->GetName(), phaseEnd.first));
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

        SimpleMotion* simpleMotion = motionComponent->simpleMotion;
        if (simpleMotion != nullptr && simpleMotion->IsPlaying())
        {
            simpleMotion->Update(dTime);
            if (simpleMotion->IsFinished())
                motionSingleComponent->simpleMotionFinished.emplace_back(motionComponent);

            simpleMotion->EvaluatePose(&resultPose);
        }

        skeleton->ApplyPose(resultPose);
    }
}
}