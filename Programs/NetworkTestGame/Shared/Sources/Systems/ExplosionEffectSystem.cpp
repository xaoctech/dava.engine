#include "ExplosionEffectSystem.h"
#include "Components/ExplosionEffectComponent.h"
#include "Components/ShooterRocketComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include <Engine/Engine.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/ComponentGroup.h>
#include <Scene3D/EntityGroup.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/TransformComponent.h>

#include <NetworkCore/NetworkTypes.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>

#include <Physics/CollisionSingleComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ExplosionEffectSystem)
{
    ReflectionRegistrator<ExplosionEffectSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ExplosionEffectSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 14.1f)]
    .End();
}

ExplosionEffectSystem::ExplosionEffectSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, ComponentMask())
{
    if (IsServer(scene))
    {
        rocketEntities = scene->AquireEntityGroup<ShooterRocketComponent>();
    }
    explosionEffectComponents = scene->AquireComponentGroup<ExplosionEffectComponent, ExplosionEffectComponent>();
}

ExplosionEffectSystem::~ExplosionEffectSystem() = default;

void ExplosionEffectSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    Scene* scene = GetScene();
    if (IsServer(scene))
    {
        for (ExplosionEffectComponent* x : explosionEffectComponents->components)
        {
            x->duration -= timeElapsed;
            if (x->duration <= 0.f)
            {
                Entity* e = x->GetEntity();
                scene->RemoveNode(e);
            }
        }

        CollisionSingleComponent* collisionComponent = scene->GetSingleComponent<CollisionSingleComponent>();
        for (Entity* rocket : rocketEntities->GetEntities())
        {
            Vector<CollisionInfo> collisions = collisionComponent->GetCollisionsWithEntity(rocket);
            if (!collisions.empty())
            {
                const CollisionPoint& cp = collisions[0].points[0];

                static int n = 0;
                const int type = n & 1;
                Entity* explosion = nullptr;
                if (!Engine::Instance()->IsConsoleMode())
                {
                    explosion = CreateExplosionEffect(type);

                    ParticleEffectComponent* particle = explosion->GetComponent<ParticleEffectComponent>();
                    particle->Start();
                }
                else
                {
                    explosion = new Entity;
                }

                explosion->AddComponent(new ObservableComponent);
                explosion->AddComponent(new SimpleVisibilityShapeComponent);

                NetworkTransformComponent* ntc = new NetworkTransformComponent;
                explosion->AddComponent(ntc);

                NetworkReplicationComponent* r = new NetworkReplicationComponent(NetworkID::CreatePlayerOwnId(0));
                r->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
                r->SetForReplication<ExplosionEffectComponent>(M::Privacy::PUBLIC);
                explosion->AddComponent(r);

                static float32 duration = 5.f;
                ExplosionEffectComponent* effectComponent = new ExplosionEffectComponent;
                effectComponent->duration = duration;
                effectComponent->effectType = type;
                explosion->AddComponent(effectComponent);
                n += 1;

                TransformComponent* transformComponent = explosion->GetComponent<TransformComponent>();
                transformComponent->SetLocalTransform(cp.position, Quaternion{}, Vector3{ 1.f, 1.f, 1.f });

                scene->AddNode(explosion);
            }
        }
    }
    else
    {
        for (ExplosionEffectComponent* x : explosionEffectComponents->components)
        {
            Entity* parent = x->GetEntity();
            if (!x->effectStarted)
            {
                x->effectStarted = true;

                Entity* explosion = CreateExplosionEffect(x->effectType);
                ParticleEffectComponent* particle = explosion->GetComponent<ParticleEffectComponent>();
                particle->Start();

                parent->AddNode(explosion);
                continue;
            }

            x->duration -= timeElapsed;
            if (x->duration <= 0.f)
            {
                scene->RemoveNode(parent);
            }
        }
    }
}

Entity* ExplosionEffectSystem::CreateExplosionEffect(int type)
{
    auto loadModel = [](const FilePath& filename, const char* modelName = nullptr) -> Entity* {
        ScopedPtr<Scene> scene(new Scene);
        SceneFileV2::eError err = scene->LoadScene(filename);
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        Entity* model = scene->GetEntityByID(1)->Clone();
        if (modelName != nullptr)
            model->SetName(modelName);
        return model;
    };

    Entity* result = nullptr;
    switch (type)
    {
    case 0:
        if (explosionModel1 == nullptr)
            explosionModel1 = loadModel("~res:/3d/Explosion/groundHit_APCR1.sc2", "ExplosionModel_0");
        result = explosionModel1->Clone();
        break;
    case 1:
        if (explosionModel2 == nullptr)
            explosionModel2 = loadModel("~res:/3d/Explosion/shot_large_mb.sc2", "ExplosionModel_1");
        result = explosionModel2->Clone();
        break;
    }
    DVASSERT(result != nullptr);
    return result;
}
