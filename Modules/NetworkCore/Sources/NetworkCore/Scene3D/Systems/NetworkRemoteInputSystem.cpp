#include "NetworkRemoteInputSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Base/Set.h>
#include <Entity/Component.h>
#include <Entity/ComponentUtils.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Time/SystemTimer.h>

#include <numeric>

#define EXTRAPOLATION_ENABLED // Extrapolate input (i.e. filling current client frames with last known server input)

#define REMOTE_INPUT_SYSTEM_DEBUG_LOG(...) // Logger::Info(__VA_ARGS__)

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkRemoteInputSystem)
{
    ReflectionRegistrator<NetworkRemoteInputSystem>::Begin()[M::Tags("network", "input")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessClient", &NetworkRemoteInputSystem::ProcessClient)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 6.0f)]
    .Method("ProcessServer", &NetworkRemoteInputSystem::ProcessServer)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 2.0f)]
    .End();
}

namespace NetworkRemoteInputSystemDetails
{
// Helper function to increment ring buffer's index
uint32 GetIncrementedRingBufferIndex(uint32 currentIndex, int32 increment, uint32 bufferSize)
{
    int32 absoluteIndex = static_cast<int32>(currentIndex) + increment;
    int32 absoluteIndexModuloSize = absoluteIndex % static_cast<int32>(bufferSize);
    if (absoluteIndexModuloSize >= 0)
    {
        return absoluteIndexModuloSize;
    }
    else
    {
        return bufferSize + absoluteIndexModuloSize;
    }
}
}

NetworkRemoteInputSystem::NetworkRemoteInputSystem(Scene* scene)
    : DAVA::BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkRemoteInputComponent>())
    , comparisonEnabled(false)
    , lastComparedFrameId(0)
    , numNotComparedFrames(0)
    , numIncorrectInputs()
    , numIncorrectInputsIndex(0)
    , numIncorrectInputsCurrent(0)
    , numHandledFrames(0)
{
}

void NetworkRemoteInputSystem::AddEntity(Entity* entity)
{
    NetworkRemoteInputComponent* remoteInputComponent = entity->GetComponent<NetworkRemoteInputComponent>();
    DVASSERT(remoteInputComponent != nullptr);

    remoteInputComponents.insert(remoteInputComponent);
    lastReplicatedFrameIds[remoteInputComponent] = -1;
}

void NetworkRemoteInputSystem::RemoveEntity(Entity* entity)
{
    NetworkRemoteInputComponent* remoteInputComponent = entity->GetComponent<NetworkRemoteInputComponent>();
    DVASSERT(remoteInputComponent != nullptr);

    remoteInputComponents.erase(remoteInputComponent);
    lastReplicatedFrameIds.erase(remoteInputComponent);
}

void NetworkRemoteInputSystem::ProcessFixed(float32 dt)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkRemoteInputSystem::ProcessFixed");

    if (remoteInputComponents.size() == 0)
    {
        return;
    }

    if (IsServer(GetScene()))
    {
        TransferInputToComponents();
    }
    else
    {
        DVASSERT(IsClient(GetScene()));
        TransferInputFromComponentsAndCompare();
    }
}

void NetworkRemoteInputSystem::ProcessClient(float32 dt)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkRemoteInputSystem::ProcessClient");

    if (IsClient(GetScene()))
    {
        TransferInputFromComponentsAndCompare();
    }
}

void NetworkRemoteInputSystem::ProcessServer(float32 dt)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkRemoteInputSystem::ProcessServer");

    if (IsServer(GetScene()))
    {
        TransferInputToComponents();
    }
}

void NetworkRemoteInputSystem::PrepareForRemove()
{
}

void NetworkRemoteInputSystem::ReSimulationStart(Entity* entity, uint32 frameId)
{
    if (!IsClientOwner(this, entity))
    {
        resimFrameId = frameId;
    }
}

void NetworkRemoteInputSystem::Simulate(Entity* entity)
{
    if (resimFrameId > 0)
    {
        ++resimFrameId;
        NetworkRemoteInputComponent* netRemoteInputComp = entity->GetComponent<NetworkRemoteInputComponent>();
        ReplicateInputOnRange(netRemoteInputComp, resimFrameId, resimFrameId);
    }
}

void NetworkRemoteInputSystem::ReSimulationEnd(Entity* entity)
{
    resimFrameId = 0;
}

void NetworkRemoteInputSystem::SetFullInputComparisonFlag(bool enabled)
{
    comparisonEnabled = enabled;
}

