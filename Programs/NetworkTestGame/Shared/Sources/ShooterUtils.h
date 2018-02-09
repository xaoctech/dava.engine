#pragma once

#include "Components/ShooterAimComponent.h"

#include <Base/BaseTypes.h>
#include <Math/Vector.h>

#include <Scene3D/Components/TransformComponent.h>
#include <physx/PxQueryReport.h>
#include <physx/PxQueryFiltering.h>

namespace DAVA
{
class Entity;
class Scene;
}

// Filters to use when raycasting
enum class RaycastFilter : DAVA::uint32
{
    NONE = 0,

    // Ignore entity which is marked as 'source': i.e. if a player fires a bullet which originated inside of it's shape, this shape should be ignored to avoid collisions with itself
    IGNORE_SOURCE = 1 << 0,

    // Ignore all non-static objects. If raycast affects predicted state, this flag should always be used since for now all cars and other players are not predicted
    IGNORE_DYNAMICS = 1 << 1
};
DAVA_DEFINE_ENUM_BITWISE_OPERATORS(RaycastFilter)

// Its methods are invoked when raycasting
// Filters out entities according to passed `filter`
class QueryFilterCallback final : public physx::PxQueryFilterCallback
{
public:
    QueryFilterCallback(DAVA::Entity const* source, RaycastFilter filter);
    physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
    physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;

private:
    DAVA::Entity const* source;
    RaycastFilter filter;
};

// Raycast and get its first block hit
bool GetRaycastHit(DAVA::Scene& scene, const DAVA::Vector3& origin, const DAVA::Vector3& direction, DAVA::float32 distance,
                   physx::PxQueryFilterCallback* filterCallback, physx::PxRaycastHit& outHit);

// Get aim ray for current angle
void GetCurrentAimRay(ShooterAimComponent const& aimComponent, RaycastFilter filter, DAVA::Vector3& outAimRayOrigin, DAVA::Vector3& outAimRayDirection, DAVA::Vector3& outAimRayEnd, DAVA::Entity** outAimRayEndEntity);

// Get aim ray for final angle
void GetFinalAimRay(ShooterAimComponent const& aimComponent, RaycastFilter filter, DAVA::Vector3& outAimRayOrigin, DAVA::Vector3& outAimRayDirection, DAVA::Vector3& outAimRayEnd, DAVA::Entity** outAimRayEndEntity);

// Initialize game scene
void InitializeScene(DAVA::Scene& scene);

// Get random spawn position for placing a player
DAVA::Vector3 GetRandomPlayerSpawnPosition();