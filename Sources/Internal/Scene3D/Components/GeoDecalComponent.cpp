#include "Scene3D/Components/GeoDecalComponent.h"
#include "Scene3D/Entity.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(GeoDecalComponent)
{
    ReflectionRegistrator<GeoDecalComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

Component* GeoDecalComponent::Clone(Entity* toEntity)
{
    GeoDecalComponent* result = new GeoDecalComponent();
    result->SetEntity(toEntity);
    result->config = config;
    return result;
}

void GeoDecalComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (archive)
    {
        archive->SetVector3("box.center", config.boundingBox.GetCenter());
        archive->SetVector3("box.size", config.boundingBox.GetSize());
        archive->SetString("albedo", config.albedo.GetRelativePathname(serializationContext->GetScenePath()));
        archive->SetString("normal", config.normal.GetRelativePathname(serializationContext->GetScenePath()));
        archive->SetUInt32("mapping", config.mapping);
        archive->SetVector2("uvoffset", config.uvOffset);
        archive->SetVector2("uvscale", config.uvScale);
    }
}

void GeoDecalComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive)
    {
        GeoDecalManager::DecalConfig localConfig;
        Vector3 center = archive->GetVector3("box.center");
        Vector3 size = 0.5f * archive->GetVector3("box.size", Vector3(1.0f, 1.0f, 1.0f));
        localConfig.boundingBox = AABBox3(center - size, center + size);
        localConfig.albedo = serializationContext->GetScenePath() + archive->GetString("albedo");
        localConfig.normal = serializationContext->GetScenePath() + archive->GetString("normal");
        localConfig.mapping = static_cast<GeoDecalManager::Mapping>(archive->GetUInt32("mapping", localConfig.mapping));
        localConfig.uvOffset = archive->GetVector2("uvoffset", Vector2(0.0f, 0.0f));
        localConfig.uvScale = archive->GetVector2("uvscale", Vector2(1.0f, 1.0f));
        config = localConfig;
    }
    Component::Deserialize(archive, serializationContext);
}
}
