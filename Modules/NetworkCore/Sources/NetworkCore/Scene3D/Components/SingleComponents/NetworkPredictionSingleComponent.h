#pragma once

#include <Base/UnordererMap.h>
#include <Entity/SingleComponent.h>
#include "NetworkCore/Snapshot.h"

namespace DAVA
{
struct EntityMisprediction
{
    uint32 frameId;
    Vector<SnapshotComponentKey> components;
};

class NetworkPredictionSingleComponent : public ClearableSingleComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkPredictionSingleComponent, ClearableSingleComponent);

public:
    NetworkPredictionSingleComponent();

    using EntityToMisprediction = UnorderedMap<Entity*, EntityMisprediction>;
    EntityToMisprediction mispredictedEntities;

private:
    void Clear() override;
};
}
