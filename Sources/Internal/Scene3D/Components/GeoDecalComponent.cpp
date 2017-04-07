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
        archive->SetString("image", config.image.GetRelativePathname(serializationContext->GetScenePath()));
        archive->SetUInt32("mapping", config.mapping);
    }
}

void GeoDecalComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive)
    {
        Config localConfig;
        Vector3 center = archive->GetVector3("box.center");
        Vector3 size = 0.5f * archive->GetVector3("box.size", Vector3(1.0f, 1.0f, 1.0f));
        localConfig.boundingBox = AABBox3(center - size, center + size);
        localConfig.image = serializationContext->GetScenePath() + archive->GetString("image");
        localConfig.mapping = static_cast<Mapping>(archive->GetUInt32("mapping", localConfig.mapping));
        config = localConfig;
    }
    Component::Deserialize(archive, serializationContext);
}
}
