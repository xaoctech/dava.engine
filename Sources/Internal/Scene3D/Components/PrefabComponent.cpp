#include "PrefabComponent.h"
#include "Engine/Engine.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchive.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Asset/AssetManager.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(PrefabComponent)
{
    ReflectionRegistrator<PrefabComponent>::Begin()
    .ConstructorByPointer()
    .Field("filepath", &PrefabComponent::filepath)[M::DisplayName("Prefab Path")]
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
    PrefabComponent* newPrefab = new PrefabComponent();
    newPrefab->SetEntity(toEntity);
    newPrefab->filepath = filepath;

    return newPrefab;
}

void PrefabComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    String relativePath = filepath.GetRelativePathname(serializationContext->GetScenePath());
    archive->SetString("relPath", relativePath);
}

void PrefabComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    String relativePath = archive->GetString("relPath");
    filepath = serializationContext->GetScenePath() + relativePath;

    prefab = GetEngineContext()->assetManager->LoadAsset<Prefab>(filepath);
}

void PrefabComponent::SetFilepath(const FilePath& path)
{
    filepath = path;
    prefab = GetEngineContext()->assetManager->LoadAsset<Prefab>(filepath);
}

const FilePath& PrefabComponent::GetFilepath() const
{
    return filepath;
}

const Asset<Prefab>& PrefabComponent::GetPrefab() const
{
    return prefab;
}
}
