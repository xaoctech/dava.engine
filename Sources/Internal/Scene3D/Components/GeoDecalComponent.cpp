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

GeoDecalComponent::GeoDecalComponent()
{
    char materialName[256] = {};
    sprintf(materialName, "decal_material_%p", this);

    ScopedPtr<Texture> defaultTexture(Texture::CreatePink());
    dataNodeMaterial->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, defaultTexture.get());
    dataNodeMaterial->AddTexture(NMaterialTextureName::TEXTURE_NORMAL, defaultTexture.get());
    dataNodeMaterial->SetMaterialName(FastName(materialName));
}

Component* GeoDecalComponent::Clone(Entity* toEntity)
{
    GeoDecalComponent* result = new GeoDecalComponent();
    result->SetEntity(toEntity);
    result->config = config;
    result->ConfigChanged();
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
        Vector3 center = archive->GetVector3("box.center");
        Vector3 size = 0.5f * archive->GetVector3("box.size", Vector3(1.0f, 1.0f, 1.0f));
        String albedo = archive->GetString("albedo");
        String normal = archive->GetString("normal");

        GeoDecalManager::DecalConfig localConfig;
        localConfig.boundingBox = AABBox3(center - size, center + size);
        localConfig.albedo = albedo.empty() ? FilePath() : (serializationContext->GetScenePath() + albedo);
        localConfig.normal = normal.empty() ? FilePath() : (serializationContext->GetScenePath() + normal);
        localConfig.mapping = static_cast<GeoDecalManager::Mapping>(archive->GetUInt32("mapping", localConfig.mapping));
        localConfig.uvOffset = archive->GetVector2("uvoffset", Vector2(0.0f, 0.0f));
        localConfig.uvScale = archive->GetVector2("uvscale", Vector2(1.0f, 1.0f));
        config = localConfig;
        ConfigChanged();
    }
    Component::Deserialize(archive, serializationContext);
}

void GeoDecalComponent::GetDataNodes(Set<DataNode*>& dataNodes)
{
    dataNodes.insert(dataNodeMaterial.get());
}

void GeoDecalComponent::ConfigChanged()
{
    ScopedPtr<Texture> albedoTexture(Texture::CreateFromFile(config.albedo));
    ScopedPtr<Texture> normalTexture(Texture::CreateFromFile(config.normal));
    dataNodeMaterial->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, albedoTexture);
    dataNodeMaterial->SetTexture(NMaterialTextureName::TEXTURE_NORMAL, normalTexture);
}
}
