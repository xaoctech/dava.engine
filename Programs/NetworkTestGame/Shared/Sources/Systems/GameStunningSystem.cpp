#include "GameStunningSystem.h"

#include "Components/GameStunningComponent.h"
#include "Components/GameStunnableComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include <Physics/CollisionSingleComponent.h>
#include <algorithm>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameStunningSystem)
{
    ReflectionRegistrator<GameStunningSystem>::Begin()[M::Tags("server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &GameStunningSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 1.0f)]
    .End();
}

namespace GameStunningSystemDetail
{
static const float32 STUNNING_COOLDOWN_S = 5.f;
}

GameStunningSystem::GameStunningSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    stunnableGroup = scene->AquireComponentGroup<GameStunnableComponent, GameStunnableComponent, NetworkTransformComponent>();
}

void GameStunningSystem::ProcessFixed(float32 timeElapsed)
{
    CollisionSingleComponent* collSingleComp = GetScene()->GetSingletonComponent<CollisionSingleComponent>();
    for (CollisionInfo& ci : collSingleComp->collisions)
    {
        GameStunningComponent* stunning1 = ci.first->GetComponent<GameStunningComponent>();
        GameStunningComponent* stunning2 = ci.second->GetComponent<GameStunningComponent>();
        GameStunnableComponent* stunnable1 = ci.first->GetComponent<GameStunnableComponent>();
        GameStunnableComponent* stunnable2 = ci.second->GetComponent<GameStunnableComponent>();

        if (stunning1 && stunnable2)
        {
            stunnable2->SetCooldown(GameStunningSystemDetail::STUNNING_COOLDOWN_S);
        }
        if (stunning2 && stunnable1)
        {
            stunnable1->SetCooldown(GameStunningSystemDetail::STUNNING_COOLDOWN_S);
        }
    }

    for (GameStunnableComponent* c : stunnableGroup->components)
    {
        if (c->IsStunned())
        {
            c->SetCooldown(std::max(0.f, c->GetCooldown() - NetworkTimeSingleComponent::FrameDurationS));
        }
    }
}
