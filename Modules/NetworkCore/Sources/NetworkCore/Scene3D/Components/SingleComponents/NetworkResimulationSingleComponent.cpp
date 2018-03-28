#include "NetworkResimulationSingleComponent.h"

#include <Engine/EngineContext.h>
#include <Engine/EngineSettings.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkResimulationSingleComponent)
{
    ReflectionRegistrator<NetworkResimulationSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

uint32 NetworkResimulationSingleComponent::GetResimulationFrameId() const
{
    DVASSERT(BaseSimulationSystem::IsReSimulating());

    return resimulationFrameId;
}

const Vector<std::pair<Entity*, uint32>>& NetworkResimulationSingleComponent::GetResimulatingEntities() const
{
    return resimulatingEntities;
}

void NetworkResimulationSingleComponent::RegisterEngineVariables()
{
    static bool registered = false;

    if (!registered)
    {
        EngineSettings* es = GetEngineContext()->settings;

        es->RegisterVar(FastName("cl_resimulation_debug_draw"), debugDraw.enabled, "Enable resimulation debug draw.");
        es->RegisterVar(FastName("cl_resimulation_debug_draw_fade_time"), debugDraw.fadeTimeInSeconds, "Resimulation debug draw fade time (in seconds).", M::Range(1, 10, 1));
        es->RegisterVar(FastName("cl_bb_resimulation"), boundingBoxResimulation.enabled, "Enable bounding box resimulation.");
        es->RegisterVar(FastName("cl_bb_resimulation_debug_draw"), debugDraw.drawBbs, "Enable bounding box resimulation debug draw.");

        es->varChanged.Connect(this, [this](EngineSettingsVar* var) {
            FastName name = var->GetName();

            if (name == FastName("cl_resimulation_debug_draw"))
            {
                debugDraw.enabled = var->GetValue().Cast<bool>();
            }
            else if (name == FastName("cl_resimulation_debug_draw_fade_time"))
            {
                debugDraw.fadeTimeInSeconds = var->GetValue().Cast<uint16>();
            }
            else if (name == FastName("cl_bb_resimulation"))
            {
                boundingBoxResimulation.enabled = var->GetValue().Cast<bool>();
            }
            else if (name == FastName("cl_bb_resimulation_debug_draw"))
            {
                debugDraw.drawBbs = var->GetValue().Cast<bool>();
            }
        });

        registered = true;
    }
    else
    {
        DVASSERT(false);
    }
}

namespace NetworkResimulationSingleComponentDetails
{
BoundingBoxResimulation::BoundingBoxResimulation()
{
    builder = [](Entity* entity, float32 inflation) -> AABBox3
    {
        AABBox3 box;

        box = entity->GetWTMaximumBoundingBoxSlow();

        Vector3 extents = (box.max - box.min) * 0.5f * inflation;
        Vector3 center = box.GetCenter();

        return AABBox3(center - extents, center + extents);
    };

    collisionChecker = [](Entity*, Entity*)
    {
        return true;
    };
}

void BoundingBoxResimulation::SetBuilder(BuilderFn builder_)
{
    builder = builder_;
    customBuilderIsSet = true;
}

const BoundingBoxResimulation::BuilderFn& BoundingBoxResimulation::GetBuilder() const
{
    return builder;
}

const UnorderedMap<NetworkID, AABBox3>& BoundingBoxResimulation::GetHistoryForFrame(uint32 frameId) const
{
    static const UnorderedMap<NetworkID, AABBox3> empty;

    uint32 minFrameId = lastHistoryFrameId - currentHistorySize;

    if (minFrameId <= frameId && frameId <= lastHistoryFrameId)
    {
        return history[frameId % HistorySize];
    }

    return empty;
}

const BoundingBoxResimulation::CollisionCheckFn& BoundingBoxResimulation::GetCollisionChecker() const
{
    return collisionChecker;
}

void BoundingBoxResimulation::SetCollisionChecker(CollisionCheckFn collisionChecker_)
{
    collisionChecker = collisionChecker_;
    customCollisionCheckerIsSet = true;
}

} // namespace NetworkResimulationSingleComponentDetails

} // namespace DAVA
