#pragma once

#include <Base/FixedVector.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;

/**
    Component that contains hitbox transforms for debugging and visual comparison.
    At any moment, hitboxes can be snapshoted into this component using `NetworkPhysicsUtils::SnapshotDebugDrawHitboxes`.
    Afterwards, they are drawn by `HitboxesDebugDrawSystem`.
*/
class HitboxesDebugDrawComponent final : public Component
{
public:
    DAVA_VIRTUAL_REFLECTION(HitboxesDebugDrawComponent, Component);

    HitboxesDebugDrawComponent();

    Component* Clone(Entity* toEntity) override;

    static const uint32 NumMaxHitboxes = 15;
    FixedVector<Vector3> serverHitboxPositions;
    FixedVector<Quaternion> serverHitboxOrientations;
    FixedVector<Vector3> clientHitboxPositions;
    FixedVector<Quaternion> clientHitboxOrientations;
};
}
