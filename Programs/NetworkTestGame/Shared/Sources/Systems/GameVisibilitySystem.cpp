#include "GameVisibilitySystem.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Scene3D/Components/TransformComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "Components/PlayerTankComponent.h"

#include "Logger/Logger.h"

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameVisibilitySystem)
{
    ReflectionRegistrator<GameVisibilitySystem>::Begin()[M::Tags("server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &GameVisibilitySystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_END, SP::Type::FIXED, 2.0f)]
    .End();
}

GameVisibilitySystem::GameVisibilitySystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>())
    , maxAOI(5e6f)
    , periodIncreaseDistance(1e6f)
{
}

void GameVisibilitySystem::AddEntity(DAVA::Entity* entity)
{
    NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
    NetworkPlayerID playerID = netReplComp->GetNetworkPlayerID();
    CacheItem item{ entity, playerID, entity->GetComponent<TransformComponent>() };

    auto findIt = std::find_if(entities.begin(), entities.end(), [entity](const CacheItem& item) { return item.entity == entity; });
    if (findIt == entities.end())
    {
        entities.push_back(item);
        if (entity->GetComponent<PlayerTankComponent>() || netReplComp->GetEntityType() == EntityType::VEHICLE)
        {
            playerEntities.push_back(item);
        }
        ObserveEntity(item);
    }
}

void GameVisibilitySystem::RemoveEntity(DAVA::Entity* entity)
{
    auto ifPred = [entity](const CacheItem& item) { return item.entity == entity; };
    entities.erase(std::find_if(entities.begin(), entities.end(), ifPred), entities.end());
    NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
    if (entity->GetComponent<PlayerTankComponent>() || netReplComp->GetEntityType() == EntityType::VEHICLE)
    {
        playerEntities.erase(std::find_if(playerEntities.begin(), playerEntities.end(), ifPred), playerEntities.end());
    }

    NetworkVisibilitySingleComponent* singleComponent = GetScene()->GetSingletonComponent<NetworkVisibilitySingleComponent>();
    for (const auto& item : playerEntities)
    {
        singleComponent->SetVisibility(item.playerID, entity, 0);
    }
}

void GameVisibilitySystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("GameVisibilitySystem::ProcessFixed");
    for (const CacheItem& item : entities)
    {
        ObserveEntity(item);
    }
}

void GameVisibilitySystem::ObserveEntity(const CacheItem& item) const
{
    NetworkVisibilitySingleComponent* singleComponent = GetScene()->GetSingletonComponent<NetworkVisibilitySingleComponent>();
    NetworkPlayerID entityPlayerID = item.playerID;
    const Vector3& entityPos = item.transformComp->GetPosition();

    for (const CacheItem& playerItem : playerEntities)
    {
        const NetworkPlayerID observerPlayerID = playerItem.playerID;
        const Entity* observer = playerItem.entity;

        uint8 frequency = 0;

        if (observerPlayerID == entityPlayerID)
        {
            frequency = 1;
        }
        else
        {
            const Vector3& observerPos = playerItem.transformComp->GetPosition();
            float32 distance = Distance(entityPos, observerPos);

            if (distance < GetPeriodIncreaseDistance())
            {
                frequency = 1;
            }
            else if (distance < GetMaxAOI())
            {
                frequency = static_cast<uint8>(ceil(distance / GetPeriodIncreaseDistance()));
            }
        }

        singleComponent->SetVisibility(observerPlayerID, item.entity, frequency);
    }
}

void GameVisibilitySystem::SetMaxAOI(float32 maxAOI_)
{
    maxAOI = maxAOI_;
}

float32 GameVisibilitySystem::GetMaxAOI() const
{
    return maxAOI;
}

void GameVisibilitySystem::SetPeriodIncreaseDistance(float32 periodIncreaseDistance_)
{
    DVASSERT(periodIncreaseDistance_ > 0.f);
    periodIncreaseDistance = periodIncreaseDistance_;
}

float32 GameVisibilitySystem::GetPeriodIncreaseDistance() const
{
    return periodIncreaseDistance;
}
