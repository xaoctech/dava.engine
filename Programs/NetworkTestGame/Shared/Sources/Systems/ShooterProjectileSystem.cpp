#include "Systems/ShooterProjectileSystem.h"
#include "Components/ShooterProjectileComponent.h"

#include "Reflection/ReflectionRegistrator.h"
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/ActionCollectSystem.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <Physics/PhysicsSystem.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterProjectileSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterProjectileSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterProjectileSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 15.0f)]
    .End();
}

ShooterProjectileSystem::ShooterProjectileSystem(DAVA::Scene* scene)
    : DAVA::BaseSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterProjectileComponent>())
{
}

void ShooterProjectileSystem::AddEntity(DAVA::Entity* entity)
{
}

void ShooterProjectileSystem::RemoveEntity(DAVA::Entity* entity)
{
}

void ShooterProjectileSystem::ProcessFixed(DAVA::float32 dt)
{
}

void ShooterProjectileSystem::Simulate(DAVA::Entity* entity)
{
}

void ShooterProjectileSystem::PrepareForRemove()
{
}