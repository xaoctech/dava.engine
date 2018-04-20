#include "NetworkPhysics/NetworkPhysicsUtils.h"
#include "NetworkPhysics/HitboxesDebugDrawComponent.h"

#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h>
#include <NetworkCore/SnapshotUtils.h>
#include <NetworkCore/NetworkCoreUtils.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/Core/CollisionShapeComponent.h>
#include <Physics/Core/PhysicsUtils.h>
#include <Physics/Core/Private/PhysicsMath.h>

#include <Base/UnordererMap.h>
#include <Engine/Engine.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <physx/extensions/PxShapeExt.h>

namespace DAVA
{
namespace NetworkPhysicsUtils
{
bool GetRaycastHitInPast(Scene& scene, const ComponentMask& possibleComponents,
                         const Vector3& origin, const Vector3& direction, float32 distance,
                         uint32 frameId, physx::PxQueryFilterCallback* filterCallback, physx::PxRaycastHit& outHit)
{
    // Get all dynamics

    Vector<Entity*> dynamics;
    scene.GetChildEntitiesWithCondition(dynamics, [possibleComponents](Entity* e)
                                        {
                                            // All entities should have network transform component since we keep history of it
                                            NetworkTransformComponent* networkTransformComponent = e->GetComponent<NetworkTransformComponent>();
                                            if (!networkTransformComponent)
                                            {
                                                return false;
                                            }

                                            const ComponentMask& entityComponentMask = e->GetAvailableComponentMask();
                                            return (possibleComponents & entityComponentMask).IsAnySet();
                                        });

    // Save current network transforms
    UnorderedMap<Entity*, Transform> oldTransforms;
    for (Entity* e : dynamics)
    {
        DVASSERT(e != nullptr);

        TransformComponent* transformComponent = e->GetComponent<TransformComponent>();
        DVASSERT(transformComponent != nullptr);

        oldTransforms[e] = transformComponent->GetLocalTransform();
    }

    // Roll back transforms

    SnapshotSingleComponent* snapshotSingleComponent = scene.GetSingleComponent<SnapshotSingleComponent>();
    DVASSERT(snapshotSingleComponent != nullptr);

    Snapshot* snapshot = snapshotSingleComponent->GetServerSnapshot(frameId);
    if (snapshot == nullptr)
    {
        return false;
    }

    for (Entity* e : dynamics)
    {
        DVASSERT(e != nullptr);

        NetworkTransformComponent* networkTransformComponent = e->GetComponent<NetworkTransformComponent>();
        DVASSERT(networkTransformComponent != nullptr);

        NetworkReplicationComponent* replicationComponent = e->GetComponent<NetworkReplicationComponent>();
        DVASSERT(replicationComponent != nullptr);

        SnapshotComponentKey componentKey(ComponentUtils::GetRuntimeId<NetworkTransformComponent>(), 0);
        bool applyResult = SnapshotUtils::ApplySnapshot(snapshot, replicationComponent->GetNetworkID(), componentKey, networkTransformComponent);
        if (!applyResult)
        {
            return false;
        }

        TransformComponent* transformComponent = e->GetComponent<TransformComponent>();
        DVASSERT(transformComponent != nullptr);

        transformComponent->SetLocalTransform(Transform(
        networkTransformComponent->GetPosition(),
        transformComponent->GetLocalTransform().GetScale(),
        networkTransformComponent->GetOrientation()));
    }

    PhysicsSystem* physicsSystem = scene.GetSystem<PhysicsSystem>();
    DVASSERT(physicsSystem != nullptr);

    // Sync transforms so that raycast uses new positions
    physicsSystem->SyncTransformToPhysx();

    // Raycast
    bool result = false;

    physx::PxQueryFilterData queryFilter;
    queryFilter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
    if (filterCallback != nullptr)
    {
        queryFilter.flags |= physx::PxQueryFlag::ePREFILTER;
    }

    physx::PxRaycastBuffer hitBuffer;
    bool collisionDetected = physicsSystem->Raycast(origin, direction, distance, hitBuffer, queryFilter, filterCallback);
    if (collisionDetected)
    {
        outHit = hitBuffer.block;
        result = true;
    }

    // Restore transforms

    for (Entity* e : dynamics)
    {
        NetworkTransformComponent* networkTransformComponent = e->GetComponent<NetworkTransformComponent>();
        DVASSERT(networkTransformComponent != nullptr);

        TransformComponent* transformComponent = e->GetComponent<TransformComponent>();
        DVASSERT(transformComponent);

        Transform& transform = oldTransforms[e];

        networkTransformComponent->SetPosition(transform.GetTranslation());
        networkTransformComponent->SetOrientation(transform.GetRotation());

        transformComponent->SetLocalTransform(transform);
    }

    // Sync restored transforms for potential future raycasts on same frame
    physicsSystem->SyncTransformToPhysx();

    // Return result
    return result;
}

void SnapshotDebugDrawHitboxes(HitboxesDebugDrawComponent& hitboxesDebugDrawComponent, Entity& target)
{
    Entity* entity = hitboxesDebugDrawComponent.GetEntity();
    DVASSERT(entity != nullptr);

    Scene* scene = entity->GetScene();
    DVASSERT(scene != nullptr);

    const bool isServer = IsServer(scene);
    FixedVector<Vector3>& positions = isServer ? hitboxesDebugDrawComponent.serverHitboxPositions : hitboxesDebugDrawComponent.clientHitboxPositions;
    FixedVector<Quaternion>& orientations = isServer ? hitboxesDebugDrawComponent.serverHitboxOrientations : hitboxesDebugDrawComponent.clientHitboxOrientations;

    CollisionShapeComponent* shapes[HitboxesDebugDrawComponent::NumMaxHitboxes];
    uint32 numShapes = 0;
    PhysicsUtils::ForEachShapeComponent(&target, [&shapes, &numShapes](CollisionShapeComponent* shape)
                                        {
                                            if (numShapes < HitboxesDebugDrawComponent::NumMaxHitboxes)
                                            {
                                                shapes[numShapes] = shape;
                                                ++numShapes;
                                            }
                                        });

    positions.resize(numShapes);
    orientations.resize(numShapes);
    for (uint32 i = 0; i < numShapes; ++i)
    {
        const physx::PxShape* shape = shapes[i]->GetPxShape();
        DVASSERT(shape != nullptr);

        const physx::PxActor* actor = shape->getActor();
        DVASSERT(actor != nullptr);

        const physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
        DVASSERT(rigidActor != nullptr);

        physx::PxTransform globalPose = physx::PxShapeExt::getGlobalPose(*shape, *rigidActor);

        positions[i] = PhysicsMath::PxVec3ToVector3(globalPose.p);
        orientations[i] = PhysicsMath::PxQuatToQuaternion(globalPose.q);
    }
}
}
}
