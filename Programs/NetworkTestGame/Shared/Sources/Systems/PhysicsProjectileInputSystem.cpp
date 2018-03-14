#include "PhysicsProjectileInputSystem.h"
#include "InputUtils.h"

#include "Systems/PhysicsProjectileSystem.h"
#include "Components/PhysicsProjectileComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/ActionCollectSystem.h>
#include <Scene3D/Components/TransformComponent.h>

#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>

#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Core/BoxShapeComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsProjectileInputSystem)
{
    ReflectionRegistrator<PhysicsProjectileInputSystem>::Begin()[M::Tags("gm_characters", "physics", "input")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &PhysicsProjectileInputSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 7.0f)]
    .End();
}

namespace PhysicsProjectileInputSystemDetail
{
static const FastName FIRE_MISSILE("FIRE_MISSILE");
static const FastName FIRE_GRENADE("FIRE_GRENADE");
}

PhysicsProjectileInputSystem::PhysicsProjectileInputSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkInputComponent>())
{
    using namespace PhysicsProjectileInputSystemDetail;

    uint32 mouseId = InputUtils::GetMouseDeviceId();
    uint32 keyboardId = InputUtils::GetKeyboardDeviceId();

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingleComponent<ActionsSingleComponent>();
    entityGroup = scene->AquireEntityGroup<NetworkInputComponent>();

    actionsSingleComponent->CollectDigitalAction(FIRE_MISSILE, eInputElements::MOUSE_LBUTTON, mouseId);
    actionsSingleComponent->CollectDigitalAction(FIRE_GRENADE, eInputElements::MOUSE_RBUTTON, mouseId);
    actionsSingleComponent->CollectDigitalAction(FIRE_MISSILE, eInputElements::KB_SPACE, keyboardId);
    actionsSingleComponent->CollectDigitalAction(FIRE_GRENADE, eInputElements::KB_LSHIFT, keyboardId);
}

void PhysicsProjectileInputSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("PhysicsProjectileInputSystem::ProcessFixed");

    for (Entity* entity : entityGroup->GetEntities())
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        for (const auto& actions : allActions)
        {
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
        }
    }
}

Entity* PhysicsProjectileInputSystem::CreateProjectileModel() const
{
    static Entity* bulletModel = nullptr;

    if (nullptr == bulletModel)
    {
        ScopedPtr<Scene> model(new Scene());
        SceneFileV2::eError err = model->LoadScene("~res:/Sniper_2.sc2");
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        bulletModel = model->GetEntityByID(1)->GetChild(1)->Clone();
    }

    return bulletModel->Clone();
}

void PhysicsProjectileInputSystem::ApplyDigitalActions(Entity* entity, const Vector<FastName>& actions, uint32 clientFrameId, float32 duration)
{
    using namespace PhysicsProjectileInputSystemDetail;

    static uint32 lastShootFrameId = 0;

    if (clientFrameId - lastShootFrameId < 30)
    {
        return;
    }

    for (const FastName& action : actions)
    {
        if (action == FIRE_MISSILE || action == FIRE_GRENADE)
        {
            NetworkPlayerID playerID = entity->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID();

            TransformComponent* shooterTransform = entity->GetComponent<TransformComponent>();
            PhysicsProjectileComponent* projectileComponent = new PhysicsProjectileComponent();

            // Instantiate
            Entity* projectile = CreateProjectileModel();
            TransformComponent* projectileTransform = projectile->GetComponent<TransformComponent>();
            projectileTransform->SetLocalTransform(shooterTransform->GetPosition(), shooterTransform->GetRotation(), shooterTransform->GetScale());
            projectile->AddComponent(projectileComponent);

            projectileComponent->SetInitialPosition(projectileTransform->GetPosition());

            ComponentMask predictionComponentMask;

            NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictionComponentMask);
            projectile->AddComponent(networkPredictComponent);
            projectile->AddComponent(new NetworkTransformComponent());

            BoxShapeComponent* boxShape = new BoxShapeComponent();
            boxShape->SetHalfSize(Vector3(0.3f, 2.0f, 0.3f));
            boxShape->SetTypeMaskToCollideWith(1);
            boxShape->SetLocalPosition(Vector3(0.0f, 3.12f, 1.7f));
            projectile->AddComponent(boxShape);

            DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();

            if (action == FIRE_MISSILE)
            {
                projectileComponent->SetProjectileType(PhysicsProjectileComponent::eProjectileTypes::MISSILE);

                dynamicBody->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
                dynamicBody->SetLinearDamping(0.0f);
                dynamicBody->SetLinearVelocity(entity->GetComponent<TransformComponent>()->GetRotation().ApplyToVectorFast(Vector3(0.0f, 30.0f, 0.0f)));
            }
            else
            {
                projectileComponent->SetProjectileType(PhysicsProjectileComponent::eProjectileTypes::GRENADE);

                Vector3 velocity = (entity->GetComponent<TransformComponent>()->GetRotation()).ApplyToVectorFast(Vector3(0.0f, 15.0f, 10.0f));
                dynamicBody->SetLinearVelocity(velocity);
                dynamicBody->SetAngularVelocity(Vector3(-10.0f, 0.0f, 0.0f));
            }

            projectile->AddComponent(dynamicBody);

            NetworkID projectileId = NetworkID::CreatePlayerActionId(playerID, clientFrameId, (action == FIRE_MISSILE) ? 1 : 2);
            NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(projectileId);
            projectile->AddComponent(replicationComponent);

            GetScene()->AddNode(projectile);
            SafeRelease(projectile);

            lastShootFrameId = clientFrameId;
        }
    }
}
