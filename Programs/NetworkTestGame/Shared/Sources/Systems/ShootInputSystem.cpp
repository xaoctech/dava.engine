#include "ShootInputSystem.h"

#include "InputUtils.h"

#include "Components/ExplosiveRocketComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Systems/NetworkIdSystem.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkFactoryComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>

#include <NetworkCore/NetworkFactoryUtils.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Core/BoxShapeComponent.h>
#include <Scene3D/Components/TransformComponent.h>

#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

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

    actionsSingleComponent = scene->GetSingleComponent<ActionsSingleComponent>();

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

            // on re-simulation bullet can be already in scene
            // so try to find it
            NetworkID bulletId = NetworkID::CreatePlayerActionId(playerID, clientFrameId, static_cast<uint32>(isFirstShoot));
            Entity* bullet = GetScene()->GetSingleComponent<NetworkEntitiesSingleComponent>()->FindByID(bulletId);

            if (!bullet)
            {
                if (isSecondShoot)
                {
                    bullet = new Entity;

                    NetworkReplicationComponent* bulletReplComp = new NetworkReplicationComponent(bulletId);

                    ExplosiveRocketComponent* rocketComp = new ExplosiveRocketComponent();
                    rocketComp->shooterId = shooterReplComp->GetNetworkID();
                    bullet->AddComponent(rocketComp);

                    ComponentMask predictionComponentMask;
                    predictionComponentMask.Set<ExplosiveRocketComponent>();
                    bulletReplComp->SetForReplication<ExplosiveRocketComponent>(M::Privacy::PUBLIC);

                    predictionComponentMask.Set<NetworkTransformComponent>();
                    NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictionComponentMask);
                    bullet->AddComponent(networkPredictComponent);
                    bullet->AddComponent(new NetworkTransformComponent());

                    if (IsServer(this))
                    {
                        bullet->AddComponent(new ObservableComponent());
                        bullet->AddComponent(new SimpleVisibilityShapeComponent());
                    }

                    bulletReplComp->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
                    bullet->AddComponent(bulletReplComp);
                }
                else
                {
                    NetworkFactoryComponent* factoryComponent = CreateFactoryEntity("Bullet", bulletId);
                    bullet = factoryComponent->GetEntity();
                    const TransformComponent* src = shooter->GetComponent<TransformComponent>();
                    factoryComponent->SetInitialTransform(src->GetPosition(), src->GetRotation());
                    factoryComponent->OverrideField("DynamicBodyComponent/BodyFlags",
                                                    PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);

                    {
                        //TODO : Two syntax equal variants.

                        SETUP_AFTER_INIT(factoryComponent, BoxShapeComponent, c)
                        {
                            Entity* rocketModel = c->GetEntity()->FindByName("Model");
                            const AABBox3 bbox = rocketModel->GetWTMaximumBoundingBoxSlow();
                            c->SetHalfSize(bbox.GetSize() / 2.0);
                        };

                        factoryComponent->SetupAfterInit<BoxShapeComponent>([shooterReplComp](BoxShapeComponent* c)
                                                                            {
                                                                                Entity* rocketModel = c->GetEntity()->FindByName(
                                                                                "Model");
                                                                                const AABBox3 bbox = rocketModel->GetWTMaximumBoundingBoxSlow();
                                                                                c->SetHalfSize(bbox.GetSize() / 2.0);
                                                                            });
                    }

                    factoryComponent->SetupAfterInit([](Entity* e)
                                                     {
                                                         Logger::Debug("SetupAfterInit entity:%s", e->GetName());
                                                     });
                }

                GetScene()->AddNode(bullet);
                shootCooldownComponent->SetLastShootFrameId(clientFrameId);
            }

            Logger::Debug("Vehicle:%u player:%d shoots with action: %u | Frame: %u", static_cast<uint32>(shooterReplComp->GetNetworkID()), playerID, static_cast<uint32>(bulletId), clientFrameId);
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
