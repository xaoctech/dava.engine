#include "Scene3D/Components/PrefabComponent.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(PrefabComponent)
{
    ReflectionRegistrator<PrefabComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

PrefabComponent::PrefabComponent()
{
}

PrefabComponent::~PrefabComponent()
{
}

Component* PrefabComponent::Clone(Entity* toEntity)
{
    PrefabComponent* component = new PrefabComponent();
    component->SetEntity(toEntity);
    return component;
}

void PrefabComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (archive != 0)
    {
        String relativePathname = prefabFilepath.GetRelativePathname(serializationContext->GetScenePath());
        archive->SetString("prefabPathname", relativePathname);
    }
}

void PrefabComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive)
    {
    }
    Component::Deserialize(archive, serializationContext);
}
};
