#include "GameShowSystem.h"

#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameShowSystem)
{
    ReflectionRegistrator<GameShowSystem>::Begin()[M::Tags("gameshow", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &GameShowSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY_END, SP::Type::NORMAL, 1000.0f)]
    .End();
}

GameShowSystem::GameShowSystem(DAVA::Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPlayerComponent>())
{
    DVASSERT(IsClient(scene));
}

void GameShowSystem::AddEntity(Entity* entity)
{
    DVASSERT(!playerComponent);
    playerComponent = entity->GetComponent<NetworkPlayerComponent>();
    DVASSERT(playerComponent);
}

void GameShowSystem::RemoveEntity(Entity* entity)
{
    DVASSERT(playerComponent);
    playerComponent = nullptr;
}

void GameShowSystem::Process(DAVA::float32 timeElapsed)
{
    if (playerComponent)
    {
        NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>();
        for (auto& it : disclosureEntities)
        {
            it.second = false;
        }
        for (size_t i = 0; i < playerComponent->visibleEntityIds.size(); ++i)
        {
            NetworkID netEntityId = playerComponent->visibleEntityIds[i];
            disclosureEntities[netEntityId] = true;
        }

        for (const auto& it : disclosureEntities)
        {
            const NetworkID netEntityId = it.first;
            const bool isVisible = it.second;
            Entity* entity = networkEntities->FindByID(netEntityId);
            if (entity)
            {
                entity->SetVisible(isVisible);
            }
        }
    }
}
