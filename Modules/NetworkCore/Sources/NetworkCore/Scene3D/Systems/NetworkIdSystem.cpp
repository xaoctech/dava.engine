#include "NetworkCore/NetworkCoreUtils.h"

#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"

#include <Debug/DVAssert.h>
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
    networkEntities = scene->GetSingleComponentForWrite<NetworkEntitiesSingleComponent>(this);
    networkEntities->RegisterEntity(NetworkID::SCENE_ID, scene);
    isServer = IsServer(this);
}

void NetworkIdSystem::AddEntity(Entity* entity)
{
    NetworkReplicationComponent* nrc = entity->GetComponent<NetworkReplicationComponent>();
    if (nullptr != nrc && nrc->id != NetworkID::SCENE_ID)
    {
        DVASSERT(nrc->id.IsValid());

        if (nrc->id.IsStaticId())
        {
            // TODO:
            // may be some asserts
            // ...
        }
        else if (nrc->id.IsPlayerOwnId())
        {
            if (!nrc->id.IsValid())
            {
                DVASSERT(isServer, "PlayerOwdId can be verified only on server side");
            }
        }
        else
        {
            DVASSERT(nrc->id.IsPlayerActionId());
            DVASSERT(nrc->id.GetPlayerId() > 0);
            DVASSERT(nrc->id.GetPlayerActionFrameId() > 0);
        }

        networkEntities->RegisterEntity(nrc->id, entity);
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

} // namespace DAVA
