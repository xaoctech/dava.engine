#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/Set.h>
#include <Entity/Component.h>

#include <Base/FixedVector.h>

namespace DAVA
{
/**
Contains input applied by a server
    Used for:
        - Handling other players input
        - Analyzing how many frames were processed with lost input on a server
*/
class NetworkRemoteInputComponent final : public Component
{
public:
    static const uint32 ACTIONS_BUFFER_SIZE = 5;

    DAVA_VIRTUAL_REFLECTION(NetworkRemoteInputComponent, Component);

    NetworkRemoteInputComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    /** Add action that will be replicated from server. By default there are no actions to replicate. */
    void AddActionToReplicate(const FastName& action);

private:
    // `triggeredActions[i]` were being invoked on interval [`actionFrames[i]`; `actionFrames[i + 1]`) in general
    // (`i` should be increment as a ring buffer index)
    // (if `i` is tail, then `triggeredActions[i]` ARE being invoked on interval [`actionFrames[i]`, currentFrame]
    std::array<uint64, ACTIONS_BUFFER_SIZE> triggeredActions;
    std::array<uint64, ACTIONS_BUFFER_SIZE> triggeredAnalogActionsValues;
    std::array<uint32, ACTIONS_BUFFER_SIZE> actionFrames;

    UnorderedSet<FastName> actionsToReplicate;

private:
    friend class NetworkRemoteInputSystem;
};
}
