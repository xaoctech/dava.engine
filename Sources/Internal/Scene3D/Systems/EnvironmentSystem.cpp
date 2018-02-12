#include "Scene3D/Systems/EnvironmentSystem.h"

#include "Entity/ComponentUtils.h"
#include "Scene3D/Components/EnvironmentComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Render/Renderer.h"

namespace DAVA
{
EnvironmentSystem::EnvironmentSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<EnvironmentComponent>())
{
}

void EnvironmentSystem::AddEntity(Entity* entity)
{
    Component* c = entity->GetComponent<EnvironmentComponent>();
    DVASSERT(c != nullptr);
    entityWithEnvironmentComponent = entity;
}

void EnvironmentSystem::RemoveEntity(Entity* entity)
{
    if (entity == entityWithEnvironmentComponent)
    {
        entityWithEnvironmentComponent = nullptr;
    }
}

void EnvironmentSystem::Process(float32 timeElapsed)
{
    if (entityWithEnvironmentComponent != nullptr)
    {
        EnvironmentComponent* c = static_cast<EnvironmentComponent*>(entityWithEnvironmentComponent->GetComponent<EnvironmentComponent>());
        UpdateFog(c);
    }
}

void EnvironmentSystem::UpdateFog(EnvironmentComponent* c)
{
    fogValues.x = c->GetFogDistanceScale();
    fogValues.y = c->GetFogTurbidity();
    fogValues.z = c->GetFogAnisotropy();
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_FOG_VALUES, &fogValues, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
}
}
