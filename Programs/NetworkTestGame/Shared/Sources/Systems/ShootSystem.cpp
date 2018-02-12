#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "ShootSystem.h"
#include "Components/ShootComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/TransformInterpolationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "Utils/Random.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h"
#include "NetworkCore/Snapshot.h"

#include "Components/GameStunningComponent.h"
#include "Components/DamageComponent.h"

#include <Physics/PhysicsSystem.h>
#include <Physics/CollisionSingleComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/BoxShapeComponent.h>


#include "NetworkCore/NetworkCoreUtils.h"

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ShootSystem)
{
    ReflectionRegistrator<ShootSystem>::Begin()[M::Tags("shoot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShootSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 8.0f)]
    .End();
}

namespace ShootSystemDetail
{
template <typename T>
static bool CompareTransform(const T& lhs, const T& rhs, uint32 size, float32 epsilon, uint32 frameId)
{
    for (uint32 i = 0; i < size; ++i)
    {
        if (!FLOAT_EQUAL_EPS(lhs.data[i], rhs.data[i], epsilon))
        {
            Logger::Debug("Transforms aren't equal (compared by shoot system), diff: %f, index: %d, frame: %d", std::abs(lhs.data[i] - rhs.data[i]), i, frameId);

            return false;
        }
    }
    return true;
}
}

ShootSystem::ShootSystem(Scene* scene)
    : DAVA::BaseSimulationSystem(scene, ComponentUtils::MakeMask<ShootComponent>())
{
}

ShootSystem::~ShootSystem()
{
}

void ShootSystem::AddEntity(Entity* entity)
{
    pendingEntities.insert(entity);
    if (IsServer(this) || IsClientOwner(this, entity))
    {
        BaseSimulationSystem::AddEntity(entity);
    }
}

void ShootSystem::RemoveEntity(Entity* entity)
{
    pendingEntities.erase(entity);
    if (IsServer(this) || IsClientOwner(this, entity))
    {
        BaseSimulationSystem::RemoveEntity(entity);
    }
}

void ShootSystem::NextState(Entity* bullet, ShootComponent* shootComponent, DAVA::float32 timeElapsed)
{
    CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingletonComponent<CollisionSingleComponent>();

    switch (shootComponent->GetPhase())
    {
    case ShootPhase::BURN:
    {
        if (shootComponent->GetDistance() > 8.0)
        {
            shootComponent->SetPhase(ShootPhase::FLY);
            BoxShapeComponent* boxShape = bullet->GetComponent<BoxShapeComponent>();
            if (boxShape)
            {
                boxShape->SetTypeMask(2);
                boxShape->SetTypeMaskToCollideWith(1);
            }
        }
        break;
    }

    case ShootPhase::FLY:
    {
        if (shootComponent->GetDistance() > ShootComponent::MAX_DISTANCE)
        {
            shootComponent->SetPhase(ShootPhase::DESTROY);
        }
        else if (!collisionSingleComponent->GetCollisionsWithEntity(bullet).empty())
        {
            shootComponent->SetPhase(ShootPhase::DESTROY);
        }
        break;
    }

    case ShootPhase::DESTROY:
        return;
    }

    switch (shootComponent->GetPhase())
    {
    case ShootPhase::BURN:
    case ShootPhase::FLY:
    {
        Matrix4 locTrans = bullet->GetLocalTransform();
        Vector3 moveVector(0.f, ShootComponent::MOVE_SPEED * timeElapsed, 0.f);
        bullet->SetLocalTransform(Matrix4::MakeTranslation(moveVector) * locTrans);
        shootComponent->SetDistance(shootComponent->GetDistance() + 1);
        return;
    }

    case ShootPhase::DESTROY:
        return;
    }
}

void ShootSystem::Simulate(Entity* bullet)
{
    ShootComponent* shootComponent = bullet->GetComponent<ShootComponent>();
    NextState(bullet, shootComponent, NetworkTimeSingleComponent::FrameDurationS);
}

void ShootSystem::ProcessFixed(float32 timeElapsed)
{
    for (Entity* bullet : pendingEntities)
    {
        ShootComponent* shootComponent = bullet->GetComponent<ShootComponent>();
        Entity* bulletModel = GetBulletModel();
        bullet->AddNode(bulletModel);
        bulletModel->Release();
        bullet->SetName("Bullet");

        const Entity* shooter = shootComponent->GetShooter();
        if (shooter)
        {
            const TransformComponent* srcTransComp = shooter->GetComponent<TransformComponent>();
            TransformComponent* transComp = bullet->GetComponent<TransformComponent>();
            Quaternion rotation = srcTransComp->GetRotation();
            Vector3 translation = srcTransComp->GetPosition();

            // translation += rotation.ApplyToVectorFast(Vector3(0.f, 5.f, 0.f));
            transComp->SetLocalTransform(translation,
                                         rotation,
                                         srcTransComp->GetScale());
        }

        if (IsServer(GetScene()))
        {
            if (shootComponent->GetShootType() & ShootComponent::ShootType::MAIN)
            {
                bullet->AddComponent(new DamageComponent());
            }
            if (shootComponent->GetShootType() & ShootComponent::ShootType::STUN)
            {
                bullet->AddComponent(new GameStunningComponent());
            }

            BoxShapeComponent* boxShape = new BoxShapeComponent();
            const AABBox3 bbox = bulletModel->GetWTMaximumBoundingBoxSlow();
            boxShape->SetHalfSize(bbox.GetSize() / 2.0);
            boxShape->SetOverrideMass(true);
            boxShape->SetMass(0.0001f);
            bullet->AddComponent(boxShape);

            DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
            dynamicBody->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
            bullet->AddComponent(dynamicBody);
        }
        else
        {
            NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
            debugDrawComponent->box = bulletModel->GetWTMaximumBoundingBoxSlow();
            bullet->AddComponent(debugDrawComponent);

            TransformInterpolationComponent* tic = new TransformInterpolationComponent();
            tic->time = 1.0f;
            tic->ApplyImmediately();
            bullet->AddComponent(tic);
        }
    }

    pendingEntities.clear();

    Vector<Entity*> destroyedBullets;
    for (const auto& bullet : entities)
    {
        ShootComponent* shootComponent = bullet->GetComponent<ShootComponent>();
        NextState(bullet, shootComponent, timeElapsed);

        if (shootComponent->GetPhase() == ShootPhase::DESTROY)
        {
            destroyedBullets.push_back(bullet);
        }
    }

    for (auto destroyedBullet : destroyedBullets)
    {
        GetScene()->RemoveNode(destroyedBullet);
        // SERVER_COMPLETE
        //SafeRelease(destroyedBullet);
    }
}

Entity* ShootSystem::GetBulletModel() const
{
    if (nullptr == bulletModel)
    {
        ScopedPtr<Scene> model(new Scene());
        SceneFileV2::eError err = model->LoadScene("~res:/Sniper_2.sc2");
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        bulletModel = model->GetEntityByID(1)->GetChild(1)->Clone();
    }

    return bulletModel->Clone();
}
