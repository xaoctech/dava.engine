#include "Scene3D/Components/ComponentHelpers.h"

#include "Particles/ParticleEmitter.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/DecalComponent.h"
#include "Scene3D/Components/GeoDecalComponent.h"
#include "Scene3D/Components/GeoDecalComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/SplineComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/VTDecalComponent.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Components/LandscapeComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Lod/LodComponent.h"

namespace DAVA
{
bool HasComponent(const Entity* fromEntity, const Type* componentType)
{
    if (fromEntity != nullptr)
    {
        return (fromEntity->GetComponentCount(componentType) > 0);
    }

    return false;
}

RenderComponent* GetRenderComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return fromEntity->GetComponent<RenderComponent>();
    else
        return nullptr;
}

TransformComponent* GetTransformComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return fromEntity->GetComponent<TransformComponent>();
    else
        return nullptr;
}

SkeletonComponent* GetSkeletonComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return fromEntity->GetComponent<SkeletonComponent>();
    else
        return nullptr;
}

MotionComponent* GetMotionComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return fromEntity->GetComponent<MotionComponent>();
    else
        return nullptr;
}

RenderObject* GetRenderObject(const Entity* fromEntity)
{
    RenderObject* object = nullptr;

    if (nullptr != fromEntity)
    {
        RenderComponent* renderComponent = fromEntity->GetComponent<RenderComponent>();
        if (renderComponent)
        {
            object = renderComponent->GetRenderObject();
        }

        MeshComponent* meshComponent = fromEntity->GetComponent<MeshComponent>();
        if (meshComponent)
        {
            object = meshComponent->GetMesh();
        }

        SpeedTreeComponent* speedTreeComponent = fromEntity->GetComponent<SpeedTreeComponent>();
        if (speedTreeComponent)
        {
            object = speedTreeComponent->GetSpeedTreeObject();
        }

        LandscapeComponent* landscapeComponent = fromEntity->GetComponent<LandscapeComponent>();
        if (landscapeComponent)
        {
            object = landscapeComponent->GetLandscape();
        }
    }

    return object;
}

Vector<RenderObject*> GetRenderObjects(const Entity* fromEntity)
{
    Vector<RenderObject*> result;

    if (fromEntity != nullptr)
    {
        uint32 componentCount = 0;

        componentCount = fromEntity->GetComponentCount(Type::Instance<RenderComponent>());
        for (uint32 i = 0; i < componentCount; ++i)
            result.emplace_back(fromEntity->GetComponent<RenderComponent>(i)->GetRenderObject());

        componentCount = fromEntity->GetComponentCount(Type::Instance<MeshComponent>());
        for (uint32 i = 0; i < componentCount; ++i)
            result.emplace_back(fromEntity->GetComponent<MeshComponent>(i)->GetMesh());

        componentCount = fromEntity->GetComponentCount(Type::Instance<SpeedTreeComponent>());
        for (uint32 i = 0; i < componentCount; ++i)
            result.emplace_back(fromEntity->GetComponent<SpeedTreeComponent>(i)->GetSpeedTreeObject());

        componentCount = fromEntity->GetComponentCount(Type::Instance<LandscapeComponent>());
        for (uint32 i = 0; i < componentCount; ++i)
            result.emplace_back(fromEntity->GetComponent<LandscapeComponent>(i)->GetLandscape());
    }

    return result;
}

Vector<std::pair<Entity*, RenderObject*>> GetRenderObjects(const SortedEntityContainer& container)
{
    Vector<std::pair<Entity*, RenderObject*>> result;

    for (const auto& pair : container.map)
    {
        const EntityFamily* entityFamily = pair.first;
        uint32 componentCount = 0;

        componentCount = entityFamily->GetComponentsCount(Type::Instance<RenderComponent>());
        if (componentCount > 0u)
        {
            for (Entity* entity : pair.second)
            {
                for (uint32 i = 0; i < componentCount; ++i)
                    result.emplace_back(entity, entity->GetComponent<RenderComponent>(i)->GetRenderObject());
            }
        }

        componentCount = entityFamily->GetComponentsCount(Type::Instance<MeshComponent>());
        if (componentCount > 0u)
        {
            for (Entity* entity : pair.second)
            {
                for (uint32 i = 0; i < componentCount; ++i)
                    result.emplace_back(entity, entity->GetComponent<MeshComponent>(i)->GetMesh());
            }
        }

        componentCount = entityFamily->GetComponentsCount(Type::Instance<SpeedTreeComponent>());
        if (componentCount > 0u)
        {
            for (Entity* entity : pair.second)
            {
                for (uint32 i = 0; i < componentCount; ++i)
                    result.emplace_back(entity, entity->GetComponent<SpeedTreeComponent>(i)->GetSpeedTreeObject());
            }
        }

        componentCount = entityFamily->GetComponentsCount(Type::Instance<LandscapeComponent>());
        if (componentCount > 0u)
        {
            for (Entity* entity : pair.second)
            {
                for (uint32 i = 0; i < componentCount; ++i)
                    result.emplace_back(entity, entity->GetComponent<LandscapeComponent>(i)->GetLandscape());
            }
        }
    }

    return result;
}

