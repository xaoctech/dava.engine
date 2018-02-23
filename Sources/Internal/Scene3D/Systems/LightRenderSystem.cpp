#include "Scene3D/Systems/LightRenderSystem.h"

#include "Entity/ComponentUtils.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
LightRenderSystem::LightRenderSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<LightComponent>() | ComponentUtils::MakeMask<LightRenderComponent>())
{
    if (scene)
    {
        renderSystem = scene->GetRenderSystem();
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LIGHT_PROPERTIES_CHANGED);
    }
}

void LightRenderSystem::InvalidateMaterials()
{
    for (auto& i : allComponents)
        i.second->GetRenderObject()->InvalidateMaterial();
}

void LightRenderSystem::ImmediateEvent(Component* component, uint32 event)
{
    if ((component->GetType() == Type::Instance<LightComponent>()) && (event == EventSystem::LIGHT_PROPERTIES_CHANGED))
    {
        for (const LightComponentsPair& c : allComponents)
        {
            if (c.first == component)
            {
                LightComponent* light = dynamic_cast<LightComponent*>(c.first);
                LightRenderComponent* lightRender = dynamic_cast<LightRenderComponent*>(c.second);
                lightRender->SetLight(light->GetLightObject());
            }
        }
    }
}

void LightRenderSystem::AddEntity(Entity* entity)
{
    LightComponent* light = entity->GetComponent<LightComponent>();
    LightRenderComponent* lightRender = entity->GetComponent<LightRenderComponent>();
    if (light && lightRender)
    {
        lightRender->SetLight(light->GetLightObject());
        renderSystem->RenderPermanent(lightRender->GetRenderObject());

        allComponents.emplace_back(light, lightRender);
    }
}

void LightRenderSystem::RemoveEntity(Entity* entity)
{
    LightComponent* light = entity->GetComponent<LightComponent>();
    LightRenderComponent* lightRender = entity->GetComponent<LightRenderComponent>();
    if (light && lightRender)
    {
        lightRender->SetLight(nullptr);
        renderSystem->RemoveFromRender(lightRender->GetRenderObject());

        FindAndRemoveExchangingWithLast(allComponents, LightComponentsPair(light, lightRender));
    }
}
void LightRenderSystem::PrepareForRemove()
{
    for (auto lightPair : allComponents)
    {
        LightRenderComponent* lightRender = lightPair.second;
        lightRender->SetLight(nullptr);
        renderSystem->RemoveFromRender(lightRender->GetRenderObject());
    }
    allComponents.clear();
}
};
