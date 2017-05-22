#include "Scene3D/Components/GeoDecalComponent.h"
#include "Scene3D/Entity.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Render/Material/NMaterialNames.h"

namespace DAVA
{
Texture* GetSharedPinkTexture()
{
    Texture* staticTexture = nullptr;
    if (staticTexture == nullptr)
        staticTexture = Texture::CreatePink();
    return SafeRetain(staticTexture);
}

DAVA_VIRTUAL_REFLECTION_IMPL(GeoDecalComponent)
{
    ReflectionRegistrator<GeoDecalComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

GeoDecalComponent::GeoDecalComponent(uint32 flags)
{
    if ((flags & SuppressMaterialCreation) == 0)
    {
        char materialName[256] = {};
        sprintf(materialName, "Geo_Decal_Material_%p", reinterpret_cast<void*>(this));

        dataNodeMaterial.reset(new NMaterial());
        dataNodeMaterial->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, GetSharedPinkTexture());
        dataNodeMaterial->AddTexture(NMaterialTextureName::TEXTURE_NORMAL, GetSharedPinkTexture());
        dataNodeMaterial->SetMaterialName(FastName(materialName));
    }
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
    if (dataNodeMaterial.get() != nullptr)
        dataNodes.insert(dataNodeMaterial.get());
}

void GeoDecalComponent::ConfigChanged()
{
    if (dataNodeMaterial.get() != nullptr)
    {
        ScopedPtr<Texture> albedoTexture(Texture::CreateFromFile(config.albedo));
        ScopedPtr<Texture> normalTexture(Texture::CreateFromFile(config.normal));
        dataNodeMaterial->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, albedoTexture);
        dataNodeMaterial->SetTexture(NMaterialTextureName::TEXTURE_NORMAL, normalTexture);
    }
}
}
