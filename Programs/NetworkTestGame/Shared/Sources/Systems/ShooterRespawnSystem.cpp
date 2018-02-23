#include "Systems/ShooterRespawnSystem.h"
#include "Components/HealthComponent.h"
#include "ShooterUtils.h"
#include "ShooterConstants.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/TransformComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterRespawnSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterRespawnSystem>::Begin()[M::Tags("gm_shooter", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterRespawnSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 20.0f)]
    .End();
}

ShooterRespawnSystem::ShooterRespawnSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, 0)
    , healthComponents(scene->AquireComponentGroup<HealthComponent, HealthComponent>())
    , healthComponentsPending(new DAVA::ComponentGroupOnAdd<HealthComponent>(healthComponents))
{
}

void ShooterRespawnSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    for (HealthComponent* newHealthComponent : healthComponentsPending->components)
    {
        newHealthComponent->GetEntity()->GetComponent<TransformComponent>()->SetLocalTransform(GetRandomPlayerSpawnPosition(), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));
    }
    healthComponentsPending->components.clear();

    for (HealthComponent* healthComponent : healthComponents->components)
    {
        if (healthComponent->GetHealth() == 0)
        {
            healthComponent->SetHealth(SHOOTER_CHARACTER_MAX_HEALTH);
            healthComponent->GetEntity()->GetComponent<TransformComponent>()->SetLocalTransform(GetRandomPlayerSpawnPosition(), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));
        }
    }
}

void ShooterRespawnSystem::PrepareForRemove()
{
}
