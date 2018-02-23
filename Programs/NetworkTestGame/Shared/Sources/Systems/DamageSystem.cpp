#include "DamageSystem.h"

#include "Components/DamageComponent.h"
#include "Components/HealthComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkTimeSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include <Reflection/ReflectionRegistrator.h>

#include <Physics/CollisionSingleComponent.h>

#include <algorithm>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(DamageSystem)
{
    ReflectionRegistrator<DamageSystem>::Begin()[M::Tags("server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &DamageSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 3.0f)]
    .End();
}

DamageSystem::DamageSystem(DAVA::Scene* scene)
    : SceneSystem(scene, 0)
{
    timeComp = scene->GetSingletonComponent<NetworkTimeSingleComponent>();
}

void DamageSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    CollisionSingleComponent* collSingleComp = GetScene()->GetSingletonComponent<CollisionSingleComponent>();
    DamageComponent* damage = nullptr;
    HealthComponent* health = nullptr;
    for (CollisionInfo& ci : collSingleComp->collisions)
    {
        damage = ci.first->GetComponent<DamageComponent>();
        if (damage)
        {
            health = ci.second->GetComponent<HealthComponent>();
            if (health)
            {
                ApplyDamage(health, damage);
            }
        }
        else
        {
            health = ci.first->GetComponent<HealthComponent>();
            if (health)
            {
                damage = ci.second->GetComponent<DamageComponent>();
                if (damage)
                {
                    ApplyDamage(health, damage);
                }
            }
        }
    }
}

void DamageSystem::ApplyDamage(HealthComponent* health, DamageComponent* damage)
{
    health->DecHealth(damage->GetDamage(), timeComp->GetFrameId());
}
