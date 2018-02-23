#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/StreamingSettingsComponent.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(StreamingSettingsComponent)
{
    ReflectionRegistrator<StreamingSettingsComponent>::Begin()
    .ConstructorByPointer()
    .Field("unloadSolidAngle", &StreamingSettingsComponent::unloadSolidAngle)[M::DisplayName("Unload solid angle")]
    .End();
}

Component* StreamingSettingsComponent::Clone(Entity* toEntity)
{
    StreamingSettingsComponent* component = new StreamingSettingsComponent();
    component->SetEntity(toEntity);
    component->unloadSolidAngle = unloadSolidAngle;
    return component;
}

void StreamingSettingsComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (NULL != archive)
    {
        archive->SetFloat("unloadSolidAngle", unloadSolidAngle);
    }
}

void StreamingSettingsComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    if (NULL != archive)
    {
        unloadSolidAngle = archive->GetFloat("unloadSolidAngle", unloadSolidAngle);
    }
}
}
