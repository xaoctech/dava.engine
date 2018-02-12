#include "PowerupSpawnSystem.h"
#include "Components/PowerupComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Utils/Random.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Scene.h>
#include <Render/Highlevel/RenderObject.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>

#include <Physics/ConvexHullShapeComponent.h>
#include <Physics/StaticBodyComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(PowerupSpawnSystem)
{
    ReflectionRegistrator<PowerupSpawnSystem>::Begin()
    .ConstructorByPointer<Scene*>()
    //.Method("ProcessFixed", &PowerupSpawnSystem::ProcessFixed)
    .End();
}

PowerupSpawnSystem::PowerupSpawnSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<PowerupComponent>())
    , isServer(IsServer(scene))
{
    Logger::Info("create PowerupSpawnSystem");
}

void PowerupSpawnSystem::PrepareForRemove()
{
}

void PowerupSpawnSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("PowerupSpawnSystem::ProcessFixed");

    if (isServer)
    {
        if (bonusCount < bonusCountLimit)
        {
            //generation
            lastPowerupTime -= timeElapsed;
            if (lastPowerupTime < 0)
            {
                lastPowerupTime = cooldown;
                Logger::Info("PowerupSpawnSystem: create bonus!");
                GetScene()->AddNode(CreateBonusEntityOnServer());

                bonusCount++;
            }
        }
    }
    else
    {
        if (!pendingSetupEntities.empty())
        {
            for (Entity* entity : pendingSetupEntities)
            {
                AddBonusModel(entity);
            }
            pendingSetupEntities.clear();
        }
    }
}

ScopedPtr<Entity> PowerupSpawnSystem::CreateBonusEntityOnServer()
{
    ScopedPtr<Entity> bonus = ScopedPtr<Entity>(new Entity());
    bonus->SetName("PowerupBonus");

    float32 posX = static_cast<float32>(Random::Instance()->RandFloat(100.) - 50.);
    float32 posY = static_cast<float32>(Random::Instance()->RandFloat(100.) - 50.);

    bonus->GetComponent<TransformComponent>()->SetLocalTransform(
    Vector3(posX, posY, 0),
    Quaternion(),
    Vector3(1.f, 1.f, 1.f));

    PowerupDescriptor descr;
    descr.type = Random::Instance()->Rand() % 2 ? PowerupType::HEALTH : PowerupType::SPEED;
    descr.factor = 1.5f;

    PowerupComponent* powerup = new PowerupComponent();
    powerup->SetDescriptor(descr);
    bonus->AddComponent(powerup);

    bonus->AddComponent(new NetworkTransformComponent());
    bonus->AddComponent(new NetworkReplicationComponent());

    Entity* model = AddBonusModel(bonus);

    // TODO: reconsider using convex shape
    ConvexHullShapeComponent* shape = new ConvexHullShapeComponent();
    shape->SetTypeMaskToCollideWith(1);
    shape->SetTypeMask(2);
    shape->SetOverrideMass(true);
    shape->SetTriggerMode(true);
    model->AddComponent(shape);

    bonus->AddComponent(new StaticBodyComponent());

    return bonus;
}

Entity* PowerupSpawnSystem::AddBonusModel(Entity* bonus)
{
    if (!refBonusModel)
    {
        ScopedPtr<Scene> modelScene(new Scene());
        SceneFileV2::eError ret = modelScene->LoadScene("~res:/Maps/00_global_content/stones/stn_01_flatstone.sc2");
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == ret);
        refBonusModel.reset(modelScene->GetEntityByID(1)->Clone());
    }

    Entity* model = refBonusModel->Clone();
    bonus->AddNode(model);

    PowerupType bonusType = bonus->GetComponent<PowerupComponent>()->GetDescriptor().type;
    TuneBonusModel(model, bonusType);

    return model;
}

void PowerupSpawnSystem::TuneBonusModel(Entity* model, PowerupType bonusType)
{
    Color color;

    switch (bonusType)
    {
    case PowerupType::HEALTH:
        color = Color(1.f, 0.5f, 0.5f, 1.f);
        break;
    case PowerupType::SPEED:
        color = Color(0.5f, 0.5f, 1.f, 1.f);
        break;
    }

    RenderObject* ro = GetRenderObject(model);

    for (uint32 batchIdx = 0; batchIdx < ro->GetRenderBatchCount(); ++batchIdx)
    {
        NMaterial* mat = ro->GetRenderBatch(batchIdx)->GetMaterial();
        mat->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, 1);
        mat->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, color.color, rhi::ShaderProp::TYPE_FLOAT4);
    }
}

void PowerupSpawnSystem::AddEntity(Entity* entity)
{
    if (!isServer)
    {
        pendingSetupEntities.push_back(entity);
    }
}

void PowerupSpawnSystem::RemoveEntity(Entity* entity)
{
    if (!isServer)
    {
        auto found = std::find(pendingSetupEntities.begin(), pendingSetupEntities.end(), entity);
        if (found != pendingSetupEntities.end())
            pendingSetupEntities.erase(found);
    }
}
