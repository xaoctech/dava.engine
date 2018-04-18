#pragma once

#include <Base/Vector.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Functional/Function.h>

#include <physx/PxQueryFiltering.h>

namespace physx
{
class PxRigidActor;
class PxShape;
}

namespace DAVA
{
class Entity;
class Scene;
class PhysicsComponent;
class CollisionShapeComponent;
class CharacterControllerComponent;
class Vector3;

struct PhysicsUtils final
{
    /** Get static or dynamic body component attached to the entity. Return nullptr if there is none */
    static PhysicsComponent* GetBodyComponent(Entity* entity);

    /** Get vector of collision components attached to the entity */
    static Vector<CollisionShapeComponent*> GetShapeComponents(Entity* entity);

    static void ForEachShapeComponent(Entity* entity, Function<void(CollisionShapeComponent*)> callback);

    /** Get character controller component attached to the entity. Return nullptr if there is none */
    static CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity);

    static void CopyTransformToPhysics(Entity* entity);

    static void SyncJointsTransformsWithPhysics(Entity* entity);

    static void SetActorTransform(physx::PxRigidActor* actor, const Vector3& position, const Quaternion& orientation);

    static void SetShapeTransform(physx::PxShape* shape, const Vector3& position, const Quaternion& orientation);
};
}