bool NetworkRemoteInputSystem::GetFullInputComparisonFlag() const
{
    return comparisonEnabled;
}

float32 NetworkRemoteInputSystem::GetIncorrectServerFramesPercentage() const
{
    // total number of incorrect frames / (number of seconds * 60 fps)
    return std::accumulate(numIncorrectInputs.begin(), numIncorrectInputs.end(), 0) /
    static_cast<float32>(numIncorrectInputs.size() * NetworkTimeSingleComponent::FrequencyHz);
}

void NetworkRemoteInputSystem::TransferInputToComponents()
{
    NetworkTimeSingleComponent* networkTimeSingleComponent = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();

    uint32 currentFrameId = networkTimeSingleComponent->GetFrameId();

    // Move input from ActionsSingleComponent to NetworkRemoteInputComponents
    for (NetworkRemoteInputComponent* remoteInputComponent : remoteInputComponents)
    {
        DVASSERT(remoteInputComponent != nullptr);

        Entity* entity = remoteInputComponent->GetEntity();
        DVASSERT(entity != nullptr);

        // If comparison is enabled, we do not apply user filter
        // Since we need to send all the input to compare it correctly
        UnorderedSet<FastName>* actionsFilter = comparisonEnabled ? nullptr : &remoteInputComponent->actionsToReplicate;

        // Pack input
        uint64 triggeredActions = 0;
        NetworkInputSystem::PackDigitalActions(GetScene(), triggeredActions, entity, actionsFilter);
        uint64 triggeredAnalogActionsValues = NetworkInputSystem::PackAnalogActions(GetScene(), triggeredActions, entity, actionsFilter);

        // If input for this frame is the same as the previous one, do nothing
        // Otherwise save new input values and frame information
        uint32 tailIndex = GetTailIndex(remoteInputComponent);
        if (remoteInputComponent->triggeredActions[tailIndex] != triggeredActions ||
            remoteInputComponent->triggeredAnalogActionsValues[tailIndex] != triggeredAnalogActionsValues)
        {
            tailIndex = NetworkRemoteInputSystemDetails::GetIncrementedRingBufferIndex(tailIndex, 1, NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE);

            remoteInputComponent->triggeredActions[tailIndex] = triggeredActions;
            remoteInputComponent->triggeredAnalogActionsValues[tailIndex] = triggeredAnalogActionsValues;
            remoteInputComponent->actionFrames[tailIndex] = currentFrameId;

            REMOTE_INPUT_SYSTEM_DEBUG_LOG("Actions have changed. New data:\n\tactions = [%llu, %llu, %llu, %llu, %llu]\n\tanalogValues = [%llu, %llu, %llu, %llu, %llu]\n\tframes = [%u, %u, %u, %u, %u]",
                                          remoteInputComponent->triggeredActions[0], remoteInputComponent->triggeredActions[1], remoteInputComponent->triggeredActions[2], remoteInputComponent->triggeredActions[3], remoteInputComponent->triggeredActions[4],
                                          remoteInputComponent->triggeredAnalogActionsValues[0], remoteInputComponent->triggeredAnalogActionsValues[1], remoteInputComponent->triggeredAnalogActionsValues[2], remoteInputComponent->triggeredAnalogActionsValues[3], remoteInputComponent->triggeredAnalogActionsValues[4],
                                          remoteInputComponent->actionFrames[0], remoteInputComponent->actionFrames[1], remoteInputComponent->actionFrames[2], remoteInputComponent->actionFrames[3], remoteInputComponent->actionFrames[4]);
        }
    }
}

