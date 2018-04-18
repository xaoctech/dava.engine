#pragma once

#include <Base/FixedVector.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;

/**
    Contains data for synchronizing skeleton animation.
    `NetworkMotionSystem` copies data from `MotionComponent` to this component and vice versa.
*/
class NetworkMotionComponent final : public Component
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkMotionComponent, Component);

    NetworkMotionComponent();

    DAVA::Component* Clone(Entity* toEntity) override;

    static const uint32 NumMaxParams = 3;
    FixedVector<FastName> paramNames;
    FixedVector<float32> paramValues;

    static const uint32 NumMaxLayers = 3;
    FixedVector<FastName> currentMotionIds;
    FixedVector<FastName> nextMotionIds;
    FixedVector<float32> transitionPhases;
    FixedVector<uint32> transitionInfoIndices;

    static const uint32 NumMaxMotions = 3;
    FixedVector<uint32> motionCurrentPhaseIndices;
    FixedVector<float32> motionCurrentPhases;
};
}
