#include "InvaderAttackSystem.h"
#include "InputUtils.h"

#include "Components/HealthComponent.h"
#include "Components/PlayerInvaderComponent.h"
#include "Components/AI/InvaderBehaviorComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Systems/ActionCollectSystem.h"

#include <Debug/ProfilerCPU.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkPhysics/NetworkPhysicsUtils.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/PhysicsSystem.h>
#include <physx/PxQueryFiltering.h>
#include <physx/PxQueryReport.h>
#include <physx/PxRigidActor.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderSystem.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(InvaderAttackSystem)
{
    ReflectionRegistrator<InvaderAttackSystem>::Begin()[M::Tags("gm_invaders", "input")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &InvaderAttackSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 6.2f)]
    .End();
}

namespace InvaderAttackSystemDetail
{
static const Vector3 SHOOT_OFFSET(0.f, 10.f, 0.f);
static const Vector3 AIM_OFFSET(0.f, 100.f, 0.f);
static const float32 MAX_SHOOTING_DISTANCE = 100.f;
}

InvaderAttackSystem::InvaderAttackSystem(DAVA::Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<PlayerInvaderComponent>())
{
    using namespace InvaderAttackSystemDetail;

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();

    uint32 mouseId = InputUtils::GetMouseDeviceId();
    uint32 keyboardId = InputUtils::GetKeyboardDeviceId();
    actionsSingleComponent->CollectDigitalAction(FIRST_SHOOT, eInputElements::MOUSE_LBUTTON, mouseId);
    actionsSingleComponent->CollectDigitalAction(FIRST_SHOOT, eInputElements::KB_SPACE, keyboardId);

    optionsComp = scene->GetSingletonComponent<BattleOptionsSingleComponent>();
    if (!optionsComp->options.gameStatsLogPath.empty())
    {
        statsComp = scene->GetSingletonComponent<StatsLoggingSingleComponent>();
    }

    entityGroup = scene->AquireEntityGroup<PlayerInvaderComponent>();
}

void InvaderAttackSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("InvaderAttackSystem::ProcessFixed");

    for (Entity* entity : entityGroup->GetEntities())
    {
        if (ownerEntity == nullptr && IsClientOwner(this, entity))
        {
            ownerEntity = entity;
        }

        if (IsReSimulating() && !IsClientOwner(this, entity))
        {
            continue;
        }

        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        if (!allActions.empty())
        {
            const auto& actions = allActions.back();
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
            ApplyAnalogActions(entity, actions.analogActions, actions.clientFrameId, timeElapsed);
        }
    }
}

void InvaderAttackSystem::ApplyDigitalActions(Entity* entity, const Vector<FastName>& actions,
                                              uint32 clientFrameId, float32 duration)
{
    using namespace InvaderAttackSystemDetail;

    for (const FastName& action : actions)
    {
        if (action == FIRST_SHOOT)
        {
            Vector3 shootStart = entity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslationVector() + SHOOT_OFFSET;
            Vector3 shootEnd = shootStart + AIM_OFFSET;
            Vector3 shootDirection = AIM_OFFSET;
            shootDirection.Normalize();

            // debug draw shot
            RenderHelper* renderHelper = GetScene()->GetRenderSystem()->GetDebugDrawer();
            renderHelper->DrawLine(shootStart, shootEnd, Color::White);

            if (IsServer(GetScene()))
            {
                // check if there was a strike
                Entity* victim = nullptr;
                if (optionsComp->isEnemyRewound)
                {
                    victim = ShootInPast(entity, shootStart, shootDirection, clientFrameId);
                }
                else
                {
                    victim = Shoot(shootStart, shootDirection);
                }

                if (victim)
                {
                    HealthComponent* healthComp = victim->GetComponent<HealthComponent>();
                    DVASSERT(healthComp);

                    healthComp->DecHealth(2, 0);
                    if (healthComp->GetHealth() <= 0)
                    {
                        healthComp->SetHealth(10);
                    }

                    LogStrike(entity, victim, clientFrameId);
                }
            }
            else
            {
                // check if client saw the strike
                Entity* victim = Shoot(shootStart, shootDirection);
                if (victim)
                {
                    LogStrike(entity, victim, clientFrameId);
                }
            }
        }
    }
}

void InvaderAttackSystem::ApplyAnalogActions(Entity* entity, const AnalogActionsMap& actions,
                                             uint32 clientFrameId, float32 duration)
{
}

