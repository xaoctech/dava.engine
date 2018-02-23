#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "ShooterRocketSystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"

#include "Utils/Random.h"
#include "ShooterConstants.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include "Components/DamageComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShooterRocketComponent.h"

#include <Physics/PhysicsSystem.h>
#include <Physics/CollisionSingleComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/BoxShapeComponent.h>
#include <Physics/CapsuleCharacterControllerComponent.h>

#include <Render/Highlevel/RenderObject.h>

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterRocketSystem)
{
    ReflectionRegistrator<ShooterRocketSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterRocketSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 14.2f)]
    .End();
}

ShooterRocketSystem::ShooterRocketSystem(Scene* scene)
    : DAVA::BaseSimulationSystem(scene, ComponentUtils::MakeMask<ShooterRocketComponent>())
{
    entitiesComp = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>();
}

ShooterRocketSystem::~ShooterRocketSystem()
{
}

void ShooterRocketSystem::AddEntity(Entity* entity)
{
    pendingEntities.insert(entity);
    if (IsSimulated(entity))
    {
        BaseSimulationSystem::AddEntity(entity);
    }
}

void ShooterRocketSystem::RemoveEntity(Entity* entity)
{
    pendingEntities.erase(entity);
    if (IsSimulated(entity))
    {
        BaseSimulationSystem::RemoveEntity(entity);
    }
}

void ShooterRocketSystem::Simulate(Entity* rocket)
{
    CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingletonComponent<CollisionSingleComponent>();
    ShooterRocketComponent* rocketComp = rocket->GetComponent<ShooterRocketComponent>();
    const uint32 distance = rocketComp->GetDistance();
    if (destroyedEntities.find(rocket) != destroyedEntities.end())
    {
        return;
    }

    if (distance > ShooterRocketComponent::MAX_DISTANCE ||
        !collisionSingleComponent->GetCollisionsWithEntity(rocket).empty())
    {
        destroyedEntities.insert(rocket);
        return;
    }

    Entity* shooter = entitiesComp->FindByID(rocketComp->shooterId);
    DVASSERT(shooter);

    if ((rocketComp->GetStage() == ShooterRocketComponent::Stage::DESTROYER && distance > 5) ||
        (rocketComp->GetStage() == ShooterRocketComponent::Stage::BOOSTER && distance > 10))
    {
        BoxShapeComponent* boxShape = rocket->GetComponent<BoxShapeComponent>();
        if (boxShape)
        {
            boxShape->SetTypeMask(SHOOTER_PROJECTILE_COLLISION_TYPE);
            uint32 collideWith = static_cast<uint32>(~0) & ~SHOOTER_PROJECTILE_COLLISION_TYPE;
            boxShape->SetTypeMaskToCollideWith(collideWith);
        }
    };

    TransformComponent* transComp = rocket->GetComponent<TransformComponent>();
    Vector3 position = transComp->GetPosition();
    Quaternion rotation = transComp->GetRotation();

    if (rocketComp->GetStage() == ShooterRocketComponent::Stage::BOOSTER &&
        distance == ShooterRocketComponent::SPLIT_DISTANCE)
    {
        destroyedEntities.insert(rocket);
        const Entity* target = GetTarget(rocket, shooter);
        if (!target)
        {
            return;
        }

        const NetworkPlayerID playerId = rocket->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID();
        const uint32 frameId = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>()->GetFrameId();

        uint32 subRocketCount = 3;
        if (HAS_MISPREDICTION && IsServer(this))
        {
            subRocketCount = (frameId % (subRocketCount + 3)) + 1;
        }

        for (uint32 subRocketIdx = 0; subRocketIdx < subRocketCount; ++subRocketIdx)
        {
            FrameActionID shootActionId(frameId, playerId, subRocketIdx);
            NetworkID entityId = NetworkIdSystem::GetEntityIdFromAction(shootActionId);
            Entity* subRocket = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>()->FindByID(entityId);
            if (!subRocket)
            {
                subRocket = SpawnSubRocket(shooter, target, shootActionId);
                TransformComponent* subTrunsComp = subRocket->GetComponent<TransformComponent>();
                float32 angle = (subRocketIdx - (subRocketCount / 2.f)) * DEG_TO_RAD * 45.f;
                Quaternion subRotation = rotation * Quaternion::MakeRotation(Vector3::UnitZ, -angle);
                subTrunsComp->SetLocalTransform(position, subRotation, Vector3(1.f, 1.0f, 1.0f));
                GetScene()->AddNode(subRocket);
            }
        }

        return;
    }

    const float32 timeElapsed = NetworkTimeSingleComponent::FrameDurationS;
    if (rocketComp->GetStage() == ShooterRocketComponent::Stage::DESTROYER)
    {
        const Entity* target = entitiesComp->FindByID(rocketComp->targetId);
        DVASSERT(target);
        const Vector3& targetFeet = target->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslationVector();
        const float32 targetHeight = target->GetComponent<CapsuleCharacterControllerComponent>()->GetHeight();
        const Vector3 targetPos = targetFeet + Vector3(0.f, 0.f, targetHeight);
        const Vector3& currentPos = rocket->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslationVector();

        Vector3 targetDir = targetPos - currentPos;
        targetDir.Normalize();
        Vector3 forward = rotation.ApplyToVectorFast(Vector3(0.f, 1.f, 0.f));
        Vector3 axis = targetDir.CrossProduct(forward);
        float angle = ShooterRocketComponent::ROT_SPEED * timeElapsed;
        if (axis.Length() < 0.0000001f)
        {
            axis = Vector3(0.f, 0.f, 1.f);
            if (forward.DotProduct(targetDir) > 0)
            {
                angle = 0;
            }
        }
        else
        {
            axis.Normalize();
        }

        rotation = Quaternion::MakeRotation(axis, -angle) * rotation;
    }

    position += rotation.ApplyToVectorFast(Vector3(0, ShooterRocketComponent::MOVE_SPEED, 0) * timeElapsed);
    transComp->SetLocalTransform(position, rotation, Vector3(1.0, 1.0, 1.0));
    rocketComp->SetDistance(distance + 1);
}

