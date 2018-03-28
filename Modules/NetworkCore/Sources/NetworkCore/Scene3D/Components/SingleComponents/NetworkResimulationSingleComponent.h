#pragma once

#include "NetworkCore/NetworkTypes.h"

#include <Entity/ComponentMask.h>
#include <Entity/SingleComponent.h>

#include <Scene3D/Scene.h>

namespace DAVA
{
class NetworkResimulationSystem;

namespace NetworkResimulationSingleComponentDetails
{
class BoundingBoxResimulation
{
public:
    using BuilderFn = Function<AABBox3(Entity*, float32 /* inflation */)>;
    using CollisionCheckFn = Function<bool(Entity*, Entity*)>;

    BoundingBoxResimulation();

    void SetBuilder(BuilderFn builder);
    const BuilderFn& GetBuilder() const;

    void SetCollisionChecker(CollisionCheckFn collisionChecker);
    const CollisionCheckFn& GetCollisionChecker() const;

    const UnorderedMap<NetworkID, AABBox3>& GetHistoryForFrame(uint32 frameId) const;

    static const size_t HistorySize = 64;

    float32 inflation = 1.01f;
    bool enabled = false;

private:
    BuilderFn builder;
    CollisionCheckFn collisionChecker;

    bool customBuilderIsSet = false;
    bool customCollisionCheckerIsSet = false;

    uint32 currentHistorySize = 0;
    uint32 lastHistoryFrameId = 0;

    Array<UnorderedMap<NetworkID, AABBox3>, HistorySize> history;

    friend class DAVA::NetworkResimulationSystem;
};
} // namespace NetworkResimulationSingleComponentDetails

class NetworkResimulationSingleComponent : public SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkResimulationSingleComponent, SingleComponent);

public:
    uint32 GetResimulationFrameId() const;
    const Vector<std::pair<Entity*, uint32>>& GetResimulatingEntities() const;

    NetworkResimulationSingleComponentDetails::BoundingBoxResimulation boundingBoxResimulation;

    struct
    {
        bool enabled = false;
        bool drawBbs = false;
        uint16 fadeTimeInSeconds = 3;
    } debugDraw;

private:
    void RegisterEngineVariables();

    uint32 resimulationFrameId = 0;

    Vector<std::pair<Entity*, uint32>> resimulatingEntities;

    friend class NetworkResimulationSystem;
};
} // namespace DAVA
