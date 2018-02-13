#include "NetworkPhysics/NetworkPhysicsUtils.h"

#include <Base/UnordererMap.h>

#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h>
#include <NetworkCore/SnapshotUtils.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/PhysicsUtils.h>

#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>

#include <Engine/Engine.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

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
                                            return (possibleComponents & entityComponentMask).any();
                                        });

    CharacterMirrorsSingleComponent* mirrorsSingleComponent = scene.GetSingletonComponent<CharacterMirrorsSingleComponent>();

    // Save current network transforms

    struct Transform
    {
        Vector3 position;
        Quaternion orientation;
    };

    UnorderedMap<Entity*, Transform> oldTransforms;
    for (Entity* e : dynamics)
    {
        DVASSERT(e != nullptr);

        Entity* mirror = mirrorsSingleComponent->GetMirrorForCharacter(e);
        if (mirror)
        {
            // no need to restore mirrors
            continue;
        }

        TransformComponent* transformComponent = e->GetComponent<TransformComponent>();
        DVASSERT(transformComponent != nullptr);

        oldTransforms[e] = { transformComponent->GetPosition(), transformComponent->GetRotation() };
    }

    // Roll back transforms

    SnapshotSingleComponent* snapshotSingleComponent = scene.GetSingletonComponent<SnapshotSingleComponent>();
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

        SnapshotComponentKey componentKey(ComponentUtils::GetRuntimeIndex<NetworkTransformComponent>(), 0);
        bool applyResult = SnapshotUtils::ApplySnapshot(snapshot, replicationComponent->GetNetworkID(), componentKey, networkTransformComponent);
        if (!applyResult)
        {
            return false;
        }

        Entity* mirror = mirrorsSingleComponent->GetMirrorForCharacter(e);
        TransformComponent* transformComponent = mirror ? mirror->GetComponent<TransformComponent>() : e->GetComponent<TransformComponent>();
        DVASSERT(transformComponent != nullptr);

        transformComponent->SetLocalTransform(
        networkTransformComponent->GetPosition(),
        networkTransformComponent->GetOrientation(),
        transformComponent->GetScale());
    }

    PhysicsSystem* physicsSystem = scene.GetSystem<PhysicsSystem>();
    DVASSERT(physicsSystem != nullptr);

    // Sync transforms so that raycast uses new positions
    physicsSystem->SyncTransformToPhysx();

    // Raycast
    bool result = false;

    physx::PxQueryFilterData queryFilter;
    queryFilter.flags = physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

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
        Entity* mirror = mirrorsSingleComponent->GetMirrorForCharacter(e);
        if (mirror)
        {
            // no need to restore mirrors
            continue;
        }

        NetworkTransformComponent* networkTransformComponent = e->GetComponent<NetworkTransformComponent>();
        DVASSERT(networkTransformComponent != nullptr);

        TransformComponent* transformComponent = e->GetComponent<TransformComponent>();
        DVASSERT(transformComponent);

        Transform& transform = oldTransforms[e];

        networkTransformComponent->SetPosition(transform.position);
        networkTransformComponent->SetOrientation(transform.orientation);

        transformComponent->SetLocalTransform(transform.position, transform.orientation, transformComponent->GetScale());
    }

    // Sync restored transforms for potential future raycasts on same frame
    physicsSystem->SyncTransformToPhysx();

    // Return result

    return result;
}
}
}
