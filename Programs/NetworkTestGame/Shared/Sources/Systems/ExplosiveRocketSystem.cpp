#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "ExplosiveRocketSystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"

#include "Utils/Random.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Snapshot.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include "Components/GameStunningComponent.h"
#include "Components/DamageComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ExplosiveRocketComponent.h"

#include <Physics/PhysicsSystem.h>
#include <Physics/CollisionSingleComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/BoxShapeComponent.h>

#include <Render/Highlevel/RenderObject.h>

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ExplosiveRocketSystem)
{
    ReflectionRegistrator<ExplosiveRocketSystem>::Begin()[M::Tags("shoot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ExplosiveRocketSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 17.0f)]
    .End();
}

ExplosiveRocketSystem::ExplosiveRocketSystem(Scene* scene)
    : DAVA::BaseSimulationSystem(scene, ComponentUtils::MakeMask<ExplosiveRocketComponent>())
{
    entitiesComp = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>();
}

ExplosiveRocketSystem::~ExplosiveRocketSystem()
{
}

void ExplosiveRocketSystem::AddEntity(Entity* entity)
{
    pendingEntities.insert(entity);
    if (IsSimulated(entity))
    {
        BaseSimulationSystem::AddEntity(entity);
    }
}

void ExplosiveRocketSystem::RemoveEntity(Entity* entity)
{
    pendingEntities.erase(entity);
    if (IsSimulated(entity))
    {
        BaseSimulationSystem::RemoveEntity(entity);
    }
}

void ExplosiveRocketSystem::Simulate(Entity* rocket)
{
    CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingletonComponent<CollisionSingleComponent>();
    ExplosiveRocketComponent* rocketComp = rocket->GetComponent<ExplosiveRocketComponent>();
    const uint32 distance = rocketComp->GetDistance();
    if (destroyedEntities.find(rocket) != destroyedEntities.end())
    {
        return;
    }

    if (distance > ExplosiveRocketComponent::MAX_DISTANCE ||
        !collisionSingleComponent->GetCollisionsWithEntity(rocket).empty())
    {
        destroyedEntities.insert(rocket);
        return;
    }

    Entity* shooter = entitiesComp->FindByID(rocketComp->shooterId);
    DVASSERT(shooter);
    if (!shooter)
    {
        return;
    }

    if ((rocketComp->GetStage() == ExplosiveRocketComponent::Stage::DESTROYER && distance > 1) ||
        (rocketComp->GetStage() == ExplosiveRocketComponent::Stage::BOOSTER && distance > 8))
    {
        BoxShapeComponent* boxShape = rocket->GetComponent<BoxShapeComponent>();
        if (boxShape)
        {
            boxShape->SetTypeMask(2);
            boxShape->SetTypeMaskToCollideWith(1);
        }
    };

    TransformComponent* transComp = rocket->GetComponent<TransformComponent>();
    Vector3 translation = transComp->GetPosition();
    Quaternion rotation = transComp->GetRotation();

    if (rocketComp->GetStage() == ExplosiveRocketComponent::Stage::BOOSTER &&
        distance == ExplosiveRocketComponent::SPLIT_DISTANCE)
    {
        destroyedEntities.insert(rocket);
        const Entity* target = GetTarget(rocket, shooter);
        DVASSERT(!SELF_DAMAGE || target);
        if (!target)
        {
            return;
        }
        DVASSERT(!SELF_DAMAGE || target);

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
            Entity* subRocket = entitiesComp->FindByID(entityId);

            if (!subRocket)
            {
                subRocket = SpawnSubRocket(shooter, target, shootActionId);
                TransformComponent* subTrunsComp = subRocket->GetComponent<TransformComponent>();
                float32 angle = (subRocketIdx - (subRocketCount / 2.f)) * DEG_TO_RAD * 15.f;
                Quaternion subRotation = rotation * Quaternion::MakeRotation(Vector3::UnitZ, -angle);
                subTrunsComp->SetLocalTransform(translation, subRotation, Vector3(1.f, 1.f, 1.f));
                GetScene()->AddNode(subRocket);
            }
        }

        return;
    }

    const float32 timeElapsed = NetworkTimeSingleComponent::FrameDurationS;
    if (rocketComp->GetStage() == ExplosiveRocketComponent::Stage::DESTROYER)
    {
        Entity* target = entitiesComp->FindByID(rocketComp->targetId);
        DVASSERT(target);
        const Vector3& targetPos = target->GetComponent<TransformComponent>()->GetPosition();
        Vector3 delta = targetPos - translation;
        delta.Normalize();

        Vector3 forward = rotation.ApplyToVectorFast(Vector3(0.f, 1.f, 0.f));
        Vector3 side = rotation.ApplyToVectorFast(Vector3(1.f, 0.f, 0.f));

        float32 angle = atan2f(side.DotProduct(delta), forward.DotProduct(delta));
        if (fabsf(angle) > 0.1)
        {
            const int8 direct = static_cast<const int8>((angle < 0.f) ? -1 : 1);
            const float32 moveAngle = ExplosiveRocketComponent::ROT_SPEED * timeElapsed * direct;
            rotation *= Quaternion::MakeRotation(Vector3::UnitZ, -moveAngle);
        }
    }

    translation += rotation.ApplyToVectorFast(Vector3(0, ExplosiveRocketComponent::MOVE_SPEED, 0) * timeElapsed);
    transComp->SetLocalTransform(translation, rotation, Vector3(1.0, 1.0, 1.0));
    rocketComp->SetDistance(distance + 1);
}

void ExplosiveRocketSystem::ProcessFixed(float32 timeElapsed)
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

    destroyedEntities.clear();
}

