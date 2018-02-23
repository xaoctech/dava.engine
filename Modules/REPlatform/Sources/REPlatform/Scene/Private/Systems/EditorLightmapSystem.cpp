#include "REPlatform/Scene/Systems/EditorLightmapSystem.h"

#include <Debug/DVAssert.h>
#include <Entity/Component.h>
#include <Entity/ComponentUtils.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/LightmapComponent.h>
#include <Scene3D/Components/SingleComponents/LightmapSingleComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Render/Highlevel/RenderObject.h>

namespace DAVA
{
EditorLightmapSystem::EditorLightmapSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<LightmapComponent>())
{
}

void EditorLightmapSystem::AddEntity(Entity* entity)
{
    for (uint32 c = 0; c < entity->GetComponentCount<LightmapComponent>(); ++c)
        UpdateDynamicParams(entity->GetComponent<LightmapComponent>(c));
}

void EditorLightmapSystem::RemoveEntity(Entity* entity)
{
    for (uint32 c = 0; c < entity->GetComponentCount<LightmapComponent>(); ++c)
        RemoveDynamicParams(entity->GetComponent<LightmapComponent>(c));
}

void EditorLightmapSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (component->GetType()->Is<LightmapComponent>())
        UpdateDynamicParams(static_cast<LightmapComponent*>(component));
}

void EditorLightmapSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (component->GetType()->Is<LightmapComponent>())
        RemoveDynamicParams(static_cast<LightmapComponent*>(component));
}

void EditorLightmapSystem::Process(float32 delta)
{
    for (LightmapComponent* component : GetScene()->GetSingletonComponent<LightmapSingleComponent>()->lightmapSizeChanged)
        UpdateDynamicParams(component);
}

void EditorLightmapSystem::UpdateDynamicParams(LightmapComponent* component)
{
    if (component == nullptr || component->GetParamsCount() == 0u)
        return;

    for (RenderObject* renderObject : GetRenderObjects(component->GetEntity()))
    {
        if (renderObject->GetType() == RenderObject::TYPE_LANDSCAPE)
        {
            const float32* propertyData = component->GetLightmapParam(0).GetLightmapSizePtr();
            renderObject->SetDynamicProperty(DynamicBindings::PARAM_STATIC_SHADOW_AO_SIZE, propertyData, pointer_size(propertyData));
        }
        else
        {
            uint32 paramsCount = Min(component->GetParamsCount(), renderObject->GetRenderBatchCount());
            for (uint32 p = 0; p < paramsCount; ++p)
            {
                const float32* propertyData = component->GetLightmapParam(p).GetLightmapSizePtr();
                renderObject->SetDynamicProperty(renderObject->GetRenderBatch(p), DynamicBindings::PARAM_STATIC_SHADOW_AO_SIZE, propertyData, pointer_size(propertyData));
            }
        }
    }
}

void EditorLightmapSystem::RemoveDynamicParams(LightmapComponent* component)
{
    if (component == nullptr || component->GetParamsCount() == 0u)
        return;

    for (RenderObject* renderObject : GetRenderObjects(component->GetEntity()))
    {
        if (renderObject->GetType() == RenderObject::TYPE_LANDSCAPE)
        {
            renderObject->RemoveDynamicProperty(DynamicBindings::PARAM_STATIC_SHADOW_AO_SIZE);
        }
        else
        {
            uint32 paramsCount = Min(component->GetParamsCount(), renderObject->GetRenderBatchCount());
            for (uint32 p = 0; p < paramsCount; ++p)
                renderObject->RemoveDynamicProperty(renderObject->GetRenderBatch(p), DynamicBindings::PARAM_STATIC_SHADOW_AO_SIZE);
        }
    }
}

} // namespace DAVA
