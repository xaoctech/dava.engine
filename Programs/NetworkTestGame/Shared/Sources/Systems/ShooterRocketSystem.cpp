#include "ShooterRocketSystem.h"
#include "ShooterConstants.h"

#include "Visibility/ObservableComponent.h"

#include <Base/BaseMath.h>
#include <Base/BaseObject.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Utils/Random.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkIdSystem.h>

#include "Components/DamageComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShooterRocketComponent.h"
#include "Components/SingleComponents/EffectQueueSingleComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/Controllers/CapsuleCharacterControllerComponent.h>
#include <Physics/CollisionSingleComponent.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/PhysicsSystem.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterRocketSystem)
{
    ReflectionRegistrator<ShooterRocketSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterRocketSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 14.2f)]
    .End();
}

ShooterRocketSystem::ShooterRocketSystem(Scene* scene)
    : DAVA::BaseSimulationSystem(scene, ComponentUtils::MakeMask<ShooterRocketComponent>())
    , entityGroup(scene->AquireEntityGroup<ShooterRocketComponent>())
    , pendingEntities(scene->AquireEntityGroupOnAdd(entityGroup, this))
{
    entitiesComp = scene->GetSingleComponent<NetworkEntitiesSingleComponent>();
    effectQueue = scene->GetSingleComponent<EffectQueueSingleComponent>();
}

ShooterRocketSystem::~ShooterRocketSystem()
{
}

