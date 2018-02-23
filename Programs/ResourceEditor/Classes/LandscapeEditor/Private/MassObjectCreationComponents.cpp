#include "Classes/LandscapeEditor/Private/MassObjectCreationComponents.h"

#include <REPlatform/Global/REMeta.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

DAVA::Component* MassObjectCreationLayer::Clone(DAVA::Entity* toEntity)
{
    MassObjectCreationLayer* newLayer = new MassObjectCreationLayer();
    newLayer->SetEntity(toEntity);
    newLayer->layerName = layerName;

    return newLayer;
}

void MassObjectCreationLayer::Serialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
    archive->SetFastName("layerName", layerName);
    DAVA::Component::Serialize(archive, serializationContext);
}

void MassObjectCreationLayer::Deserialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
    DAVA::Component::Deserialize(archive, serializationContext);
    layerName = archive->GetFastName("layerName");
}

const DAVA::FastName& MassObjectCreationLayer::GetName() const
{
    return layerName;
}

void MassObjectCreationLayer::SetName(const DAVA::FastName& layerName)
{
    this->layerName = layerName;
}

DAVA_VIRTUAL_REFLECTION_IMPL(MassObjectCreationLayer)
{
    DAVA::ReflectionRegistrator<MassObjectCreationLayer>::Begin()[DAVA::M::NonExportableComponent(),
                                                                  DAVA::M::HiddenField(),
                                                                  DAVA::M::CantBeCreatedManualyComponent(),
                                                                  DAVA::M::CantBeDeletedManualyComponent(),
                                                                  DAVA::M::DisableEntityReparent()]
    .End();
}

DAVA::Component* MassCreatedObjectComponent::Clone(DAVA::Entity* toEntity)
{
    MassCreatedObjectComponent* newComponent = new MassCreatedObjectComponent();
    newComponent->SetEntity(toEntity);
    newComponent->sourceModelPath = sourceModelPath;

    return newComponent;
}

void MassCreatedObjectComponent::Serialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
    DAVA::String path = sourceModelPath.GetRelativePathname(serializationContext->GetScenePath());
    archive->SetString("sourceModelPath", path);
    DAVA::Component::Serialize(archive, serializationContext);
}

void MassCreatedObjectComponent::Deserialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
    DAVA::Component::Deserialize(archive, serializationContext);
    DAVA::String path = archive->GetString("sourceModelPath", "");
    sourceModelPath = DAVA::FilePath(serializationContext->GetScenePath().GetAbsolutePathname() + path);
}

const DAVA::FilePath& MassCreatedObjectComponent::GetSourceModelPath() const
{
    return sourceModelPath;
}

void MassCreatedObjectComponent::SetSourceModelPath(const DAVA::FilePath& path)
{
    sourceModelPath = path;
}

DAVA_VIRTUAL_REFLECTION_IMPL(MassCreatedObjectComponent)
{
    DAVA::ReflectionRegistrator<MassCreatedObjectComponent>::Begin()[DAVA::M::NonExportableComponent(),
                                                                     DAVA::M::HiddenField(),
                                                                     DAVA::M::CantBeCreatedManualyComponent(),
                                                                     DAVA::M::CantBeDeletedManualyComponent(),
                                                                     DAVA::M::DisableEntityReparent()]
    .End();
}
