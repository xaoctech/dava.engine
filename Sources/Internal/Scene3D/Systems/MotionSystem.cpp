#include "MotionSystem.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/SkeletonAnimation/MotionLayer.h"
#include "Scene3D/SkeletonAnimation/SimpleMotion.h"
#include "Scene3D/SkeletonAnimation/MotionUtils.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(MotionSystem)
{
    ReflectionRegistrator<MotionSystem>::Begin()[M::SystemTags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &MotionSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::EngineEnd, SPI::Type::Fixed, 1.1f)]
    .End();
}

MotionSystem::MotionSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<SkeletonComponent, MotionComponent>())
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

MotionSystem::~MotionSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

void MotionSystem::SetScene(Scene* scene)
{
    if (scene != nullptr)
    {
        motionSingleComponent = scene->GetSingleComponentForWrite<MotionSingleComponent>(this);
    }
}

void MotionSystem::AddEntity(Entity* entity)
{
    DVASSERT(motionSingleComponent != nullptr);

    MotionComponent* motionComponent = GetMotionComponent(entity);
    motionSingleComponent->reloadDescriptor.push_back(motionComponent);
}

void MotionSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(activeComponents, GetMotionComponent(entity));
}

void MotionSystem::PrepareForRemove()
{
    activeComponents.clear();
}

void MotionSystem::UnregisterEntity(Entity* entity)
{
    SceneSystem::UnregisterEntity(entity);

    DVASSERT(GetScene() != nullptr);

    motionSingleComponent->EntityRemoved(entity);
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

void MotionSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_MOTION_SYSTEM);
    DVASSERT(motionSingleComponent != nullptr);

    for (MotionComponent* motionComponent : motionSingleComponent->reloadDescriptor)
    {
        motionComponent->ReloadFromFile();
        FindAndRemoveExchangingWithLast(activeComponents, motionComponent);
        motionSingleComponent->rebindSkeleton.emplace_back(motionComponent);
    }

    for (MotionComponent* motionComponent : motionSingleComponent->rebindSkeleton)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
        if (skeleton != nullptr)
        {
            uint32 motionLayersCount = motionComponent->GetMotionLayersCount();
            for (uint32 l = 0; l < motionLayersCount; ++l)
            {
                MotionLayer* motionLayer = motionComponent->GetMotionLayer(l);
                motionLayer->BindSkeleton(skeleton);
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

    for (MotionComponent* component : activeComponents)
    {
        MotionUtils::UpdateMotionLayers(component, timeElapsed);
    }
}
}