void ShooterRocketSystem::SimulateRocket(Entity* rocket)
{
    const CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingleComponentForRead<CollisionSingleComponent>(this);
    ShooterRocketComponent* rocketComp = rocket->GetComponent<ShooterRocketComponent>();
    const uint32 distance = rocketComp->GetDistance();
    if (destroyedEntities.find(rocket) != destroyedEntities.end())
    {
        return;
    }

    Vector<CollisionInfo> collisions = collisionSingleComponent->GetCollisionsWithEntity(rocket);
    if (distance > ShooterRocketComponent::MAX_DISTANCE || !collisions.empty())
    {
        Vector3 explosionPosition;
        if (!collisions.empty())
        {
            const CollisionPoint& cp = collisions[0].points[0];
            explosionPosition = cp.position;
        }
        else
        {
            TransformComponent* transComp = rocket->GetComponent<TransformComponent>();
            Vector3 position = transComp->GetPosition();
            explosionPosition = transComp->GetPosition();
        }

        // effect: rocket explosion
        effectQueue->CreateEffect(0)
        .SetDuration(5.f)
        .SetPosition(explosionPosition);

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
        // effect: rocket split
        effectQueue->CreateEffect(1)
        .SetDuration(5.f)
        .SetPosition(position);

        destroyedEntities.insert(rocket);
        const Entity* target = GetTarget(rocket, shooter);
        if (!target)
        {
            return;
        }

        const NetworkPlayerID playerId = rocket->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID();
        const uint32 frameId = GetScene()->GetSingleComponent<NetworkTimeSingleComponent>()->GetFrameId();

        uint32 subRocketCount = 3;
        if (HAS_MISPREDICTION && IsServer(this))
        {
            subRocketCount = (frameId % (subRocketCount + 3)) + 1;
        }

        for (uint32 subRocketIdx = 0; subRocketIdx < subRocketCount; ++subRocketIdx)
        {
            NetworkID rocketId = NetworkID::CreatePlayerActionId(playerId, frameId, subRocketIdx);
            Entity* subRocket = GetScene()->GetSingleComponent<NetworkEntitiesSingleComponent>()->FindByID(rocketId);

            if (!subRocket)
            {
                subRocket = SpawnSubRocket(shooter, target, rocketId);

                // effect: subrocket smoke track
                NetworkID trackEffectId = NetworkID::CreatePlayerActionId(playerId, frameId, subRocketIdx + subRocketCount);
                effectQueue->CreateEffect(2)
                .SetNetworkId(trackEffectId)
                .SetParentId(rocketId);

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

void ShooterRocketSystem::SimulateRocket2(DAVA::Entity* rocket)
{
    const CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingleComponentForRead<CollisionSingleComponent>(this);

    TransformComponent* transComp = rocket->GetComponent<TransformComponent>();
    ShooterRocketComponent* rocketComp = rocket->GetComponent<ShooterRocketComponent>();
    Vector3 position = transComp->GetPosition();
    Quaternion rotation = transComp->GetRotation();

    bool createExplosionAndReturn = false;
    Vector3 explosionPosition;

    Vector<CollisionInfo> collisions = collisionSingleComponent->GetCollisionsWithEntity(rocket);
    if (!collisions.empty())
    {
        const CollisionPoint& cp = collisions[0].points[0];
        createExplosionAndReturn = true;
        explosionPosition = cp.position;
    }

    const uint32 distance = rocketComp->GetDistance();
    if (distance > ShooterRocketComponent::MAX_DISTANCE)
    {
        createExplosionAndReturn = true;
        explosionPosition = position;
    }

    if (createExplosionAndReturn)
    {
        // effect: rocket explosion
        effectQueue->CreateEffect(0)
        .SetDuration(5.f)
        .SetPosition(explosionPosition);
        destroyedEntities.insert(rocket);
        return;
    }

    BoxShapeComponent* boxShape = rocket->GetComponent<BoxShapeComponent>();
    if (boxShape)
    {
        boxShape->SetTypeMask(SHOOTER_PROJECTILE_COLLISION_TYPE);
        uint32 collideWith = static_cast<uint32>(~0) & ~SHOOTER_PROJECTILE_COLLISION_TYPE;
        boxShape->SetTypeMaskToCollideWith(collideWith);
    }

    const float32 timeElapsed = NetworkTimeSingleComponent::FrameDurationS;

    position += rotation.ApplyToVectorFast(Vector3(0, ShooterRocketComponent::MOVE_SPEED, 0) * timeElapsed);

    transComp->SetLocalTransform(position, rotation, Vector3(1.0, 1.0, 1.0));
    rocketComp->SetDistance(distance + 1);
}

void ShooterRocketSystem::ProcessFixed(float32 timeElapsed)
{
    DVASSERT(pendingEntities->entities.size() < 100);

    for (Entity* rocket : Vector<Entity*>(pendingEntities->entities))
    {
        FillRocket(rocket);
    }
    pendingEntities->entities.clear();

    for (Entity* rocket : entityGroup->GetEntities())
    {
        if (IsClient(this) && !IsClientOwner(rocket))
        {
            continue;
        }

        ShooterRocketComponent* rocketComponent = rocket->GetComponent<ShooterRocketComponent>();
        if (rocketComponent->multirocket)
            SimulateRocket(rocket);
        else
            SimulateRocket2(rocket);
    }

    for (Entity* destroyedBullet : destroyedEntities)
    {
        GetScene()->RemoveNode(destroyedBullet);
    }

    destroyedEntities.clear();
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

    if (IsServer(this))
    {
        rocket->AddComponent(new ObservableComponent());
        rocket->AddComponent(new SimpleVisibilityShapeComponent());
    }
    else
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
    NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingleComponent<NetworkEntitiesSingleComponent>();
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

Entity* ShooterRocketSystem::SpawnSubRocket(Entity* shooter, const Entity* target, NetworkID rocketId)
{
    Entity* subRocket = new Entity;

    ShooterRocketComponent* subRocketComponent = new ShooterRocketComponent();
    subRocketComponent->SetStage(ShooterRocketComponent::Stage::DESTROYER);
    NetworkReplicationComponent* shooterReplicationComp = shooter->GetComponent<NetworkReplicationComponent>();
    subRocketComponent->shooterId = shooterReplicationComp->GetNetworkID();
    NetworkReplicationComponent* targetReplicationComp = target->GetComponent<NetworkReplicationComponent>();
    subRocketComponent->targetId = targetReplicationComp->GetNetworkID();
    subRocket->AddComponent(subRocketComponent);

    NetworkTransformComponent* transfComp = new NetworkTransformComponent();
    subRocket->AddComponent(transfComp);

    NetworkReplicationComponent* replComp = new NetworkReplicationComponent(rocketId);
    replComp->SetForReplication<ShooterRocketComponent>(M::Privacy::PUBLIC);
    replComp->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
    subRocket->AddComponent(replComp);

    ComponentMask predictionMask;
    predictionMask.Set<NetworkTransformComponent>();
    predictionMask.Set<ShooterRocketComponent>();
    NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictionMask);
    subRocket->AddComponent(networkPredictComponent);

    return subRocket;
}

void ShooterRocketSystem::Colorize(Entity* rocket)
{
    Color color;
    if (IsServer(this) || IsClientOwner(rocket))
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

void ShooterRocketSystem::Colorize(Entity* model, const Color& color)
{
    RenderObject* ro = GetRenderObject(model);
    for (uint32 batchIdx = 0; batchIdx < ro->GetRenderBatchCount(); ++batchIdx)
    {
        NMaterial* mat = ro->GetRenderBatch(batchIdx)->GetMaterial();
        mat->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, 1);
        mat->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, color.color, rhi::ShaderProp::TYPE_FLOAT4);
    }
}
