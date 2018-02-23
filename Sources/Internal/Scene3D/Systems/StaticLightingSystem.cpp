#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/StaticLightingSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LightmapComponent.h"
#include "Scene3D/Components/LightmapDataComponent.h"
#include "Scene3D/Components/LandscapeComponent.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/SingleComponents/LightmapSingleComponent.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/SpeedTreeObject.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(StaticLightingSystem)
{
    ReflectionRegistrator<StaticLightingSystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &StaticLightingSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 17.0f)]
    .End();
}

StaticLightingSystem::StaticLightingSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    static const uint32 DEFAULT_TEXTURE_SIZE = 16u;

    uint32 data[DEFAULT_TEXTURE_SIZE * DEFAULT_TEXTURE_SIZE];
    Memset(data, 0xff, sizeof(data));

    rhi::Texture::Descriptor desc;
    desc.format = rhi::TEXTURE_FORMAT_R8G8B8A8;
    desc.width = desc.height = DEFAULT_TEXTURE_SIZE;
    desc.levelCount = uint32(HighestBitIndex(DEFAULT_TEXTURE_SIZE)) + 1u;

    for (uint32 l = 0; l < desc.levelCount; ++l)
        desc.initialData[l] = data;

    defaultLightmapTexture = rhi::CreateTexture(desc);

    lightmapSingleComponent = scene->GetSingletonComponent<LightmapSingleComponent>();
}

StaticLightingSystem::~StaticLightingSystem()
{
    rhi::DeleteTexture(defaultLightmapTexture);
}

void StaticLightingSystem::AddEntity(Entity* entity)
{
    LightmapDataComponent* lightmapDataComponent = entity->GetComponent<LightmapDataComponent>();
    if (lightmapDataComponent != nullptr)
    {
        lightmapDataComponents.emplace_back(lightmapDataComponent);

        lightmapDataComponent->RebuildIDs();
        UpdateDynamicParams(lightmapDataComponent);
    }

    LightmapComponent* lightmapComponent = entity->GetComponent<LightmapComponent>();
    if (lightmapComponent != nullptr)
    {
        lightmapComponents.emplace_back(lightmapComponent);

        LightmapDataComponent* dataComponent = FindLightmapData(entity);
        if (dataComponent != nullptr)
        {
            UpdateDynamicParams(dataComponent);
        }
        else
        {
            SetDefaultLighmap(lightmapComponent);
        }
    }
}

void StaticLightingSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (component->GetType()->Is<LightmapDataComponent>())
    {
        LightmapDataComponent* lightmapDataComponent = static_cast<LightmapDataComponent*>(component);
        lightmapDataComponents.emplace_back(lightmapDataComponent);

        lightmapDataComponent->RebuildIDs();
        UpdateDynamicParams(lightmapDataComponent);
    }

    if (component->GetType()->Is<LightmapComponent>())
    {
        LightmapComponent* lightmapComponent = static_cast<LightmapComponent*>(component);
        lightmapComponents.emplace_back(lightmapComponent);

        MeshComponent* meshComponent = entity->GetComponent<MeshComponent>();
        if (meshComponent != nullptr)
            lightmapComponent->SetParamsCount(meshComponent->GetMesh()->GetRenderBatchCount());

        SpeedTreeComponent* speedTreeComponent = entity->GetComponent<SpeedTreeComponent>();
        if (speedTreeComponent != nullptr)
            lightmapComponent->SetParamsCount(speedTreeComponent->GetSpeedTreeObject()->GetRenderBatchCount());

        LandscapeComponent* landscapeComponent = entity->GetComponent<LandscapeComponent>();
        if (landscapeComponent != nullptr)
            lightmapComponent->SetParamsCount(1);

        if (FindLightmapData(entity) == nullptr)
            SetDefaultLighmap(lightmapComponent);
    }
}

void StaticLightingSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (component->GetType()->Is<LightmapDataComponent>())
    {
        RemoveDynamicParams(static_cast<LightmapDataComponent*>(component));
        lightmapDataComponents.erase(std::remove(lightmapDataComponents.begin(), lightmapDataComponents.end(), component), lightmapDataComponents.end());
    }

    if (component->GetType()->Is<LightmapComponent>())
    {
        lightmapComponents.erase(std::remove(lightmapComponents.begin(), lightmapComponents.end(), component), lightmapComponents.end());
        lightmapSingleComponent->EraseComponent(static_cast<LightmapComponent*>(component));
    }
}

void StaticLightingSystem::RemoveEntity(Entity* entity)
{
    LightmapDataComponent* lightmapDataComponent = entity->GetComponent<LightmapDataComponent>();
    if (lightmapDataComponent != nullptr)
    {
        RemoveDynamicParams(lightmapDataComponent);
        lightmapDataComponents.erase(std::remove(lightmapDataComponents.begin(), lightmapDataComponents.end(), lightmapDataComponent), lightmapDataComponents.end());
    }

    LightmapComponent* lightmapComponent = entity->GetComponent<LightmapComponent>();
    if (lightmapComponent != nullptr)
    {
        lightmapComponents.erase(std::remove(lightmapComponents.begin(), lightmapComponents.end(), lightmapComponent), lightmapComponents.end());
        lightmapSingleComponent->EraseComponent(lightmapComponent);
    }
}

void StaticLightingSystem::PrepareForRemove()
{
    for (LightmapDataComponent* component : lightmapDataComponents)
        RemoveDynamicParams(component);

    lightmapDataComponents.clear();
}

