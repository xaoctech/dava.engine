#include "ShooterUtils.h"
#include "ShooterConstants.h"
#include "Components/ShooterRoleComponent.h"

#include <Base/UnordererMap.h>
#include <Scene3D/Scene.h>
#include <Entity/Component.h>
#include <Utils/Random.h>
#include <Render/RenderHelper.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Scene3D/Components/RenderComponent.h>

#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/SnapshotUtils.h>
#include <NetworkPhysics/NetworkPhysicsUtils.h>
#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/PhysicsUtils.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/CollisionShapeComponent.h>
#include <Physics/Private/PhysicsMath.h>

#include <physx/PxRigidActor.h>

QueryFilterCallback::QueryFilterCallback(DAVA::Entity const* source_, RaycastFilter filter_)
    : source(source_)
    , filter(filter_)
{
}

physx::PxQueryHitType::Enum QueryFilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
{
    using namespace DAVA;

    Component* component = static_cast<Component*>(actor->userData);
    DVASSERT(component != nullptr);

    Entity* entity = component->GetEntity();
    DVASSERT(entity != nullptr);

    bool block = true;

    block &= (entity->GetComponent<CapsuleCharacterControllerComponent>() == nullptr);

    if ((filter & RaycastFilter::IGNORE_SOURCE) == RaycastFilter::IGNORE_SOURCE)
    {
        DVASSERT(source != nullptr);
        block &= (entity != source);
    }
    if ((filter & RaycastFilter::IGNORE_DYNAMICS) == RaycastFilter::IGNORE_DYNAMICS)
    {
        block &= (entity->GetComponent<DynamicBodyComponent>() == nullptr);
    }

    return block ? physx::PxQueryHitType::eBLOCK : physx::PxQueryHitType::eNONE;
}

physx::PxQueryHitType::Enum QueryFilterCallback::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
{
    // Should never be called, PxQueryFlag::ePOSTFILTER should be disabled
    DVASSERT(false);
    return physx::PxQueryHitType::eBLOCK;
}

bool GetRaycastHit(DAVA::Scene& scene, const DAVA::Vector3& origin, const DAVA::Vector3& direction, DAVA::float32 distance,
                   physx::PxQueryFilterCallback* filterCallback, physx::PxRaycastHit& outHit)
{
    using namespace DAVA;

    physx::PxQueryFilterData queryFilter;
    queryFilter.flags = physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

    PhysicsSystem* physics = scene.GetSystem<PhysicsSystem>();
    DVASSERT(physics != nullptr);

    physx::PxRaycastBuffer hitBuffer;
    bool collisionDetected = physics->Raycast(origin, direction, distance, hitBuffer, queryFilter, filterCallback);
    if (collisionDetected)
    {
        outHit = hitBuffer.block;
        return true;
    }

    return false;
}

void GetAimRay(ShooterAimComponent const& aimComponent, RaycastFilter filter, DAVA::Vector3& outAimRayOrigin, DAVA::Vector3& outAimRayDirection, DAVA::Vector3& outAimRayEnd, DAVA::Entity** outEntity, bool current)
{
    using namespace DAVA;

    // Calculate aim offset in local space

    // Leave out z coordinate, will be added later when calculating world space positions
    Vector3 aimOffsetXY(SHOOTER_AIM_OFFSET.x, SHOOTER_AIM_OFFSET.y, 0.0f);

    float32 angleX = current ? aimComponent.GetCurrentAngleX() : aimComponent.GetFinalAngleX();
    float32 angleZ = current ? aimComponent.GetCurrentAngleZ() : aimComponent.GetFinalAngleZ();

    Matrix4 rotation = Matrix4::MakeRotation(SHOOTER_CHARACTER_RIGHT, angleX) * Matrix4::MakeRotation(Vector3::UnitZ, angleZ);
    Vector3 aimPositionRelative = aimOffsetXY * rotation;
    Vector3 aimDirection = SHOOTER_CHARACTER_FORWARD * rotation;

    Entity* aimingEntity = aimComponent.GetEntity();

    DVASSERT(aimingEntity != nullptr);

    TransformComponent* aimingEntityTransform = aimingEntity->GetComponent<TransformComponent>();
    DVASSERT(aimingEntityTransform != nullptr);

    // Calculate aim offset in world space

    const Vector3& characterPosition = aimingEntityTransform->GetPosition();
    Vector3 aimPositionAbsolute = characterPosition + aimPositionRelative + Vector3(0.0f, 0.0f, SHOOTER_AIM_OFFSET.z);

    // Check if something is between aim and the character. Move aim in this case so that the character is always visible to the camera
    Vector3 lookFromPoint = aimingEntityTransform->GetRotation().ApplyToVectorFast(SHOOTER_CHARACTER_LOOK_FROM) + characterPosition;
    Vector3 lookFromPointToAim = aimPositionAbsolute - lookFromPoint;
    float32 lookFromPointToAimLength = lookFromPointToAim.Length();
    lookFromPointToAim.Normalize();
    physx::PxRaycastHit hit;
    QueryFilterCallback filterCallback(aimingEntity, RaycastFilter::IGNORE_SOURCE | RaycastFilter::IGNORE_DYNAMICS);
    bool collision = GetRaycastHit(*aimingEntity->GetScene(), lookFromPoint, lookFromPointToAim, lookFromPointToAimLength, &filterCallback, hit);
    if (collision)
    {
        aimPositionAbsolute = PhysicsMath::PxVec3ToVector3(hit.position) - lookFromPointToAim * 0.15f;
    }

    Vector3 aimEndAbsolute = aimPositionAbsolute + aimDirection * SHOOTER_MAX_SHOOTING_DISTANCE;
    *outEntity = nullptr;

    // Find first obstacle the aim hits. Let aim ray end at that point
    filterCallback = QueryFilterCallback(aimingEntity, filter);
    collision = GetRaycastHit(*aimingEntity->GetScene(), aimPositionAbsolute, aimDirection, SHOOTER_MAX_SHOOTING_DISTANCE, &filterCallback, hit);
    if (collision)
    {
        Component* component = static_cast<Component*>(hit.actor->userData);
        DVASSERT(component != nullptr);

        aimEndAbsolute = PhysicsMath::PxVec3ToVector3(hit.position);
        *outEntity = component->GetEntity();
    }

    // Return values
    outAimRayOrigin = aimPositionAbsolute;
    outAimRayDirection = aimDirection;
    outAimRayEnd = aimEndAbsolute;
}