RenderObject* GetRenderObject(const Component* fromComponent)
{
    if (fromComponent != nullptr)
    {
        if (fromComponent->GetType()->Is<RenderComponent>())
            return static_cast<const RenderComponent*>(fromComponent)->GetRenderObject();
        else if (fromComponent->GetType()->Is<MeshComponent>())
            return static_cast<const MeshComponent*>(fromComponent)->GetMesh();
        else if (fromComponent->GetType()->Is<SpeedTreeComponent>())
            return static_cast<const SpeedTreeComponent*>(fromComponent)->GetSpeedTreeObject();
        else if (fromComponent->GetType()->Is<LandscapeComponent>())
            return static_cast<const LandscapeComponent*>(fromComponent)->GetLandscape();
    }

    return nullptr;
}

SpeedTreeObject* GetSpeedTreeObject(const Entity* fromEntity)
{
    SpeedTreeComponent* component = fromEntity->GetComponent<SpeedTreeComponent>();
    if (component != nullptr)
    {
        return component->GetSpeedTreeObject();
    }

    return nullptr;
}

ParticleEffectComponent* GetEffectComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<ParticleEffectComponent>();
    }

    return nullptr;
}

AnimationComponent* GetAnimationComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<AnimationComponent>();
    }
    return nullptr;
}

LightComponent* GetLightComponent(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        return fromEntity->GetComponent<LightComponent>();
    }

    return nullptr;
}

Light* GetLight(const Entity* fromEntity)
{
    LightComponent* component = GetLightComponent(fromEntity);
    if (component)
    {
        return component->GetLightObject();
    }

    return nullptr;
}

Landscape* GetLandscape(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        LandscapeComponent* component = fromEntity->GetComponent<LandscapeComponent>();
        if (component != nullptr)
        {
            return component->GetLandscape();
        }
    }

    return nullptr;
}

VegetationRenderObject* GetVegetation(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        RenderObject* object = GetRenderObject(fromEntity);
        if (object && object->GetType() == RenderObject::TYPE_VEGETATION)
        {
            VegetationRenderObject* vegetation = static_cast<VegetationRenderObject*>(object);
            return vegetation;
        }
    }

    return nullptr;
}

Camera* GetCamera(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        CameraComponent* component = fromEntity->GetComponent<CameraComponent>();
        if (component)
        {
            return component->GetCamera();
        }
    }

    return nullptr;
}

LodComponent* GetLodComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<LodComponent>();
    }

    return nullptr;
}

SwitchComponent* GetSwitchComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<SwitchComponent>();
    }

    return nullptr;
}

ParticleEffectComponent* GetParticleEffectComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<ParticleEffectComponent>();
    }

    return nullptr;
}

SoundComponent* GetSoundComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<SoundComponent>();
    }

    return nullptr;
}

uint32 GetLodLayersCount(const Entity* fromEntity)
{
    if (!fromEntity)
        return 0;

    if (GetEffectComponent(fromEntity))
        return LodComponent::MAX_LOD_LAYERS;

    RenderObject* object = GetRenderObject(fromEntity);
    if (!object)
        return 0;

    return (object->GetMaxLodIndex() + 1);
}

uint32 GetLodLayersCount(LodComponent* fromComponent)
{
    if (!fromComponent)
        return 0;

    Entity* entity = fromComponent->GetEntity();

    if (GetEffectComponent(entity))
        return LodComponent::MAX_LOD_LAYERS;

    RenderObject* object = GetRenderObject(entity);
    if (!object)
        return 0;

    return (object->GetMaxLodIndex() + 1);
}

