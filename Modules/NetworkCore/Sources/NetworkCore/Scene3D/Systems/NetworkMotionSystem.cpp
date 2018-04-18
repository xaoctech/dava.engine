#include "NetworkCore/Scene3D/Systems/NetworkMotionSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkMotionComponent.h"
#include "NetworkCore/NetworkMotionUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Scene3D/EntityGroup.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/SkeletonAnimation/MotionLayer.h>
#include <Scene3D/SkeletonAnimation/Private/Motion.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkMotionSystem)
{
    ReflectionRegistrator<NetworkMotionSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixedBeginFrame", &NetworkMotionSystem::ProcessFixedBeginFrame)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 20.0f)]
    .Method("ProcessFixedEndFrame", &NetworkMotionSystem::ProcessFixedEndFrame)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 6.0f)]
    .End();
}

NetworkMotionSystem::NetworkMotionSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask())
{
    networkAnimatedEntities = scene->AquireEntityGroup<NetworkMotionComponent, MotionComponent>();
}

void NetworkMotionSystem::ProcessFixedBeginFrame(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkMotionSystem::ProcessFixedBeginFrame");

    TransferDataFromNetworkComponents();
}

void NetworkMotionSystem::ProcessFixedEndFrame(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkMotionSystem::ProcessFixedEndFrame");

    TransferDataToNetworkComponents();
}

void NetworkMotionSystem::PrepareForRemove()
{
}

void NetworkMotionSystem::TransferDataToNetworkComponents()
{
    for (Entity* entity : networkAnimatedEntities->GetEntities())
    {
        DVASSERT(entity != nullptr);

        const MotionComponent* motionComponent = entity->GetComponent<MotionComponent>();
        DVASSERT(motionComponent != nullptr);

        NetworkMotionComponent* networkMotionComponent = entity->GetComponent<NetworkMotionComponent>();
        DVASSERT(networkMotionComponent != nullptr);

        // Params

        const uint32 numParams = networkMotionComponent->paramNames.size();
        networkMotionComponent->paramValues.resize(numParams);

        for (uint32 i = 0; i < numParams; ++i)
        {
            networkMotionComponent->paramValues[i] = motionComponent->GetParameter(networkMotionComponent->paramNames[i]);
        }

        // Layers

        const uint32 numLayers = motionComponent->GetMotionLayersCount();
        networkMotionComponent->currentMotionIds.resize(numLayers);
        networkMotionComponent->nextMotionIds.resize(numLayers);
        networkMotionComponent->transitionPhases.resize(numLayers);
        networkMotionComponent->transitionInfoIndices.resize(numLayers);

        for (uint32 i = 0; i < numLayers; ++i)
        {
            MotionLayer* layer = motionComponent->GetMotionLayer(i);
            DVASSERT(layer != nullptr);

            // Motions

            const Motion* currentMotion = layer->GetCurrentMotion();
            networkMotionComponent->currentMotionIds[i] = (currentMotion == nullptr) ? FastName() : currentMotion->GetID();

            const Motion* nextMotion = layer->GetNextMotion();
            networkMotionComponent->nextMotionIds[i] = (nextMotion == nullptr) ? FastName() : nextMotion->GetID();

            uint32 transitionIndex = std::numeric_limits<uint32>::max();
            const MotionTransition& motionTransition = layer->GetMotionTransition();
            const MotionTransitionInfo* motionTransitionInfo = motionTransition.GetTransitionInfo();
            if (motionTransitionInfo != nullptr)
            {
                const Vector<MotionTransitionInfo>& transitions = layer->GetTransitions();
                for (uint32 j = 0; j < transitions.size(); ++j)
                {
                    if (&transitions[j] == motionTransitionInfo)
                    {
                        transitionIndex = j;
                        break;
                    }
                }
                DVASSERT(transitionIndex != std::numeric_limits<uint32>::max());
            }
            networkMotionComponent->transitionInfoIndices[i] = transitionIndex;
            networkMotionComponent->transitionPhases[i] = motionTransition.GetTransitionPhase();

            const uint32 numMotions = layer->GetMotionCount();

            networkMotionComponent->motionCurrentPhaseIndices.resize(numLayers * NetworkMotionComponent::NumMaxMotions);
            networkMotionComponent->motionCurrentPhases.resize(numLayers * NetworkMotionComponent::NumMaxMotions);

            for (uint32 j = 0; j < numMotions; ++j)
            {
                Motion* motion = layer->GetMotion(j);
                DVASSERT(motion != nullptr);
                uint32 motionIndex = i * NetworkMotionComponent::NumMaxMotions + j;
                networkMotionComponent->motionCurrentPhaseIndices[motionIndex] = motion->GetCurrentPhaseIndex();
                networkMotionComponent->motionCurrentPhases[motionIndex] = motion->GetCurrentPhase();
            }
        }
    }
}

void NetworkMotionSystem::TransferDataFromNetworkComponents()
{
    for (Entity* entity : networkAnimatedEntities->GetEntities())
    {
        DVASSERT(entity != nullptr);

        const NetworkMotionComponent* networkMotionComponent = entity->GetComponent<NetworkMotionComponent>();
        DVASSERT(networkMotionComponent != nullptr);

        NetworkMotionUtils::CopyToMotion(networkMotionComponent);
    }
}
} //namespace DAVA