Entity* InvaderAttackSystem::Shoot(const Vector3& shootStart, const Vector3& shootDirection) const
{
    using namespace InvaderAttackSystemDetail;

    PhysicsSystem* physics = GetScene()->GetSystem<PhysicsSystem>();
    DVASSERT(physics);

    physx::PxQueryFilterData queryFilter;
    queryFilter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

    physx::PxRaycastBuffer hitBuffer;
    bool collision = physics->Raycast(shootStart, shootDirection, MAX_SHOOTING_DISTANCE, hitBuffer, queryFilter, nullptr);
    if (collision)
    {
        Component* component = static_cast<Component*>(hitBuffer.block.actor->userData);
        if (component->GetType() == Type::Instance<DynamicBodyComponent>())
        {
            return component->GetEntity();
        }
    }

    return nullptr;
}

Entity* InvaderAttackSystem::ShootInPast(const Entity* shooter, const Vector3& shootStart, const Vector3& shootDirection,
                                         uint32 clientFrameId) const
{
    using namespace InvaderAttackSystemDetail;

    NetworkReplicationComponent* replComp = shooter->GetComponent<NetworkReplicationComponent>();
    DVASSERT(replComp);

    const FastName& token = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>()->GetToken(replComp->GetNetworkPlayerID());
    DVASSERT(token.IsValid());

    NetworkTimeSingleComponent* timeSingleComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    DVASSERT(timeSingleComp);

    int32 fdiff = timeSingleComp->GetClientViewDelay(token, clientFrameId);
    DVASSERT(fdiff > 0);

    uint32 numFramesToRollBack = std::min(10, fdiff);
    uint32 pastFrameId = clientFrameId == 0 ? timeSingleComp->GetFrameId() : clientFrameId - numFramesToRollBack;

    physx::PxRaycastHit hit;
    ComponentMask possibleComponents = ComponentUtils::MakeMask<DynamicBodyComponent>();
    bool collision = NetworkPhysicsUtils::GetRaycastHitInPast(*GetScene(), possibleComponents, shootStart, shootDirection, MAX_SHOOTING_DISTANCE, pastFrameId, nullptr, hit);
    if (collision)
    {
        Component* component = static_cast<Component*>(hit.actor->userData);
        if (component->GetType() == Type::Instance<DynamicBodyComponent>())
        {
            return component->GetEntity();
        }
    }

    return nullptr;
}

void InvaderAttackSystem::LogStrike(const Entity* shooter, const Entity* victim, uint32 clientFrameID) const
{
    if (!statsComp)
    {
        return;
    }

    if (clientFrameID == 0)
    {
        clientFrameID = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>()->GetFrameId();
    }

    if (IsServer(GetScene()))
    {
        String msg = Format("frameID %d sync -1 scenario unknown view SERVER - player %d strikes player %d",
                            clientFrameID,
                            shooter->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID(),
                            victim->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID());
        statsComp->AddMessage(msg);
    }
    else
    {
        DVASSERT(ownerEntity);
        InvaderBehaviorComponent* ownerBehaviorComp = ownerEntity->GetComponent<InvaderBehaviorComponent>();
        DVASSERT(ownerBehaviorComp);
        InvaderBehaviorComponent* shooterBehaviorComp = shooter->GetComponent<InvaderBehaviorComponent>();
        DVASSERT(shooterBehaviorComp);
        InvaderBehaviorComponent* victimBehaviorComp = victim->GetComponent<InvaderBehaviorComponent>();
        DVASSERT(victimBehaviorComp);

        String msg = Format("frameID %d sync %d scenario %s view %s - %s %d strikes %s %d",
                            clientFrameID, ownerBehaviorComp->GetSyncCounter(),
                            InvaderBehaviorComponent::scenarioNames[ownerBehaviorComp->GetScenario()].c_str(),
                            InvaderBehaviorComponent::roleNames[ownerBehaviorComp->GetRole()].c_str(),
                            InvaderBehaviorComponent::roleNames[shooterBehaviorComp->GetRole()].c_str(),
                            shooter->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID(),
                            InvaderBehaviorComponent::roleNames[victimBehaviorComp->GetRole()].c_str(),
                            victim->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID());
        statsComp->AddMessage(msg);
    }
}
