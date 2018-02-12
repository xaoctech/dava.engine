#pragma once

#include <Base/UnordererMap.h>
#include <Entity/SingletonComponent.h>
#include "NetworkCore/Snapshot.h"

namespace DAVA
{
struct EntityMisprediction
{
    uint32 frameId;
    Vector<SnapshotComponentKey> components;
};

class NetworkPredictionSingleComponent : public SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkPredictionSingleComponent, SingletonComponent);

public:
    using EntityToMisprediction = UnorderedMap<Entity*, EntityMisprediction>;
    EntityToMisprediction mispredictedEntities;
};
}