void NetworkRemoteInputSystem::TransferInputFromComponentsAndCompare()
{
    NetworkTimeSingleComponent* networkTimeSingleComponent = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();

    if (!networkTimeSingleComponent->IsInitialized())
    {
        return;
    }

    uint32 currentFrameId = networkTimeSingleComponent->GetFrameId();

    // Move input from NetworkRemoteInputComponent to ActionsSingleComponent for other players entities
    for (NetworkRemoteInputComponent* remoteInputComponent : remoteInputComponents)
    {
        DVASSERT(remoteInputComponent != nullptr);

        Entity* entity = remoteInputComponent->GetEntity();
        DVASSERT(entity != nullptr);

        if (IsClientOwner(GetScene(), entity))
        {
            if (comparisonEnabled)
            {
                CompareRemoteToLocalInput(remoteInputComponent);
            }

            continue;
        }

        int64 lastKnownFrameId = GetLastKnownFrameId(remoteInputComponent);
        if (lastKnownFrameId > currentFrameId)
        {
            lastKnownFrameId = currentFrameId;
        }

        // Replicate input only if we replicated remote input component at least once
        if (lastKnownFrameId >= 0)
        {
            DVASSERT(lastKnownFrameId <= UINT32_MAX);

#if defined(EXTRAPOLATION_ENABLED)
            uint32 frameToReplicateTo = currentFrameId;

            int64 lastReplicatedFrameId = lastReplicatedFrameIds[remoteInputComponent];
            uint32 frameToReplicateFrom;
            if (lastKnownFrameId > lastReplicatedFrameId)
            {
                frameToReplicateFrom = static_cast<uint32>(lastReplicatedFrameId + 1);
            }
            else
            {
                frameToReplicateFrom = currentFrameId;
            }
#else
            uint32 frameToReplicateFrom = static_cast<uint32>(lastReplicatedFrameIds[remoteInputComponent] + 1);
            uint32 frameToReplicateTo = lastKnownFrameId;
#endif

            if (frameToReplicateFrom <= frameToReplicateTo)
            {
                // If there are too many frames, replicate last 10 only
                // Should only happen on reconnects / huge lags
                if (frameToReplicateTo - frameToReplicateFrom > 100)
                {
                    frameToReplicateFrom = frameToReplicateTo - 9; // 9 since ReplicateInputOnRange works on closed intervals
                }

                ReplicateInputOnRange(remoteInputComponent, frameToReplicateFrom, frameToReplicateTo);

                lastReplicatedFrameIds[remoteInputComponent] = lastKnownFrameId;
            }
        }
    }
}

// Replicate on range [`fromFrameId`; `toFrameId`]
// If 'fromFrameId' is less that first known frame, replication will work on [first known frame; `toFrameId`]
// if `toFrameId` is bigger than last known frame, input will be replicated until last known frame and extrapolated after that
void NetworkRemoteInputSystem::ReplicateInputOnRange(NetworkRemoteInputComponent* remoteInputComponent, uint32 fromFrameId, uint32 toFrameId)
{
    DVASSERT(remoteInputComponent != nullptr);
    DVASSERT(fromFrameId <= toFrameId);

    uint32 firstKnownFrame = remoteInputComponent->actionFrames[GetHeadIndex(remoteInputComponent)];
    if (fromFrameId < firstKnownFrame)
    {
        fromFrameId = firstKnownFrame;
    }

    Entity* entity = remoteInputComponent->GetEntity();
    DVASSERT(entity != nullptr);

    uint64 triggeredActions;
    Vector<FastName> digitalActions;
    AnalogActionsMap analogActions;

    uint32 currentIndex = UINT32_MAX;
    uint32 currentIndexMaxFrame; // Inclusive

    for (uint32 frameId = fromFrameId; frameId <= toFrameId; ++frameId)
    {
        // If we didn't intialize index or we're out of range of frame for this index,
        // update it and also unpack input data for this index in advance
        if (currentIndex == UINT32_MAX || frameId == currentIndexMaxFrame + 1)
        {
            uint32 tailIndex = GetTailIndex(remoteInputComponent);

            bool dataExists = GetIndexWithFrame(remoteInputComponent, frameId, currentIndex);

            if (!dataExists)
            {
#if !defined(EXTRAPOLATION_ENABLED)
                // If data does not exist for this frame yet, it means that extrapolation should be enabled
                // Otherwise this frame shouldn't be in specified range
                DVASSERT(false);
#endif

                // Replicate from last known data
                currentIndex = tailIndex;
            }

            DVASSERT(currentIndex != UINT32_MAX && currentIndex < NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE);

            // If current index is tail - last frame is equal to toFrameId, otherwise it's a value at next index

            if (currentIndex == tailIndex)
            {
                currentIndexMaxFrame = toFrameId;
            }
            else
            {
                uint32 nextIndex = NetworkRemoteInputSystemDetails::GetIncrementedRingBufferIndex(currentIndex, 1, NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE);
                currentIndexMaxFrame = remoteInputComponent->actionFrames[nextIndex] - 1; // -1 since it's inclusive
            }

            DVASSERT(frameId <= currentIndexMaxFrame);

            triggeredActions = remoteInputComponent->triggeredActions[currentIndex];
            if (triggeredActions != 0)
            {
                digitalActions = NetworkInputSystem::UnpackDigitalActions(triggeredActions, GetScene());
                analogActions = NetworkInputSystem::UnpackAnalogActions(triggeredActions, remoteInputComponent->triggeredAnalogActionsValues[currentIndex], GetScene());
            }
            else
            {
                // No need to loop with zero actions, just skip these
                frameId = currentIndexMaxFrame;
                continue;
            }
        }

        // Replicate
        DVASSERT(triggeredActions != 0);
        InsertActionsToSingleComponent(digitalActions, analogActions, entity);

        REMOTE_INPUT_SYSTEM_DEBUG_LOG("Replicating input for frame %u (taken from frame %u)", frameId, remoteInputComponent->actionFrames[currentIndex]);
    }
}

