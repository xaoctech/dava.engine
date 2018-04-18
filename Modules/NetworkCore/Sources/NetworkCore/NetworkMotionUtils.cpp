#include "NetworkCore/NetworkMotionUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkMotionComponent.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/SkeletonAnimation/MotionLayer.h>
#include <Scene3D/SkeletonAnimation/Private/Motion.h>

namespace DAVA
{
void NetworkMotionUtils::CopyToMotion(const NetworkMotionComponent* networkMotionComponent)
{
    DVASSERT(networkMotionComponent != nullptr);

    Entity* entity = networkMotionComponent->GetEntity();
    DVASSERT(entity != nullptr);

    MotionComponent* motionComponent = entity->GetComponent<MotionComponent>();
    DVASSERT(motionComponent != nullptr);

    // Params
    const uint32 numParams = networkMotionComponent->paramNames.size();
    for (uint32 i = 0; i < numParams; ++i)
    {
        motionComponent->SetParameter(networkMotionComponent->paramNames[i], networkMotionComponent->paramValues[i]);
    }

    // Layers
    const uint32 numLayers = motionComponent->GetMotionLayersCount();
    for (uint32 i = 0; i < numLayers; ++i)
    {
        MotionLayer* layer = motionComponent->GetMotionLayer(i);
        DVASSERT(layer != nullptr);

        const uint32 numMotions = layer->GetMotionCount();

        MotionTransition& motionTransition = layer->GetMotionTransition();

        // Motions

        const Motion* currentMotion = layer->GetCurrentMotion();
        const Motion* nextMotion = layer->GetNextMotion();

        Motion* networkCurrentMotion = nullptr;
        if (networkMotionComponent->currentMotionIds[i].IsValid())
        {
            for (uint32 j = 0; j < numMotions; ++j)
            {
                Motion* motion = layer->GetMotion(j);
                DVASSERT(motion != nullptr);
                if (motion->GetID() == networkMotionComponent->currentMotionIds[i])
                {
                    networkCurrentMotion = motion;
                }
            }
        }

        Motion* networkNextMotion = nullptr;
        if (networkMotionComponent->nextMotionIds[i].IsValid())
        {
            for (uint32 j = 0; j < numMotions; ++j)
            {
                Motion* motion = layer->GetMotion(j);
                DVASSERT(motion != nullptr);
                if (motion->GetID() == networkMotionComponent->nextMotionIds[i])
                {
                    networkNextMotion = motion;
                }
            }
        }

        bool currentOrNextMotionChanged = (currentMotion != networkCurrentMotion) || (nextMotion != networkNextMotion);
        if (currentOrNextMotionChanged)
        {
            layer->SetCurrentMotion(networkCurrentMotion);
            layer->SetNextMotion(networkNextMotion);

            uint32 transitionInfoIndex = networkMotionComponent->transitionInfoIndices[i];
            if (networkNextMotion != nullptr && transitionInfoIndex != UINT32_MAX)
            {
                motionTransition.Reset(&layer->GetTransitions()[transitionInfoIndex], networkCurrentMotion, networkNextMotion);
            }

            motionTransition.SetTransitionPhase(networkMotionComponent->transitionPhases[i]);
        }

        for (uint32 j = 0; j < numMotions; ++j)
        {
            Motion* motion = layer->GetMotion(j);
            DVASSERT(motion != nullptr);

            uint32 motionIndex = i * NetworkMotionComponent::NumMaxMotions + j;
            motion->SetCurrentPhaseIndex(networkMotionComponent->motionCurrentPhaseIndices[motionIndex]);
            motion->SetCurrentPhase(networkMotionComponent->motionCurrentPhases[motionIndex]);
        }
    }
}
}