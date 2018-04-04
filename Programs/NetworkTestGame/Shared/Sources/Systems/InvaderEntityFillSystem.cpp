#include "InvaderEntityFillSystem.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/PlayerInvaderComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/AI/InvaderBehaviorComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Visibility/ObserverComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include "Systems/InvaderAttackSystem.h"
#include "Systems/InvaderMovingSystem.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControlSystem.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTrafficLimitComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Core/BoxShapeComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/CameraComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(InvaderEntityFillSystem)
{
    ReflectionRegistrator<InvaderEntityFillSystem>::Begin()[M::Tags("gm_invaders")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &InvaderEntityFillSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 0.31f)]
    .End();
}

InvaderEntityFillSystem::InvaderEntityFillSystem(DAVA::Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
    optionsComp = scene->GetSingleComponent<BattleOptionsSingleComponent>();
    subscriber = scene->AquireEntityGroupOnAdd(scene->AquireEntityGroup<PlayerInvaderComponent>(), this);
}

void InvaderEntityFillSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (Entity* e : subscriber->entities)
    {
        FillEntity(e);
    }
    subscriber->entities.clear();
}

void InvaderEntityFillSystem::FillEntity(DAVA::Entity* entity)
{
    String filePath("~res:/Sniper_2.sc2");
    entity->SetName("Invader");

    if (IsServer(this))
    {
        PlayerInvaderComponent* invComponent = entity->GetComponent<PlayerInvaderComponent>();
        NetworkPlayerID playerId = invComponent->playerId;
        NetworkID invaderId = NetworkID::CreatePlayerOwnId(playerId);

        Logger::Debug("[InvaderEntityFillSystem] Create Invader %u For Player ID : %d", static_cast<uint32>(invaderId), playerId);

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(invaderId);
        replicationComponent->SetForReplication<PlayerInvaderComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<NetworkPlayerComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<NetworkInputComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<NetworkRemoteInputComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<HealthComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<GameStunnableComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ShootCooldownComponent>(M::Privacy::PRIVATE);

        entity->AddComponent(replicationComponent);
        entity->AddComponent(new NetworkInputComponent());

        NetworkRemoteInputComponent* netRemoteInputComp = new NetworkRemoteInputComponent();
        if (optionsComp->isEnemyPredicted)
        {
            netRemoteInputComp->AddActionToReplicate(InvaderMovingSystemDetail::UP);
            netRemoteInputComp->AddActionToReplicate(InvaderMovingSystemDetail::DOWN);
            netRemoteInputComp->AddActionToReplicate(InvaderMovingSystemDetail::LEFT);
            netRemoteInputComp->AddActionToReplicate(InvaderMovingSystemDetail::RIGHT);
            netRemoteInputComp->AddActionToReplicate(InvaderMovingSystemDetail::ACCELERATE);
        }
        netRemoteInputComp->AddActionToReplicate(InvaderAttackSystemDetail::FIRST_SHOOT);
        entity->AddComponent(netRemoteInputComp);

        entity->AddComponent(new ShootCooldownComponent());
        entity->AddComponent(new NetworkTransformComponent());
        entity->AddComponent(new NetworkPlayerComponent());
        entity->AddComponent(new GameStunnableComponent());
        entity->AddComponent(new HealthComponent());
        entity->AddComponent(new NetworkTrafficLimitComponent());
        entity->AddComponent(new ObserverComponent());
        entity->AddComponent(new ObservableComponent());
        entity->AddComponent(new SimpleVisibilityShapeComponent());
    }
    else
    {
        const bool isClientOwner = IsClientOwner(this, entity);
        if (isClientOwner)
        {
            NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
            Logger::Debug("[PlayerEntitySystem::Process] Set player:%d vehicle:%d", netReplComp->GetNetworkPlayerID(), entity->GetID());
            entity->SetName("MyInvader");
            filePath = "~res:/Sniper_1.sc2";

            Camera* camera = new Camera();
            camera->SetUp(Vector3(0.f, 0.f, 1.f));
            camera->SetPosition(Vector3(0.f, -50.f, 100.f));
            camera->SetTarget(Vector3(0.f, 0.f, 0.f));
            camera->RebuildCameraFromValues();
            VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
            const Size2i& physicalSize = vcs->GetPhysicalScreenSize();
            float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);
            camera->SetupPerspective(70.f, screenAspect, 0.5f, 2500.f);
            GetScene()->AddCamera(camera);
            GetScene()->SetCurrentCamera(camera);
            entity->AddComponent(new CameraComponent(camera));
        }

        if (isClientOwner || optionsComp->isEnemyPredicted)
        {
            NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent();
            networkPredictComponent->SetForPrediction<NetworkTransformComponent>();
            entity->AddComponent(networkPredictComponent);
        }
    }

    Entity* invaderModel = GetModel(filePath);
    if (IsClient(this))
    {
        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = invaderModel->GetWTMaximumBoundingBoxSlow();
        entity->AddComponent(debugDrawComponent);

        if (optionsComp->options.playerKind.GetId() == PlayerKind::Id::INVADER_BOT)
        {
            bool isActor = IsClientOwner(this, entity);
            InvaderBehaviorComponent* behaviorComponent = new InvaderBehaviorComponent(isActor);
            entity->AddComponent(behaviorComponent);
        }
    }

    entity->AddNode(invaderModel);
    invaderModel->Release();

    // we need shape & body on server and client
    // to be able to compare collision checks on server and client
    BoxShapeComponent* boxShape = new BoxShapeComponent();
    const AABBox3 bbox = invaderModel->GetWTMaximumBoundingBoxSlow();
    boxShape->SetHalfSize(bbox.GetSize() / 2.0);
    boxShape->SetTypeMask(1);
    boxShape->SetTypeMaskToCollideWith(2);
    boxShape->SetOverrideMass(true);
    boxShape->SetMass(10000.f);
    entity->AddComponent(boxShape);

    DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
    dynamicBody->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
    entity->AddComponent(dynamicBody);
}

DAVA::Entity* InvaderEntityFillSystem::GetModel(const DAVA::String& pathname) const
{
    if (modelCache.find(pathname) == modelCache.end())
    {
        DAVA::FilePath name(pathname);
        ScopedPtr<Scene> model(new Scene());
        SceneFileV2::eError err = model->LoadScene(DAVA::FilePath(pathname));
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        modelCache.emplace(pathname, model->GetEntityByID(1)->Clone());
    }

    return modelCache[pathname]->Clone();
}
