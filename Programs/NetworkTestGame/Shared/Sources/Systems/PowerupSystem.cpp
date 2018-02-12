#include "PowerupSystem.h"
#include "Components/PowerupComponent.h"
#include "Components/PowerupCatcherComponent.h"
#include "Components/HealthComponent.h"
#include "Components/SpeedModifierComponent.h"

#include <limits>
#include <algorithm>
#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Utils/Random.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Scene.h>

#include <NetworkCore/NetworkCoreUtils.h>

#include <Physics/MeshShapeComponent.h>
#include <Physics/StaticBodyComponent.h>
#include <Physics/CollisionSingleComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PowerupSystem)
{
    ReflectionRegistrator<PowerupSystem>::Begin()
    .ConstructorByPointer<Scene*>()
    //.Method("ProcessFixed", &PowerupSystem::ProcessFixed)
    .End();
}

PowerupSystem::PowerupSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<PowerupComponent>())
{
    Logger::Info("create PowerupSystem");
}

void PowerupSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("PowerupSystem::ProcessFixed");

    CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingletonComponent<CollisionSingleComponent>();

    for (Entity* bonus : bonuses)
    {
        triggeredEntities.clear();
        collisionSingleComponent->GetEntitesByTrigger(bonus->GetChild(0), triggeredEntities);
        for (Entity* target : triggeredEntities)
        {
            if (ApplyBonus(bonus, target))
            {
                bonusesToRemove.push_back(bonus);
                break;
            }
        }
    }

    if (!bonusesToRemove.empty())
    {
        for (Entity* removedBonus : bonusesToRemove)
            GetScene()->RemoveNode(removedBonus);

        bonusesToRemove.clear();
    }
}

bool PowerupSystem::ApplyBonus(Entity* bonus, Entity* target)
{
    if (!target->GetComponent<PowerupCatcherComponent>())
        return false;

    PowerupComponent* powerup = bonus->GetComponent<PowerupComponent>();
    const PowerupDescriptor& descr = powerup->GetDescriptor();

    switch (descr.type)
    {
    case PowerupType::HEALTH:
    {
        HealthComponent* health = target->GetComponent<HealthComponent>();
        if (!health)
            return false;

        uint32 scaledHealth = static_cast<uint32>(health->GetHealth() * descr.factor);
        uint8 newHealth = std::min(static_cast<uint32>(std::numeric_limits<uint8>::max()), scaledHealth);
        health->SetHealth(newHealth);
    }
    break;

    case PowerupType::SPEED:
    {
        SpeedModifierComponent* speedModifier = target->GetComponent<SpeedModifierComponent>();
        if (!speedModifier)
        {
            speedModifier = new SpeedModifierComponent();
            target->AddComponent(speedModifier);
        }
        speedModifier->SetFactor(speedModifier->GetFactor() * descr.factor);
    }
    break;
    }

    return true;
}

void PowerupSystem::AddEntity(Entity* bonus)
{
    bonuses.push_back(bonus);
}

void PowerupSystem::RemoveEntity(Entity* bonus)
{
    auto found = std::find(bonuses.begin(), bonuses.end(), bonus);
    if (found != bonuses.end())
        bonuses.erase(found);
}
