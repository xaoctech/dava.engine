#include "Scene3D/Systems/LightUpdateSystem.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "Render/Highlevel/Atmosphere.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LightUpdateSystem)
{
    ReflectionRegistrator<LightUpdateSystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &LightUpdateSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 8.0f)]
    .End();
}

LightUpdateSystem::LightUpdateSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<TransformComponent>() | ComponentUtils::MakeMask<LightComponent>())
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LIGHT_PROPERTIES_CHANGED);
}

void LightUpdateSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::LIGHT_PROPERTIES_CHANGED)
    {
        Entity* entity = component->GetEntity();
        RecalcLight(entity);
    }
}

void LightUpdateSystem::Process(float32 timeElapsed)
{
    TransformSingleComponent* tsc = GetScene()->GetSingletonComponent<TransformSingleComponent>();

    for (auto& pair : tsc->worldTransformChanged.map)
    {
        if (pair.first->GetComponentsCount(Type::Instance<LightComponent>()) > 0)
        {
            for (Entity* entity : pair.second)
            {
                RecalcLight(entity);
            }
        }
    }
}

void LightUpdateSystem::RecalcLight(Entity* entity)
{
    // Update new transform pointer, and mark that transform is changed
    bool updateProbe = false;

    const Matrix4* worldTransformPointer = entity->GetComponent<TransformComponent>()->GetWorldTransformPtr();
    Light* light = entity->GetComponent<LightComponent>()->GetLightObject();
    light->SetPositionDirectionFromMatrix(*worldTransformPointer);
    light->SetWorldTransformPtr(worldTransformPointer);
    if (light->GetLightType() == Light::TYPE_DIRECTIONAL)
    {
        updateProbe = true;
        light->AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    }
    else
    {
        light->RemoveFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
        light->SetAABBox(AABBox3(Vector3(0, 0, 0), light->GetRadius() * 2));
    }

    if (light->GetAutoColor())
    {
        const Vector3& sunBaseColor = Vector3(120000.0f, 120000.0f, 120000.0f);
        Vector3 d = -light->GetDirection();
        float a = Atmosphere::IntersectAtmosphere(Atmosphere::GroundLevel, d);

        Vector3 resultColor = Atmosphere::ComputeOutScattering(Atmosphere::GroundLevel, Atmosphere::GroundLevel + a * d, Atmosphere::Parameters());
        resultColor *= sunBaseColor;

        light->SetColor(Color(resultColor.x, resultColor.y, resultColor.y, 1.0f));
    }
    entity->GetScene()->renderSystem->MarkForUpdate(light);

    if (updateProbe)
    {
        entity->GetScene()->renderSystem->GetReflectionRenderer()->UpdateGlobalLightProbe();
    }
}

void LightUpdateSystem::AddEntity(Entity* entity)
{
    Light* lightObject = entity->GetComponent<LightComponent>()->GetLightObject();
    if (lightObject == nullptr)
        return;

    entityObjectMap[entity] = lightObject;

    RecalcLight(entity);
    GetScene()->GetRenderSystem()->AddLight(lightObject);
}

void LightUpdateSystem::RemoveEntity(Entity* entity)
{
    auto lightObjectIter = entityObjectMap.find(entity);

    if (lightObjectIter != entityObjectMap.end())
    {
        Light* lightObject = lightObjectIter->second;

        if (lightObject != nullptr)
        {
            GetScene()->GetRenderSystem()->RemoveLight(lightObject);
            entityObjectMap.erase(entity);
        }
    }
}
void LightUpdateSystem::PrepareForRemove()
{
    RenderSystem* renderSystem = GetScene()->GetRenderSystem();
    for (const auto& node : entityObjectMap)
    {
        renderSystem->RemoveLight(node.second);
    }
    entityObjectMap.clear();
}
};