void NetworkRemoteInputSystem::InsertActionsToSingleComponent(const Vector<FastName>& digitalActions, const AnalogActionsMap analogActions, Entity* entity)
{
    ActionsSingleComponent::Actions allActions;
    allActions.digitalActions = digitalActions;
    allActions.analogActions = analogActions;
    AddActionsForClient(GetScene(), entity, std::move(allActions));

    for (const FastName& action : digitalActions)
    {
        REMOTE_INPUT_SYSTEM_DEBUG_LOG("Replicated remote digital action for entity `%s`: %s", entity->GetName().c_str(), action.c_str());
    }

    for (const auto& actionTuple : analogActions)
    {
        REMOTE_INPUT_SYSTEM_DEBUG_LOG("Replicated remote analog action for entity `%s`: %s", entity->GetName().c_str(), actionTuple.first.actionId.c_str());
    }
}

void NetworkRemoteInputSystem::CompareRemoteToLocalInput(NetworkRemoteInputComponent* remoteInputComponent)
{
    DVASSERT(remoteInputComponent != nullptr);

    DVASSERT(comparisonEnabled == true);

    Entity* entity = remoteInputComponent->GetEntity();
    DVASSERT(entity != nullptr);
    DVASSERT(IsClientOwner(GetScene(), entity));

    NetworkInputComponent* localInputComponent = entity->GetComponent<NetworkInputComponent>();
    DVASSERT(localInputComponent != nullptr);

    if (numHandledFrames >= NetworkTimeSingleComponent::FrequencyHz)
    {
        numHandledFrames -= NetworkTimeSingleComponent::FrequencyHz;

        // Dump information for last 60 frames

        numIncorrectInputs[numIncorrectInputsIndex] = numIncorrectInputsCurrent;
        numIncorrectInputsIndex = NetworkRemoteInputSystemDetails::GetIncrementedRingBufferIndex(numIncorrectInputsIndex, 1, COMPARISON_BUFFER_SIZE);

        numIncorrectInputsCurrent = 0;
        numHandledFrames = 0;

        if (numNotComparedFrames > 0)
        {
            Logger::Info("NetworkRemoteInputSystem: couldn't compare remote and local input for %u frames during last second", numNotComparedFrames);
        }
        numNotComparedFrames = 0;
    }

    // Compare local input to remote input
    const NetworkInputComponent::History& localInputHistory = localInputComponent->GetHistory();
    for (auto iter = localInputHistory.Begin(); iter != localInputHistory.End(); ++iter)
    {
        uint32 frame = iter.Frame();
        if (frame <= lastComparedFrameId)
        {
            // Frame was already compared
            continue;
        }

        const NetworkInputComponent::Data& data = iter.Data();

        uint32 indexForFrame;
        if (GetIndexWithFrame(remoteInputComponent, frame, indexForFrame))
        {
            if (lastComparedFrameId != 0)
            {
                uint32 numMissedFrames = frame - lastComparedFrameId - 1;

                // If we skipped some frames, remember that for statistics
                if (numMissedFrames > 0)
                {
                    numNotComparedFrames += numMissedFrames;
                    numHandledFrames += numMissedFrames;
                }
            }

            if (data.actions != remoteInputComponent->triggeredActions[indexForFrame] ||
                data.analogStates != remoteInputComponent->triggeredAnalogActionsValues[indexForFrame])
            {
                REMOTE_INPUT_SYSTEM_DEBUG_LOG("Remote input does not match local one for frame %d. Local actions: [%llu, %llu], remote actions: [%llu, %llu]", frame, data.actions, data.analogStates, remoteInputComponent->triggeredActions[indexForFrame], remoteInputComponent->triggeredAnalogActionsValues[indexForFrame]);

                ++numIncorrectInputsCurrent;
            }

            ++numHandledFrames;
            lastComparedFrameId = frame;
        }
    }
}

