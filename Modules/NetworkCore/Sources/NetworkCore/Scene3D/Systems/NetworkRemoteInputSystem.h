#pragma once

#include <Base/Set.h>
#include <Base/Map.h>
#include <Base/FastName.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

namespace DAVA
{
class NetworkRemoteInputComponent;
class Scene;

/**
System that is responsible for:
    - Transfering input data from ActionsSingleComponent to NetworkRemoteInputComponent (on a server)
    - Transfering input data from NetworkRemoteInputComponent to ActionsSingleComponent (on a client)
    - Comparing remote and local input for debugging purposes (if `SetFullInputComparisonFlag(true)` was called on a server and clients)
*/
class NetworkRemoteInputSystem final : public BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkRemoteInputSystem, SceneSystem);

public:
    NetworkRemoteInputSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void ProcessFixed(float32 dt) override;
    void ProcessClient(float32 dt);
    void ProcessServer(float32 dt);
    void PrepareForRemove() override;

    void ReSimulationStart(Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;
    void Simulate(Entity* entity) override;

    /** Enable full input comparison. Value should be the same for server and client systems in order for comparison to work correctly. */
    void SetFullInputComparisonFlag(bool enabled);

    /** Return `true` if input comparison is enabled. */
    bool GetFullInputComparisonFlag() const;

    /** Return percentage of frames for last 10 seconds that have been processed on a server with input data which does not match client's */
    float32 GetIncorrectServerFramesPercentage() const;

private:
    void TransferInputToComponents();
    void TransferInputFromComponentsAndCompare();
    void ReplicateInputOnRange(NetworkRemoteInputComponent* remoteInputComponent, uint32 fromFrameId, uint32 toFrameId);
    void InsertActionsToSingleComponent(const Vector<FastName>& digitalActions, const AnalogActionsMap analogActions, Entity* entity);
    void CompareRemoteToLocalInput(NetworkRemoteInputComponent* remoteInputComponent);
    int64 GetLastKnownFrameId(NetworkRemoteInputComponent* remoteInputComponent) const;

    // Helper functions to work with NetworkRemoteInputComponent ring buffers
    bool IsEmpty(NetworkRemoteInputComponent* remoteInputComponent) const; // Return `true` if component is empty (i.e. has no input written in it)
    uint32 GetTailIndex(NetworkRemoteInputComponent* remoteInputComponent) const; // Tail = index with biggest frame
    uint32 GetHeadIndex(NetworkRemoteInputComponent* remoteInputComponent) const; // Head = index with least frame
    bool GetIndexWithFrame(NetworkRemoteInputComponent* remoteInputComponent, uint32 frameId, uint32& outIndex) const; // Return index in remote input component buffers where action for `frameId` is saved. Return `false` if there is no known action for passed `frameId`

private:
    UnorderedSet<NetworkRemoteInputComponent*> remoteInputComponents;
    UnorderedMap<NetworkRemoteInputComponent*, int64> lastReplicatedFrameIds; // Until what frame input was replicated

    // Member variables for comparing remote and local inputs
    static const uint32 COMPARISON_BUFFER_SIZE = 10; // Use COMPARISON_BUFFER_SIZE seconds to calculate value for `GetIncorrectServerFramesPercentage`
    std::array<uint32, COMPARISON_BUFFER_SIZE> numIncorrectInputs; // Each element is number of incorrect frames during that second. Used as a ring buffer
    bool comparisonEnabled; // True = server should send input, client should receive it
    uint32 lastComparedFrameId; // Frame id that was successfully compared last time
    uint32 numNotComparedFrames; // Number of frames that couldn't be compared. For debugging purposes only
    uint32 numIncorrectInputsIndex; // Current index in `numIncorrectInputs` ring buffer
    uint32 numIncorrectInputsCurrent; // Accumulator for number of incorrect frames during the second
    uint32 numHandledFrames; // Number of frames compared in `CompareRemoteToLocalInput`

    uint32 resimFrameId = 0;
};
}
