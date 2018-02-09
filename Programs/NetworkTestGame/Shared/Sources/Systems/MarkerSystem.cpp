#include "MarkerSystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialNames.h>
#include <Scene3D/Components/ComponentHelpers.h>

#include "Scene3D/Components/TransformComponent.h"
#include "Components/HealthComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/RocketSpawnComponent.h"

#include <NetworkCore/NetworkCoreUtils.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(MarkerSystem)
{
    ReflectionRegistrator<MarkerSystem>::Begin()[M::Tags("marker")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &MarkerSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_END, SP::Type::FIXED, 1000.0f)]
    .End();
}

MarkerSystem::MarkerSystem(DAVA::Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<HealthComponent>() | ComponentUtils::MakeMask<GameStunnableComponent>())
{
}

void MarkerSystem::AddEntity(Entity* entity)
{
    pendingEntities.insert(entity);
}

void MarkerSystem::RemoveEntity(Entity* entity)
{
    pendingEntities.erase(entity);
    tankToBar.erase(entity);
}

void MarkerSystem::Simulate(DAVA::Entity* tank)
{
    Entity* bar = tankToBar[tank];
    DVASSERT(bar);
    if (!bar)
    {
        Logger::Error("Tank:%d has not health bar", tank->GetID());
        return;
    }

    Color c(0.0f, 1.0f, 0.0f, 0.5f);
    const HealthComponent* healthComponent = tank->GetComponent<HealthComponent>();

    TransformComponent* trans = bar->GetComponent<TransformComponent>();
    float percentage = healthComponent->GetHealth() / static_cast<float32>(maxHealth);
    float32 z = percentage * maxHealthBarHeight;

    c.r = Lerp(Color::Red.r, Color::Green.r, percentage);
    c.g = Lerp(Color::Red.g, Color::Green.g, percentage);
    c.b = Lerp(Color::Red.b, Color::Green.b, percentage);

    if (healthComponent->GetHealth() == 0)
    {
        z = maxHealthBarHeight / 5.0f;
    }

    RocketSpawnComponent* rocketSpawnComp = tank->GetComponent<RocketSpawnComponent>();
    bool isSpawn = false;
    if (rocketSpawnComp)
    {
        const float32 progress = rocketSpawnComp->progress / static_cast<float32>(RocketSpawnComponent::THRESHOLD);
        isSpawn = progress > 0;
        static const Color activatedColor = Color::Yellow;
        c.r = Lerp(c.r, activatedColor.r, progress);
        c.g = Lerp(c.g, activatedColor.g, progress);
        c.b = Lerp(c.b, activatedColor.b, progress);
        c.a = Lerp(c.a, activatedColor.a, progress);
    }

    bool isStun = false;
    GameStunnableComponent* stunComp = tank->GetComponent<GameStunnableComponent>();
    if (stunComp && stunComp->IsStunned())
    {
        isStun = true;
        c = Color::Blue;
    }

    if (isSpawn || isStun || z != trans->GetScale().z)
    {
        trans->SetLocalTransform(trans->GetPosition(), trans->GetRotation(), Vector3(0.01f, 0.01f, z));

        RenderObject* ro = GetRenderObject(bar);
        ro->GetRenderBatch(0)->GetMaterial()->SetPropertyValue(NMaterialParamName::PARAM_FLAT_COLOR, c.color);
    }
}

void MarkerSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (Entity* tank : pendingEntities)
    {
        ScopedPtr<Scene> model(new Scene());
        SceneFileV2::eError ret = model->LoadScene("~res:/tst.sc2");
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == ret);
        Entity* bar = model->GetEntityByID(1)->Clone();
        RenderObject* ro = GetRenderObject(bar);
        ro->GetRenderBatch(0)->GetMaterial()->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, 1);
        Color c = Color(0.0, 1.0, 0.0, 0.5);
        ro->GetRenderBatch(0)->GetMaterial()->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, c.color, rhi::ShaderProp::TYPE_FLOAT4);
        TransformComponent* trans = bar->GetComponent<TransformComponent>();
        const float32 z = maxHealthBarHeight;

        Vector3 position = Vector3(0.f, 0.f, 2.0f);
        if (!IsServer(GetScene()) && IsClientOwner(GetScene(), tank))
        {
            position = Vector3(0.5f, 0.f, 1.0f);
        }

        trans->SetLocalTransform(position, trans->GetRotation(), Vector3(0.01f, 0.01f, z));
        tank->AddNode(bar);
        bar->SetName("HealthBar");
        tankToBar[tank] = bar;
    }
    pendingEntities.clear();

    for (auto it : tankToBar)
    {
        Entity* tank = it.first;
        Simulate(tank);
    }
}

void MarkerSystem::SetHealthParams(DAVA::uint32 maxHealth_, DAVA::float32 maxHealthBarHeight_)
{
    maxHealth = maxHealth_;
    maxHealthBarHeight = maxHealthBarHeight_;
}
