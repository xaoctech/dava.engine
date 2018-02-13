#pragma once

#include <Base/BaseTypes.h>
#include <Math/Vector.h>

#include <Scene3D/Components/TransformComponent.h>

#include <physx/PxQueryReport.h>
#include <physx/PxQueryFiltering.h>

namespace DAVA
{
class Entity;
class Scene;
class PhysicsComponent;
class CollisionShapeComponent;
class CharacterControllerComponent;
namespace NetworkPhysicsUtils
{
/** Perform raycasts with dynamic objects in the past */
bool GetRaycastHitInPast(Scene& scene, const ComponentMask& possibleComponents,
                         const Vector3& origin, const Vector3& direction, float32 distance,
                         uint32 frameId, physx::PxQueryFilterCallback* filterCall, physx::PxRaycastHit& outHit);
}
}