void StaticLightingSystem::Process(float32 timeElapsed)
{
    for (LightmapComponent* component : lightmapSingleComponent->shadowReceiverParamChanged)
    {
        LightmapDataComponent* dataComponent = FindLightmapData(component->GetEntity());
        if (dataComponent != nullptr)
        {
            RemoveDynamicParams(dataComponent);
            dataComponent->RebuildIDs();

            SetDefaultLighmap(component);
            UpdateDynamicParams(dataComponent);
        }
    }
}

void StaticLightingSystem::InvalidateLightmapData()
{
    for (LightmapDataComponent* dataComponent : lightmapDataComponents)
    {
        dataComponent->RemoveLightmapData();
        UpdateDynamicParams(dataComponent);
    }
}

void StaticLightingSystem::UpdateLightmapData()
{
    for (LightmapDataComponent* dataComponent : lightmapDataComponents)
    {
        dataComponent->ReloadLightmaps();
        UpdateDynamicParams(dataComponent);
    }
}

LightmapDataComponent* StaticLightingSystem::FindLightmapData(BaseObject* object)
{
    for (LightmapDataComponent* component : lightmapDataComponents)
    {
        if (!component->GetLightmapID(object).empty())
            return component;
    }

    return nullptr;
}

LightmapDataComponent* StaticLightingSystem::FindLightmapData(Entity* entity)
{
    while (entity != nullptr)
    {
        if (entity->GetComponentCount<LightmapDataComponent>() != 0u)
            return entity->GetComponent<LightmapDataComponent>();

        entity = entity->GetParent();
    }

    return nullptr;
}

void StaticLightingSystem::UpdateDynamicParams(LightmapDataComponent* component)
{
    for (const auto& it : component->lightmapIDs)
    {
        const void* uvData = defaultLightmapUV.data;
        rhi::HTexture handle = defaultLightmapTexture;

        auto found = component->lightmapData.find(it.second);
        if (found != component->lightmapData.end() && found->second.lightmapTexture != nullptr)
        {
            uvData = found->second.uvOffsetScale.data;
            handle = found->second.lightmapTexture->handle;
        }

        UpdateDynamicParams(it.first, uvData, handle);
    }
}

void StaticLightingSystem::UpdateDynamicParams(BaseObject* object, const void* uvParam, rhi::HTexture texture)
{
    RenderObject* renderObject = dynamic_cast<RenderObject*>(object);
    RenderBatch* renderBatch = dynamic_cast<RenderBatch*>(object);

    if (renderObject != nullptr)
    {
        renderObject->SetDynamicProperty(DynamicBindings::PARAM_STATIC_SHADOW_AO_UV, uvParam, reinterpret_cast<pointer_size>(uvParam));
        renderObject->SetDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_SHADOW_AO, texture);
    }
    else if (renderBatch != nullptr)
    {
        renderObject = renderBatch->GetRenderObject();
        renderObject->SetDynamicProperty(renderBatch, DynamicBindings::PARAM_STATIC_SHADOW_AO_UV, uvParam, reinterpret_cast<pointer_size>(uvParam));
        renderObject->SetDynamicTexture(renderBatch, DynamicBindings::DYNAMIC_TEXTURE_SHADOW_AO, texture);
    }
}

void StaticLightingSystem::RemoveDynamicParams(LightmapDataComponent* component)
{
    for (const auto& it : component->lightmapIDs)
        RemoveDynamicParams(it.first);
}

void StaticLightingSystem::RemoveDynamicParams(BaseObject* object)
{
    RenderObject* renderObject = dynamic_cast<RenderObject*>(object);
    RenderBatch* renderBatch = dynamic_cast<RenderBatch*>(object);
    if (renderObject != nullptr)
    {
        renderObject->RemoveDynamicProperty(DynamicBindings::PARAM_STATIC_SHADOW_AO_UV);
        renderObject->RemoveDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_SHADOW_AO);
    }
    else if (renderBatch != nullptr)
    {
        renderObject = renderBatch->GetRenderObject();

        renderObject->RemoveDynamicProperty(renderBatch, DynamicBindings::PARAM_STATIC_SHADOW_AO_UV);
        renderObject->RemoveDynamicTexture(renderBatch, DynamicBindings::DYNAMIC_TEXTURE_SHADOW_AO);
    }
}

void StaticLightingSystem::SetDefaultLighmap(LightmapComponent* component)
{
    Entity* entity = component->GetEntity();
    for (RenderObject* renderObject : GetRenderObjects(entity))
    {
        if (renderObject->GetType() == RenderObject::TYPE_LANDSCAPE)
        {
            if (component->GetParamsCount() != 0u && component->GetLightmapParam(0).IsReceiveShadow())
                UpdateDynamicParams(renderObject, defaultLightmapUV.data, defaultLightmapTexture);
        }
        else
        {
            uint32 paramsCount = Min(component->GetParamsCount(), renderObject->GetRenderBatchCount());
            for (uint32 p = 0; p < paramsCount; ++p)
            {
                RenderBatch* batch = renderObject->GetRenderBatch(p);
                if (component->GetLightmapParam(p).IsReceiveShadow() || batch->GetMaterial()->GetEffectiveFlagValue(NMaterialFlagName::FLAG_USE_BAKED_LIGHTING) != 0)
                    UpdateDynamicParams(batch, defaultLightmapUV.data, defaultLightmapTexture);
            }
        }
    }
}

};