void GetCurrentAimRay(ShooterAimComponent const& aimComponent, RaycastFilter filter, DAVA::Vector3& outAimRayOrigin, DAVA::Vector3& outAimRayDirection, DAVA::Vector3& outAimRayEnd, DAVA::Entity** outEntity)
{
    GetAimRay(aimComponent, filter, outAimRayOrigin, outAimRayDirection, outAimRayEnd, outEntity, true);
}

void GetFinalAimRay(ShooterAimComponent const& aimComponent, RaycastFilter filter, DAVA::Vector3& outAimRayOrigin, DAVA::Vector3& outAimRayDirection, DAVA::Vector3& outAimRayEnd, DAVA::Entity** outEntity)
{
    GetAimRay(aimComponent, filter, outAimRayOrigin, outAimRayDirection, outAimRayEnd, outEntity, false);
}

DAVA::Vector3 GetRandomCarSpawnPosition()
{
    using namespace DAVA;

    float32 x = Random::Instance()->RandFloat32InBounds(40, 160);
    float32 y = Random::Instance()->RandFloat32InBounds(90, -40);

    return Vector3(x, y, 18.0f);
}

void UpdateStaticBodyTypes(DAVA::Entity* e)
{
    using namespace DAVA;

    StaticBodyComponent* staticBody = e->GetComponent<StaticBodyComponent>();
    if (staticBody != nullptr)
    {
        Vector<CollisionShapeComponent*> shapes = PhysicsUtils::GetShapeComponents(e);
        for (CollisionShapeComponent* shape : shapes)
        {
            shape->SetTypeMask(SHOOTER_STATIC_COLLISION_TYPE);
            shape->SetTypeMaskToCollideWith(UINT32_MAX);
        }
    }

    for (int32 i = 0; i < e->GetChildrenCount(); ++i)
    {
        UpdateStaticBodyTypes(e->GetChild(i));
    }
}

void InitializeScene(DAVA::Scene& scene)
{
    using namespace DAVA;

    const char* environmentScenePath = "~res:/3d/Maps/02_desert_train_dt/02_desert_train_dt.sc2";
    ScopedPtr<Scene> environmentScene(new Scene());
    SceneFileV2::eError ret = environmentScene->LoadScene(environmentScenePath);
    DVASSERT(ret == SceneFileV2::eError::ERROR_NO_ERROR);
    for (int32 i = 0; i < environmentScene->GetChildrenCount(); ++i)
    {
        scene.AddNode(environmentScene->GetChild(i));
    }

    // TODO: Should be made in editor
    UpdateStaticBodyTypes(&scene);

    if (IsServer(&scene))
    {
        for (uint32 i = 0; i < SHOOTER_NUM_CARS; ++i)
        {
            Entity* car = new Entity();
            TransformComponent* transformComponent = car->GetComponent<TransformComponent>();
            transformComponent->SetLocalTransform(GetRandomCarSpawnPosition(), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));

            NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
            replicationComponent->SetEntityType(EntityType::VEHICLE); // Workaround for GameVisbilitySystem to work properly
            car->AddComponent(replicationComponent);

            ShooterRoleComponent* roleComponent = new ShooterRoleComponent();
            roleComponent->SetRole(ShooterRoleComponent::Role::Car);
            car->AddComponent(roleComponent);

            scene.AddNode(car);
        }
    }
}

DAVA::Vector3 GetRandomPlayerSpawnPosition()
{
    using namespace DAVA;

    float32 x = Random::Instance()->RandFloat32InBounds(87, 138);
    float32 y = Random::Instance()->RandFloat32InBounds(47, -1);

    return Vector3(x, y, 18.0f);
}