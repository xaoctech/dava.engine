#include "NetworkCore/NetworkCoreUtils.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Entity/SceneSystem.h>
#include <Debug/DVAssert.h>
#include <Reflection/ReflectedTypeDB.h>

namespace DAVA
{
bool IsServer(Scene* scene)
{
    DVASSERT(scene != nullptr);
    return nullptr != scene->GetSingleComponent<NetworkServerSingleComponent>()->GetServer();
}

bool IsServer(SceneSystem* sceneSystem)
{
    DVASSERT(sceneSystem != nullptr);
    return IsServer(sceneSystem->GetScene());
}

bool IsClient(Scene* scene)
{
    DVASSERT(scene != nullptr);
    return nullptr != scene->GetSingleComponent<NetworkClientSingleComponent>()->GetClient();
}

bool IsClient(SceneSystem* sceneSystem)
{
    DVASSERT(sceneSystem != nullptr);
    return IsClient(sceneSystem->GetScene());
}

bool IsClientOwner(Scene* scene, const Entity* entity)
{
    DVASSERT(scene);
    const bool isClient = IsClient(scene);
    const NetworkGameModeSingleComponent* netGameModeComp = scene->GetSingleComponent<NetworkGameModeSingleComponent>();
    const NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
    if (isClient && netGameModeComp && netReplComp)
    {
        NetworkPlayerID entityPlayerID = netReplComp->GetNetworkPlayerID();
        NetworkPlayerID currentPlayerID = netGameModeComp->GetNetworkPlayerID();
        return (entityPlayerID == currentPlayerID);
    }
    return false;
}

bool IsClientOwner(SceneSystem* sceneSystem, const Entity* entity)
{
    DVASSERT(sceneSystem);
    return IsClientOwner(sceneSystem->GetScene(), entity);
}

bool IsClientOwner(const Entity* entity)
{
    Scene* scene = const_cast<Entity*>(entity)->GetScene();
    DVASSERT(scene);
    return IsClientOwner(scene, entity);
}

Entity* GetEntityWithNetworkId(Scene* scene, NetworkID networkId)
{
    DVASSERT(scene != nullptr);

    Vector<Entity*> children;
    scene->GetChildEntitiesWithComponent(children, Type::Instance<NetworkReplicationComponent>());

    for (Entity* child : children)
    {
        DVASSERT(child != nullptr);
        if (child->GetComponent<NetworkReplicationComponent>()->GetNetworkID() == networkId)
        {
            return child;
        }
    }

    return nullptr;
}

M::OwnershipRelation GetPlayerOwnershipRelation(const Entity* entity, NetworkPlayerID playerId)
{
    const NetworkReplicationComponent* replicationComponent = entity->GetComponent<NetworkReplicationComponent>();
    DVASSERT(replicationComponent);
    const NetworkPlayerID entityOwnerId = replicationComponent->GetNetworkPlayerID();
    if (entityOwnerId == playerId)
    {
        return M::OwnershipRelation::OWNER;
    }

    /*  TODO: Not implemented.
    if (GetTeam(entityOwnerId) == GetTeam(playerId))
    {
        return M::OwnershipRelation::TEAMMATE;
    }
    */

    return M::OwnershipRelation::ENEMY;
}

Vector<ActionsSingleComponent::Actions>& GetCollectedActionsForClient(Scene* scene, const Entity* clientEntity)
{
    NetworkReplicationComponent* networkReplicationComponent = clientEntity->GetComponent<NetworkReplicationComponent>();
    DVASSERT(networkReplicationComponent != nullptr);
    NetworkPlayerID playerId = networkReplicationComponent->GetNetworkPlayerID();

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingleComponent<ActionsSingleComponent>();

    return actionsSingleComponent->GetActions(playerId);
}

void AddActionsForClient(const SceneSystem* system, const Entity* clientEntity, ActionsSingleComponent::Actions&& actions)
{
    NetworkReplicationComponent* networkReplicationComponent = clientEntity->GetComponent<NetworkReplicationComponent>();
    DVASSERT(networkReplicationComponent != nullptr);
    NetworkPlayerID playerId = networkReplicationComponent->GetNetworkPlayerID();

    ActionsSingleComponent* actionsSingleComponent = system->GetScene()->GetSingleComponentForWrite<ActionsSingleComponent>(system);
    DVASSERT(actionsSingleComponent != nullptr);

    actionsSingleComponent->AddActions(playerId, std::move(actions));
}

void AddDigitalActionForClient(const SceneSystem* system, const Entity* clientEntity, const FastName& action)
{
    NetworkReplicationComponent* networkReplicationComponent = clientEntity->GetComponent<NetworkReplicationComponent>();
    DVASSERT(networkReplicationComponent != nullptr);
    NetworkPlayerID playerId = networkReplicationComponent->GetNetworkPlayerID();

    ActionsSingleComponent* actionsSingleComponent = system->GetScene()->GetSingleComponentForWrite<ActionsSingleComponent>(system);
    DVASSERT(actionsSingleComponent != nullptr);

    actionsSingleComponent->AddDigitalAction(action, playerId);
}

void AddAnalogActionForClient(const SceneSystem* system, const Entity* clientEntity, const FastName& action, const Vector2& data)
{
    NetworkReplicationComponent* networkReplicationComponent = clientEntity->GetComponent<NetworkReplicationComponent>();
    DVASSERT(networkReplicationComponent != nullptr);
    NetworkPlayerID playerId = networkReplicationComponent->GetNetworkPlayerID();

    ActionsSingleComponent* actionsSingleComponent = system->GetScene()->GetSingleComponentForWrite<ActionsSingleComponent>(system);
    DVASSERT(actionsSingleComponent != nullptr);

    actionsSingleComponent->AddAnalogAction(action, data, playerId);
}

const uint32 NetworkCoreUtils::ENET_DEFAULT_MTU_UNCOMPRESSED =
2 * ENET_HOST_DEFAULT_MTU - GetMaxCompressedSizeWithLz4(ENET_HOST_DEFAULT_MTU);

NetworkID NetworkCoreUtils::GetEntityId(Entity* entity)
{
    if (nullptr != entity)
    {
        NetworkReplicationComponent* nrc = entity->GetComponent<NetworkReplicationComponent>();
        if (nullptr != nrc)
        {
            return nrc->GetNetworkID();
        }
        else
        {
            if (entity == entity->GetScene())
            {
                return NetworkID::SCENE_ID;
            }
            else
            {
                return NetworkID::INVALID;
            }
        }
    }

    return NetworkID::SCENE_ID;
}

uint32 NetworkCoreUtils::GetPlayerId(Entity* entity)
{
    DVASSERT(false && "Need to be implemented");
    return 0;
}
} // end namespace DAVA
