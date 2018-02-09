#include "ShootInputSystem.h"

#include "Components/ShootComponent.h"
#include "Components/ExplosiveRocketComponent.h"
#include "Components/ShootCooldownComponent.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include <Reflection/ReflectionRegistrator.h>
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/ActionCollectSystem.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"
#include "Scene3D/Components/TransformComponent.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Snapshot.h"

#include "Debug/ProfilerCPU.h"
#include "Logger/Logger.h"

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ShootInputSystem)
{
    ReflectionRegistrator<ShootInputSystem>::Begin()[M::Tags("input", "shoot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShootInputSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 6.0f)]
    .End();
}

namespace ShootInputSystemDetail
{
static const FastName FIRST_SHOOT("FIRST_SHOOT");
static const FastName SECOND_SHOOT("SECOND_SHOOT");
static const uint32 COOLDOWN = 10;
}

ShootInputSystem::ShootInputSystem(Scene* scene)
    : INetworkInputSimulationSystem(scene, ComponentUtils::MakeMask<NetworkInputComponent>() | ComponentUtils::MakeMask<ShootCooldownComponent>())
{
    using namespace ShootInputSystemDetail;
    uint32 mouseId = GetMouseDeviceId();
    uint32 keyboardId = GetKeyboardDeviceId();

    actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();

    actionsSingleComponent->CollectDigitalAction(FIRST_SHOOT, eInputElements::MOUSE_LBUTTON, mouseId);
    actionsSingleComponent->CollectDigitalAction(SECOND_SHOOT, eInputElements::MOUSE_RBUTTON, mouseId);
    actionsSingleComponent->CollectDigitalAction(FIRST_SHOOT, eInputElements::KB_SPACE, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SECOND_SHOOT, eInputElements::KB_LSHIFT, keyboardId);
}

void ShootInputSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("ShootInputSystem::ProcessFixed");

    for (Entity* entity : entities)
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        for (const auto& actions : allActions)
        {
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
        }
    }
}

void ShootInputSystem::ApplyDigitalActions(Entity* shooter,
                                           const Vector<FastName>& actions,
                                           uint32 clientFrameId,
                                           float32 duration) const
{
    if (!CanShoot(shooter))
    {
        return;
    }

    uint32 cooldown = ShootInputSystemDetail::COOLDOWN;
    ShootCooldownComponent* shootCooldownComponent = shooter->GetComponent<ShootCooldownComponent>();
    for (const FastName& action : actions)
    {
        const bool isFirstShoot = (action == ShootInputSystemDetail::FIRST_SHOOT);
        const bool isSecondShoot = (action == ShootInputSystemDetail::SECOND_SHOOT);
        const bool isAnyShoot = (isFirstShoot || isSecondShoot);

        uint32 nextFrameForShoot = shootCooldownComponent->GetLastShootFrameId() + cooldown;
        if (isAnyShoot && clientFrameId >= nextFrameForShoot)
        {
            const NetworkReplicationComponent* shooterReplComp = shooter->GetComponent<NetworkReplicationComponent>();
            NetworkPlayerID playerID = shooterReplComp->GetNetworkPlayerID();
            FrameActionID shootActionId(clientFrameId, playerID, static_cast<uint32>(isFirstShoot));
            // on re-simulation bullet can be already in scene
            // so try to find it
            NetworkID entityId = NetworkIdSystem::GetEntityIdFromAction(shootActionId);
            Entity* bullet = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>()->FindByID(entityId);
            if (!bullet)
            {
                bullet = new Entity;
                NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent();
                if (isSecondShoot)
                {
                    ExplosiveRocketComponent* rocketComp = new ExplosiveRocketComponent();
                    rocketComp->shooterId = shooterReplComp->GetNetworkID();
                    bullet->AddComponent(rocketComp);
                    networkPredictComponent->AddPredictedComponent(Type::Instance<ExplosiveRocketComponent>());
                }
                else
                {
                    ShootComponent* shootComp = new ShootComponent();
                    shootComp->SetShootType(ShootComponent::ShootType::MAIN);
                    shootComp->SetShooter(shooter);
                    bullet->AddComponent(shootComp);
                    networkPredictComponent->AddPredictedComponent(Type::Instance<ShootComponent>());
                }

                NetworkReplicationComponent* bulletReplComp = new NetworkReplicationComponent();
                bulletReplComp->SetNetworkPlayerID(playerID);
                bullet->AddComponent(bulletReplComp);

                networkPredictComponent->SetFrameActionID(shootActionId);
                bullet->AddComponent(networkPredictComponent);
                bullet->AddComponent(new NetworkTransformComponent());
                networkPredictComponent->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
                GetScene()->AddNode(bullet);

                shootCooldownComponent->SetLastShootFrameId(clientFrameId);
            }

            Logger::Debug("Vehicle:%u player:%d shoots with action: %u | Frame: %u", static_cast<uint32>(shooterReplComp->GetNetworkID()), playerID, shootActionId.pureId, clientFrameId);
        }
    }
}

bool ShootInputSystem::CanShoot(const DAVA::Entity* entity) const
{
    const HealthComponent* healthComponent = entity->GetComponent<HealthComponent>();
    if (healthComponent && healthComponent->GetHealth() == 0)
    {
        return false;
    }

    const GameStunnableComponent* stunComp = entity->GetComponent<GameStunnableComponent>();
    if (stunComp && stunComp->IsStunned())
    {
        return false;
    }

    return true;
}
