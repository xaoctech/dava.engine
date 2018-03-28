#include "ExplosionEffectSystem.h"
#include "Components/ExplosionEffectComponent.h"
#include "Components/ShooterRocketComponent.h"
#include "Components/SingleComponents/EffectQueueSingleComponent.h"
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
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h>

#include <Physics/CollisionSingleComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ExplosionEffectSystem)
{
    ReflectionRegistrator<ExplosionEffectSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ExplosionEffectSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 30.f)]
    .End();
}

ExplosionEffectSystem::ExplosionEffectSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, ComponentMask())
    , isGuiMode(!Engine::Instance()->IsConsoleMode())
    , networkEntities(scene->GetSingleComponent<NetworkEntitiesSingleComponent>())
    , effectQueue(scene->GetSingleComponent<EffectQueueSingleComponent>())
    , explosionEffectComponents(scene->AquireComponentGroup<ExplosionEffectComponent, ExplosionEffectComponent>())
{
    if (isGuiMode)
    {
        LoadEffectModels();
    }
}

ExplosionEffectSystem::~ExplosionEffectSystem() = default;

void ExplosionEffectSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    Scene* scene = GetScene();

    ProcessNewEffects(scene);

    for (ExplosionEffectComponent* x : explosionEffectComponents->components)
    {
        Entity* parent = x->GetEntity();
        if (!x->effectStarted)
        {
            x->effectStarted = true;

            if (isGuiMode)
            {
                Entity* explosion = CreateExplosionEffect(x->effectType);
                ParticleEffectComponent* particle = explosion->GetComponent<ParticleEffectComponent>();
                particle->Start();

                parent->AddNode(explosion);
            }
            continue;
        }

        x->duration -= timeElapsed;
        if (x->duration <= 0.f && !x->linkedEffect)
        {
            scene->RemoveNode(parent);
        }
    }
}

void ExplosionEffectSystem::ProcessNewEffects(DAVA::Scene* scene)
{
    const bool isClient = IsClient(scene);
    const auto& newEffects = effectQueue->GetQueuedEffects();
    for (const auto& descriptor : newEffects)
    {
        if (descriptor.networkId == NetworkID::INVALID && isClient)
            continue;

        const bool isLinkedEffect = descriptor.parentId != NetworkID::INVALID;

        Entity* parentEntity = nullptr;
        if (isLinkedEffect)
        {
            parentEntity = networkEntities->FindByID(descriptor.parentId);
            if (parentEntity == nullptr)
            {
                DVASSERT(0);
                continue;
            }
        }

        NetworkID effectEntityId = descriptor.networkId == NetworkID::INVALID ? NetworkID::CreatePlayerOwnId(0) : descriptor.networkId;

        Entity* effectEntity = new Entity;
        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(effectEntityId);

        ExplosionEffectComponent* effectComponent = new ExplosionEffectComponent;
        effectComponent->effectType = descriptor.resourceId;
        effectComponent->duration = descriptor.duration;
        effectComponent->linkedEffect = isLinkedEffect;

        replicationComponent->SetForReplication<ExplosionEffectComponent>(M::Privacy::PUBLIC);
        effectEntity->AddComponent(effectComponent);

        if (!isLinkedEffect)
        {
            effectEntity->AddComponent(new ObservableComponent);
            effectEntity->AddComponent(new SimpleVisibilityShapeComponent);

            replicationComponent->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
            effectEntity->AddComponent(new NetworkTransformComponent);

            TransformComponent* transformComponent = effectEntity->GetComponent<TransformComponent>();
            transformComponent->SetLocalTransform(descriptor.position, descriptor.rotation, Vector3{ 1.f, 1.f, 1.f });

            scene->AddNode(effectEntity);
        }
        else
        {
            effectEntity->SetName("rocket_linked_effect");

            parentEntity->AddNode(effectEntity);
        }

        effectEntity->AddComponent(replicationComponent);
    }
}

Entity* ExplosionEffectSystem::CreateExplosionEffect(int type)
{
    Entity* e = nullptr;
    if (0 <= type && type < static_cast<int>(effectModelCache.size()))
    {
        e = effectModelCache[type]->Clone();
    }
    DVASSERT(e != nullptr);
    return e;
}

void ExplosionEffectSystem::LoadEffectModels()
{
    auto loadModel = [](const FilePath& filename, const char* modelName = nullptr) -> Entity* {
        ScopedPtr<Scene> scene(new Scene);
        SceneFileV2::eError err = scene->LoadScene(filename);
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        Entity* model = scene->GetEntityByID(1)->Clone();
        if (modelName != nullptr)
        {
            model->SetName(modelName);
        }
        return model;
    };

    effectModelCache.push_back(loadModel("~res:/3d/Explosion/groundHit_APCR1.sc2", "effect:rocket_explosion"));
    effectModelCache.push_back(loadModel("~res:/3d/Explosion/shot_large_mb.sc2", "effect:rocket_shoot"));
    effectModelCache.push_back(loadModel("~res:/3d/Explosion/Fire_and_Smoke_small.sc2", "effect:rocket_track"));
}
