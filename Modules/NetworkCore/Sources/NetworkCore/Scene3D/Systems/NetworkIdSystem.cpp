#include "NetworkCore/NetworkCoreUtils.h"

#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkIdSystem)
{
    ReflectionRegistrator<NetworkIdSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .End();
}

NetworkIdSystem::NetworkIdSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
    networkEntities = scene->GetSingletonComponent<NetworkEntitiesSingleComponent>();
    networkEntities->RegisterEntity(NetworkID::SCENE_ID, scene);
}

void NetworkIdSystem::AddEntity(Entity* entity)
{
    NetworkReplicationComponent* nrc = entity->GetComponent<NetworkReplicationComponent>();
    if (nullptr != nrc)
    {
        NetworkID id = nrc->GetNetworkID();
        if (NetworkID::INVALID == id)
        {
            id = GenerateUniqueId(entity);
            nrc->SetNetworkID(id);
        }
        networkEntities->RegisterEntity(id, entity);
    }
}

void NetworkIdSystem::RemoveEntity(Entity* entity)
{
    NetworkReplicationComponent* nrc = entity->GetComponent<NetworkReplicationComponent>();
    if (nullptr != nrc)
    {
        NetworkID id = nrc->GetNetworkID();
        networkEntities->UnregisterEntity(id);
    }
}

void NetworkIdSystem::PrepareForRemove()
{
}

bool NetworkIdSystem::IsGeneratedFromAction(NetworkID entityId)
{
    return ((static_cast<uint32>(entityId) & 0x80000000) > 0);
}

NetworkID NetworkIdSystem::GetEntityIdFromAction(FrameActionID frameActionId)
{
    return NetworkID(frameActionId.pureId | 0x80000000);
}

NetworkID NetworkIdSystem::GetEntityIdForStaticObject()
{
    static NetworkID staticObjectId = NetworkID::FIRST_STATIC_OBJ_ID;
    return staticObjectId++;
}

bool NetworkIdSystem::IsEntityIdForStaticObject(NetworkID networkID)
{
    return networkID >= NetworkID::FIRST_STATIC_OBJ_ID && networkID < NetworkID::FIRST_SERVER_ID;
}

bool NetworkIdSystem::IsPredicted(Entity* entity, const Type* componentType)
{
    NetworkPredictComponent* npc = entity->GetComponent<NetworkPredictComponent>();
    if (nullptr != npc)
    {
        return npc->IsPredictedComponent(componentType);
    }
    return false;
}

NetworkID NetworkIdSystem::GenerateUniqueId(Entity* entity)
{
    static NetworkID serverUniqueId = NetworkID::FIRST_SERVER_ID;
    static const bool isServer = IsServer(this);

    NetworkID ret = NetworkID::INVALID;

    NetworkPredictComponent* npc = entity->GetComponent<NetworkPredictComponent>();
    if (npc != nullptr)
    {
        FrameActionID frameActionId = npc->GetFrameActionID();

        if (0 != frameActionId.pureId)
        {
            DVASSERT(0 == (frameActionId.pureId & 0x80000000));

            ret = GetEntityIdFromAction(frameActionId);
        }
        else
        {
            DVASSERT(isServer && "frameActionId can be empty only on server side");
        }
    }

    if (isServer && ret == NetworkID::INVALID)
    {
        ret = serverUniqueId;
        ++serverUniqueId;
    }

    return ret;
}

} // namespace DAVA
