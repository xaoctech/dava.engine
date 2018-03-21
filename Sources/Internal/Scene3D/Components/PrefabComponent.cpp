#include "PrefabComponent.h"
#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchive.h"
#include "Reflection/ReflectionRegistrator.h"

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
    GetEngineContext()->assetManager->UnregisterListener(this);
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
    Prefab::PathKey key(filepath);

    prefab = GetEngineContext()->assetManager->GetAsset<Prefab>(key, AssetManager::SYNC, this);
}

void PrefabComponent::SetFilepath(const FilePath& path)
{
    filepath = path;
    AssetManager* assetManager = GetEngineContext()->assetManager;
    assetManager->UnregisterListener(prefab, this);
    prefab = assetManager->GetAsset<Prefab>(Prefab::PathKey(filepath), AssetManager::SYNC, this);
}

const FilePath& PrefabComponent::GetFilepath() const
{
    return filepath;
}

const Asset<Prefab>& PrefabComponent::GetPrefab() const
{
    return prefab;
}

void PrefabComponent::OnAssetReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded)
{
    DVASSERT(original == prefab);
    prefab = std::static_pointer_cast<Prefab>(reloaded);
}
}