int64 NetworkRemoteInputSystem::GetLastKnownFrameId(NetworkRemoteInputComponent* remoteInputComponent) const
{
    Entity* entity = remoteInputComponent->GetEntity();
    DVASSERT(entity != nullptr);
    NetworkID networkID = NetworkCoreUtils::GetEntityId(entity);
    DVASSERT(networkID != NetworkID::INVALID);

    // Get frame on which this entity was updated from the server last time
    NetworkReplicationSingleComponent* replicationSingleComponent = GetScene()->GetSingletonComponent<NetworkReplicationSingleComponent>();

    if (replicationSingleComponent->replicationInfo.find(networkID) != replicationSingleComponent->replicationInfo.end())
    {
        return replicationSingleComponent->replicationInfo[networkID].frameIdServer;
    }
    else
    {
        return -1;
    }
}

bool NetworkRemoteInputSystem::IsEmpty(NetworkRemoteInputComponent* remoteInputComponent) const
{
    for (int32 i = 0; i < NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE; ++i)
    {
        if (remoteInputComponent->actionFrames[i] != 0 || remoteInputComponent->triggeredActions[i] != 0)
        {
            return false;
        }
    }

    return true;
}

uint32 NetworkRemoteInputSystem::GetTailIndex(NetworkRemoteInputComponent* remoteInputComponent) const
{
    // If it's empty, assume tail to be last index
    if (IsEmpty(remoteInputComponent))
    {
        return NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE - 1;
    }

    int64 maxFirstFrame = -1;
    uint32 maxFirstFrameIndex = 0;
    for (int32 i = 0; i < NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE; ++i)
    {
        uint32 firstFrame = remoteInputComponent->actionFrames[i];
        if (static_cast<int64>(firstFrame) > maxFirstFrame)
        {
            maxFirstFrame = static_cast<int64>(firstFrame);
            maxFirstFrameIndex = i;
        }
    }

    return maxFirstFrameIndex;
}

uint32 NetworkRemoteInputSystem::GetHeadIndex(NetworkRemoteInputComponent* remoteInputComponent) const
{
    int64 minFirstFrame = INT64_MAX;
    uint32 minFirstFrameIndex = 0;
    for (int32 i = 0; i < NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE; ++i)
    {
        uint32 firstFrame = remoteInputComponent->actionFrames[i];
        if (static_cast<int64>(firstFrame) < minFirstFrame)
        {
            minFirstFrame = static_cast<int64>(firstFrame);
            minFirstFrameIndex = i;
        }
    }

    return minFirstFrameIndex;
}

// Return `true` if there is data for passed `frameId`
// Return `false` otherwise (i.e. data for `frameId` is not presented yet
bool NetworkRemoteInputSystem::GetIndexWithFrame(NetworkRemoteInputComponent* remoteInputComponent, uint32 frameId, uint32& outIndex) const
{
    uint32 tailIndex = GetTailIndex(remoteInputComponent);

    // If we request index for frame which is bigger than last action's starting frame,
    // we need to check if we haven't received data for this frame (and return false in this case) or input just has not changed (and in this case we return true + tailIndex)
    if (remoteInputComponent->actionFrames[tailIndex] < frameId)
    {
        Entity* entity = remoteInputComponent->GetEntity();
        DVASSERT(entity != nullptr);
        NetworkID networkID = NetworkCoreUtils::GetEntityId(entity);
        DVASSERT(networkID != NetworkID::INVALID);

        NetworkReplicationSingleComponent* replicationSingleComponent = GetScene()->GetSingletonComponent<NetworkReplicationSingleComponent>();

        DVASSERT(replicationSingleComponent->replicationInfo.find(networkID) != replicationSingleComponent->replicationInfo.end());
        if (frameId <= replicationSingleComponent->replicationInfo[networkID].frameIdServer)
        {
            outIndex = tailIndex;
            return true;
        }
        else
        {
            return false;
        }
    }

    // Otherwise, search for this frame in history
    uint32 headIndex = GetHeadIndex(remoteInputComponent);
    uint32 currentIndex = tailIndex;
    do
    {
        uint32 firstFrame = remoteInputComponent->actionFrames[currentIndex];
        if (firstFrame <= frameId)
        {
            outIndex = currentIndex;
            return true;
        }

        currentIndex = NetworkRemoteInputSystemDetails::GetIncrementedRingBufferIndex(currentIndex, -1, NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE);
    } while (currentIndex != tailIndex);

    return false;
}
}

#undef EXTRAPOLATION_ENABLED

#undef REMOTE_INPUT_SYSTEM_DEBUG_LOG
