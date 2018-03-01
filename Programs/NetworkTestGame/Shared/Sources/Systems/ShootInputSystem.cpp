#include "ShootInputSystem.h"

#include "InputUtils.h"

#include "Components/ExplosiveRocketComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Scene3D/Components/TransformComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ShootInputSystem)
{
    ReflectionRegistrator<ShootInputSystem>::Begin()[M::Tags("gameinput", "shoot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShootInputSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 6.0f)]
    .End();
}

namespace ShootInputSystemDetail
{
static const FastName FIRST_SHOOT("FIRST_SHOOT");
static const FastName SECOND_SHOOT("SECOND_SHOOT");
static const uint32 COOLDOWN = 10;
}

ShootInputSystem::ShootInputSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkInputComponent, ShootCooldownComponent>())
{
    using namespace ShootInputSystemDetail;
    uint32 mouseId = InputUtils::GetMouseDeviceId();
    uint32 keyboardId = InputUtils::GetKeyboardDeviceId();

    actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();

    actionsSingleComponent->CollectDigitalAction(FIRST_SHOOT, eInputElements::MOUSE_LBUTTON, mouseId);
    actionsSingleComponent->CollectDigitalAction(SECOND_SHOOT, eInputElements::MOUSE_RBUTTON, mouseId);
    actionsSingleComponent->CollectDigitalAction(FIRST_SHOOT, eInputElements::KB_SPACE, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SECOND_SHOOT, eInputElements::KB_LSHIFT, keyboardId);

    entityGroup = scene->AquireEntityGroup<NetworkInputComponent, ShootCooldownComponent>();
}

void ShootInputSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("ShootInputSystem::ProcessFixed");

    for (Entity* entity : entityGroup->GetEntities())
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        for (const auto& actions : allActions)
        {
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
        }
    }
}

void ShootInputSystem::ApplyDigitalActions(Entity* shooter, const Vector<FastName>& actions, uint32 clientFrameId, float32 duration)
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

                if (IsServer(GetScene()))
                {
                    bullet->AddComponent(new ObservableComponent());
                    bullet->AddComponent(new SimpleVisibilityShapeComponent());
                }

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