void RecursiveProcessMeshNode(Entity* curr, void* userData, void (*process)(Entity*, void*))
{
    RenderComponent* comp = curr->GetComponent<RenderComponent>();
    if (comp)
    {
        RenderObject* renderObject = comp->GetRenderObject();
        if (renderObject->GetType() == RenderObject::TYPE_MESH)
        {
            process(curr, userData);
        }
    }
    else
    {
        for (int32 i = 0; i < curr->GetChildrenCount(); i++)
            RecursiveProcessMeshNode(curr->GetChild(i), userData, process);
    }
}

SpeedTreeComponent* GetSpeedTreeComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<SpeedTreeComponent>();
    }

    return nullptr;
}

WindComponent* GetWindComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<WindComponent>();
    }

    return nullptr;
}

WaveComponent* GetWaveComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<WaveComponent>();
    }

    return nullptr;
}

Entity* FindLandscapeEntity(Entity* rootEntity)
{
    if (GetLandscape(rootEntity))
    {
        return rootEntity;
    }

    DAVA::int32 count = rootEntity->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        Entity* landscapeEntity = FindLandscapeEntity(rootEntity->GetChild(i));
        if (landscapeEntity)
        {
            return landscapeEntity;
        }
    }

    return nullptr;
}

Entity* FindVegetationEntity(Entity* rootEntity)
{
    if (GetVegetation(rootEntity))
    {
        return rootEntity;
    }

    DAVA::int32 count = rootEntity->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        Entity* vegetationEntity = FindVegetationEntity(rootEntity->GetChild(i));
        if (vegetationEntity)
        {
            return vegetationEntity;
        }
    }

    return nullptr;
}

Landscape* FindLandscape(Entity* rootEntity)
{
    Entity* entity = FindLandscapeEntity(rootEntity);
    return GetLandscape(entity);
}

VegetationRenderObject* FindVegetation(Entity* rootEntity)
{
    Entity* entity = FindVegetationEntity(rootEntity);
    return GetVegetation(entity);
}

QualitySettingsComponent* GetQualitySettingsComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<QualitySettingsComponent>();
    }

    return nullptr;
}

CustomPropertiesComponent* GetCustomProperties(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<CustomPropertiesComponent>();
    }

    return nullptr;
}

CustomPropertiesComponent* GetOrCreateCustomProperties(Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetOrCreateComponent<CustomPropertiesComponent>();
    }

    return nullptr;
}

KeyedArchive* GetCustomPropertiesArchieve(const Entity* fromEntity)
{
    CustomPropertiesComponent* comp = GetCustomProperties(fromEntity);
    return (comp != nullptr) ? comp->GetArchive() : nullptr;
}

VariantType* GetCustomPropertiesValueRecursive(Entity* fromEntity, const String& name)
{
    if (fromEntity == nullptr)
        return nullptr;

    KeyedArchive* props = GetCustomPropertiesArchieve(fromEntity);
    if ((props != nullptr) && (props->Count(name) > 0))
    {
        return props->GetVariant(name);
    }
    return GetCustomPropertiesValueRecursive(fromEntity->GetParent(), name);
}

PathComponent* GetPathComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<PathComponent>();
    }

    return nullptr;
}

WaypointComponent* GetWaypointComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<WaypointComponent>();
    }

    return NULL;
}

SnapToLandscapeControllerComponent* GetSnapToLandscapeControllerComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<SnapToLandscapeControllerComponent>();
    }

    return nullptr;
}

StaticOcclusionComponent* GetStaticOcclusionComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<StaticOcclusionComponent>();
    }

    return nullptr;
}

StaticOcclusionDebugDrawComponent* GetStaticOcclusionDebugDrawComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<StaticOcclusionDebugDrawComponent>();
    }

    return nullptr;
}

DecalComponent* GetDecalComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<DecalComponent>();
    }

    return nullptr;
}

VTDecalComponent* GetVTDecalComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<VTDecalComponent>();
    }

    return nullptr;
}

GeoDecalComponent* GetGeoDecalComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return fromEntity->GetComponent<GeoDecalComponent>();
    }

    return nullptr;
}
SplineComponent* GetSplineComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return fromEntity->GetComponent<SplineComponent>();
    else
        return nullptr;
}
}