void ShooterRocketSystem::ProcessFixed(float32 timeElapsed)
{
    for (Entity* rocket : pendingEntities)
    {
        FillRocket(rocket);
    }
    pendingEntities.clear();

    Vector<Entity*> rockets(entities);
    for (Entity* rocket : rockets)
    {
        Simulate(rocket);
    }

    for (auto destroyedBullet : destroyedEntities)
    {
        GetScene()->RemoveNode(destroyedBullet);
    }
}

Entity* ShooterRocketSystem::GetRocketModel()
{
    if (nullptr == rocketModel)
    {
        ScopedPtr<Scene> model(new Scene());
        SceneFileV2::eError err = model->LoadScene("~res:/Rocket.sc2");
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        rocketModel = model->GetEntityByID(1)->Clone();
        rocketModel->SetName("RocketModel");
    }

    return rocketModel->Clone();
}

bool ShooterRocketSystem::IsSimulated(Entity* rocket)
{
    return (IsServer(this) || rocket->GetComponent<NetworkPredictComponent>());
}

void ShooterRocketSystem::FillRocket(Entity* rocket)
{
    ShooterRocketComponent* rocketComp = rocket->GetComponent<ShooterRocketComponent>();
    Entity* rocketModel = GetRocketModel();
    rocket->AddNode(rocketModel);
    rocketModel->Release();

    Color color;
    switch (rocketComp->GetStage())
    {
    case ShooterRocketComponent::Stage::BOOSTER:
    {
        rocket->SetName("Rocket");
        color = Color::Yellow;
        break;
    }
    case ShooterRocketComponent::Stage::DESTROYER:
    {
        rocket->SetName("SubRocket");
        color = Color::Red;
        break;
    }
    }

    BoxShapeComponent* boxShape = new BoxShapeComponent();
    const AABBox3 bbox = rocketModel->GetWTMaximumBoundingBoxSlow();
    boxShape->SetHalfSize(bbox.GetSize() / 2.0);
    boxShape->SetOverrideMass(true);
    boxShape->SetMass(0.0001f);
    rocket->AddComponent(boxShape);

    DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
    dynamicBody->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
    rocket->AddComponent(dynamicBody);
    rocket->AddComponent(new DamageComponent());

    if (!IsServer(this))
    {
        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = rocketModel->GetWTMaximumBoundingBoxSlow();
        rocket->AddComponent(debugDrawComponent);
        Colorize(rocketModel, color);
    }

    if (HAS_MISPREDICTION && IsClientOwner(rocket))
    {
        Colorize(rocket);
    }
}