Entity* ExplosiveRocketSystem::GetRocketModel() const
{
    if (nullptr == rocketModel)
    {
        ScopedPtr<Scene> model(new Scene());
        SceneFileV2::eError err = model->LoadScene("~res:/Sniper_2.sc2");
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        rocketModel = model->GetEntityByID(1)->GetChild(1)->Clone();
        rocketModel->SetName("RocketModel");
    }

    return rocketModel->Clone();
}

bool ExplosiveRocketSystem::IsSimulated(Entity* rocket)
{
    return (IsServer(this) || rocket->GetComponent<NetworkPredictComponent>());
}

void ExplosiveRocketSystem::FillRocket(Entity* rocket)
{
    ExplosiveRocketComponent* rocketComp = rocket->GetComponent<ExplosiveRocketComponent>();
    Entity* rocketModel = GetRocketModel();
    rocket->AddNode(rocketModel);
    rocketModel->Release();
    rocket->SetName("Bullet");

    const Entity* shooter = entitiesComp->FindByID(rocketComp->shooterId);
    if (shooter && rocketComp->GetStage() == ExplosiveRocketComponent::Stage::BOOSTER)
    {
        const TransformComponent* srcTransComp = shooter->GetComponent<TransformComponent>();
        const Quaternion& rotation = srcTransComp->GetRotation();
        const Vector3& translation = srcTransComp->GetPosition();
        rocket->GetComponent<TransformComponent>()->SetLocalTransform(translation,
                                                                      rotation,
                                                                      srcTransComp->GetScale());
    }

    if (IsServer(this))
    {
        rocket->AddComponent(new DamageComponent());
        BoxShapeComponent* boxShape = new BoxShapeComponent();
        const AABBox3 bbox = rocketModel->GetWTMaximumBoundingBoxSlow();
        boxShape->SetHalfSize(bbox.GetSize() / 2.0);
        boxShape->SetOverrideMass(true);
        boxShape->SetMass(0.0001f);
        rocket->AddComponent(boxShape);

        DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
        dynamicBody->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
        rocket->AddComponent(dynamicBody);
    }
    else
    {
        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = rocketModel->GetWTMaximumBoundingBoxSlow();
        rocket->AddComponent(debugDrawComponent);
    }

    if (HAS_MISPREDICTION && IsClientOwner(rocket))
    {
        Colorize(rocket);
    }
}

const Entity* ExplosiveRocketSystem::GetTarget(Entity* rocket, Entity* shooter)
{
    ExplosiveRocketComponent* rocketComp = rocket->GetComponent<ExplosiveRocketComponent>();
    NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>();
    TransformComponent* transComp = rocket->GetComponent<TransformComponent>();
    const Vector3& translation = transComp->GetPosition();
    const NetworkPlayerID playerId = rocket->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID();
    DVASSERT(shooter);
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
            const float32 dist = Distance(translation, entity->GetComponent<TransformComponent>()->GetPosition());
            if (target == nullptr || dist < minDist)
            {
                minDist = dist;
                target = entity;
            }
        }
    }

    return target;
}

Entity* ExplosiveRocketSystem::SpawnSubRocket(Entity* shooter, const Entity* target, const FrameActionID& shootActionId)
{
    Entity* subRocket = new Entity;
    ExplosiveRocketComponent* subRocketComponent = new ExplosiveRocketComponent();
    subRocketComponent->SetStage(ExplosiveRocketComponent::Stage::DESTROYER);
    NetworkReplicationComponent* shooterReplicationComp = shooter->GetComponent<NetworkReplicationComponent>();
    subRocketComponent->shooterId = shooterReplicationComp->GetNetworkID();
    subRocketComponent->targetId = target->GetComponent<NetworkReplicationComponent>()->GetNetworkID();
    subRocket->AddComponent(subRocketComponent);

    NetworkReplicationComponent* replComp = new NetworkReplicationComponent();
    replComp->SetNetworkPlayerID(shooterReplicationComp->GetNetworkPlayerID());
    subRocket->AddComponent(replComp);
    NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent();
    networkPredictComponent->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
    networkPredictComponent->AddPredictedComponent(Type::Instance<ExplosiveRocketComponent>());
    networkPredictComponent->SetFrameActionID(shootActionId);
    subRocket->AddComponent(networkPredictComponent);
    subRocket->AddComponent(new NetworkTransformComponent());
    return subRocket;
}

void ExplosiveRocketSystem::Colorize(DAVA::Entity* rocket)
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
        RenderObject* ro = GetRenderObject(model);
        for (uint32 batchIdx = 0; batchIdx < ro->GetRenderBatchCount(); ++batchIdx)
        {
            NMaterial* mat = ro->GetRenderBatch(batchIdx)->GetMaterial();
            mat->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, 1);
            mat->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, color.color, rhi::ShaderProp::TYPE_FLOAT4);
        }
    }
}
