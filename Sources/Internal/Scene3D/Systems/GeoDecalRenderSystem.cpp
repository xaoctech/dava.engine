#include "Scene3D/Systems/GeoDecalRenderSystem.h"
#include "Scene3D/Components/GeoDecalRenderComponent.h"

namespace DAVA
{
GeoDecalRenderSystem::GeoDecalRenderSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void GeoDecalRenderSystem::AddComponent(Entity* entity, Component* component)
{
    GeoDecalRenderComponent* decalComponent = static_cast<GeoDecalRenderComponent*>(component);
    GetScene()->renderSystem->RenderPermanent(decalComponent->GetRenderObject());
}

void GeoDecalRenderSystem::RemoveComponent(Entity* entity, Component* component)
{
    GeoDecalRenderComponent* decalComponent = static_cast<GeoDecalRenderComponent*>(component);
    GetScene()->renderSystem->RemoveFromRender(decalComponent->GetRenderObject());
}

void GeoDecalRenderSystem::AddEntity(Entity* entity)
{
    for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_RENDER_COMPONENT); i < e; ++i)
    {
        GeoDecalRenderComponent* decal = static_cast<GeoDecalRenderComponent*>(entity->GetComponent(Component::GEO_DECAL_RENDER_COMPONENT, i));
        AddComponent(entity, decal);
    }
}

void GeoDecalRenderSystem::RemoveEntity(Entity* entity)
{
    for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_RENDER_COMPONENT); i < e; ++i)
    {
        GeoDecalRenderComponent* decal = static_cast<GeoDecalRenderComponent*>(entity->GetComponent(Component::GEO_DECAL_RENDER_COMPONENT, i));
        RemoveComponent(entity, decal);
    }
}

}