const Entity* ShooterRocketSystem::GetTarget(Entity* rocket, Entity* shooter)
{
    NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>();
    TransformComponent* transComp = rocket->GetComponent<TransformComponent>();
    const Vector3& position = transComp->GetPosition();
    const NetworkPlayerID playerId = rocket->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID();
    const NetworkPlayerComponent* playerComp = shooter->GetComponent<NetworkPlayerComponent>();
    const FixedVector<NetworkID>& visibleEntityIds = playerComp->visibleEntityIds;
    const Entity* target = nullptr;
    float32 minDist = 0.f;
    for (size_t i = 0; i < visibleEntityIds.size(); ++i)
    {
        const Entity* entity = networkEntities->FindByID(visibleEntityIds[i]);
        if (!entity)
        {
            continue;
        }

        const HealthComponent* healthComp = entity->GetComponent<HealthComponent>();
        if (!healthComp || healthComp->GetHealth() == 0)
        {
            continue;
        }

        if (SELF_DAMAGE || playerId != entity->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID())
        {
            const float32 dist = Distance(position, entity->GetComponent<TransformComponent>()->GetPosition());
            if (target == nullptr || dist < minDist)
            {
                minDist = dist;
                target = entity;
            }
        }
    }

    return target;
}

Entity* ShooterRocketSystem::SpawnSubRocket(Entity* shooter, const Entity* target, const FrameActionID& shootActionId)
{
    Entity* subRocket = new Entity;
    ShooterRocketComponent* subRocketComponent = new ShooterRocketComponent();
    subRocketComponent->SetStage(ShooterRocketComponent::Stage::DESTROYER);
    NetworkReplicationComponent* shooterReplicationComp = shooter->GetComponent<NetworkReplicationComponent>();
    subRocketComponent->shooterId = shooterReplicationComp->GetNetworkID();
    NetworkReplicationComponent* targetReplicationComp = target->GetComponent<NetworkReplicationComponent>();
    subRocketComponent->targetId = targetReplicationComp->GetNetworkID();
    subRocket->AddComponent(subRocketComponent);
    NetworkReplicationComponent* replComp = new NetworkReplicationComponent();
    const NetworkPlayerID playerId = shooterReplicationComp->GetNetworkPlayerID();
    replComp->SetNetworkPlayerID(playerId);
    subRocket->AddComponent(replComp);
    NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent();
    networkPredictComponent->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
    networkPredictComponent->AddPredictedComponent(Type::Instance<ShooterRocketComponent>());
    networkPredictComponent->SetFrameActionID(shootActionId);
    subRocket->AddComponent(networkPredictComponent);
    subRocket->AddComponent(new NetworkTransformComponent());
    return subRocket;
}

void ShooterRocketSystem::Colorize(DAVA::Entity* rocket)
{
    Color color;
    if (IsSimulated(rocket))
    {
        color = Color::Green;
    }
    else
    {
        color = Color::Red;
    }

    Vector<Entity*> models;
    rocket->GetChildEntitiesWithCondition(models, [](Entity* e) { return e->GetName() == FastName("RocketModel"); });
    for (Entity* model : models)
    {
        Colorize(model, color);
    }
}

void ShooterRocketSystem::Colorize(DAVA::Entity* model, const Color& color)
{
    RenderObject* ro = GetRenderObject(model);
    for (uint32 batchIdx = 0; batchIdx < ro->GetRenderBatchCount(); ++batchIdx)
    {
        NMaterial* mat = ro->GetRenderBatch(batchIdx)->GetMaterial();
        mat->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, 1);
        mat->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, color.color, rhi::ShaderProp::TYPE_FLOAT4);
    }
}
