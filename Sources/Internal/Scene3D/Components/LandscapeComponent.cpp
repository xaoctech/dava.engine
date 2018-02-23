#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Scene3D/Components/LandscapeComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LandscapeComponent)
{
    ReflectionRegistrator<LandscapeComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .Field("landscape", &LandscapeComponent::landscape)[M::DisplayName("Landscape")]
    .End();
}

LandscapeComponent::LandscapeComponent()
{
    landscape = new Landscape();
}

LandscapeComponent::~LandscapeComponent()
{
    SafeRelease(landscape);
}

Component* LandscapeComponent::Clone(Entity* toEntity)
{
    LandscapeComponent* newComponent = new LandscapeComponent();
    newComponent->SetEntity(toEntity);

    newComponent->SetLayersCount(GetLayersCount());
    for (uint32 l = 0; l < GetLayersCount(); ++l)
    {
        for (uint32 i = 0; i < 3; ++i)
            newComponent->SetPageMaterialPath(l, i, GetPageMaterialPath(l, i));
    }

    newComponent->SetHeightmapPath(GetHeighmapPath());
    newComponent->SetLandscapeMaterialPath(GetLandscapeMaterialPath());
    newComponent->SetLandscapeSize(GetLandscapeSize());
    newComponent->SetLandscapeHeight(GetLandscapeHeight());
    newComponent->SetMiddleLODLevel(GetMiddleLODLevel());
    newComponent->SetMacroLODLevel(GetMacroLODLevel());
    newComponent->SetMaxTexturingLevel(GetMaxTexturingLevel());
    newComponent->SetTessellationLevelCount(GetTessellationLevelCount());
    newComponent->SetTessellationHeight(GetTessellationHeight());
    newComponent->landscape->SetFlags(landscape->GetFlags());
    newComponent->landscape->GetDecorationData()->CopyParameters(landscape->GetDecorationData());
    newComponent->landscape->subdivision->SetMetrics(landscape->subdivision->GetMetrics());

    return newComponent;
}

void LandscapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (landscapeMaterial != nullptr)
        archive->SetString("landscapeMaterialPath", landscapeMaterial->GetFilepath().GetRelativePathname(serializationContext->GetScenePath()));

    if (!heightmapPath.IsEmpty())
        archive->SetString("heightmapPath", heightmapPath.GetRelativePathname(serializationContext->GetScenePath()));

    archive->SetFloat("landscapeHeight", landscapeHeight);
    archive->SetFloat("landscapeSize", landscapeSize);

    archive->SetUInt32("middleLODLevel", middleLODLevel);
    archive->SetUInt32("macroLODLevel", macroLODLevel);
    archive->SetUInt32("maxTexturingLevel", maxTexturingLevel);

    archive->SetUInt32("tessellationLevelCount", tessellationLevelCount);
    archive->SetFloat("tessellationHeight", tessellationHeight);

    archive->SetUInt32("layersCount", GetLayersCount());

    for (uint32 l = 0; l < GetLayersCount(); ++l)
    {
        for (uint32 i = 0; i < 3; ++i)
            archive->SetString(Format("layer%u_lod%u_materialPath", l, i), layersPageMaterials[l][i]->GetFilepath().GetRelativePathname(serializationContext->GetScenePath()));
    }

    landscape->GetDecorationData()->Save(archive, serializationContext);
    landscape->SaveFlags(archive, serializationContext);
    landscape->subdivision->GetMetrics().Save(archive);
}

void LandscapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    AssetManager* assetManager = GetEngineContext()->assetManager;

    Component::Deserialize(archive, serializationContext);

    SetHeightmapPath(serializationContext->GetScenePath() + archive->GetString("heightmapPath"));

    SetLandscapeHeight(archive->GetFloat("landscapeHeight"));
    SetLandscapeSize(archive->GetFloat("landscapeSize"));

    SetMiddleLODLevel(archive->GetUInt32("middleLODLevel"));
    SetMacroLODLevel(archive->GetUInt32("macroLODLevel"));
    SetMaxTexturingLevel(archive->GetUInt32("maxTexturingLevel"));

    SetTessellationLevelCount(archive->GetUInt32("tessellationLevelCount"));
    SetTessellationHeight(archive->GetFloat("tessellationHeight"));

    SetLandscapeMaterialPath(serializationContext->GetScenePath() + archive->GetString("landscapeMaterialPath"));

    SetLayersCount(archive->GetUInt32("layersCount"));
    for (uint32 l = 0; l < GetLayersCount(); ++l)
    {
        for (uint32 i = 0; i < 3; ++i)
            SetPageMaterialPath(l, i, serializationContext->GetScenePath() + archive->GetString(Format("layer%u_lod%u_materialPath", l, i)));
    }

    landscape->GetDecorationData()->Load(archive, serializationContext);
    landscape->LoadFlags(archive, serializationContext);

    LandscapeSubdivision::SubdivisionMetrics metrics;
    metrics.Load(archive);
    landscape->subdivision->SetMetrics(metrics);
}

Landscape* LandscapeComponent::GetLandscape() const
{
    return landscape;
}

void LandscapeComponent::SetLayersCount(uint32 count)
{
    layersPageMaterials.resize(count);
}

uint32 LandscapeComponent::GetLayersCount() const
{
    return uint32(layersPageMaterials.size());
}

void LandscapeComponent::SetPageMaterialPath(uint32 layer, uint32 lod, const FilePath& path)
{
    layersPageMaterials[layer][lod] = GetEngineContext()->assetManager->LoadAsset<Material>(path);
    DVASSERT(layersPageMaterials[layer][lod] != nullptr);
    if (layersPageMaterials[layer][lod] != nullptr)
        landscape->SetPageMaterial(layer, lod, layersPageMaterials[layer][lod]->GetMaterial());
}

const FilePath& LandscapeComponent::GetPageMaterialPath(uint32 layer, uint32 lod) const
{
    return layersPageMaterials[layer][lod]->GetFilepath();
}

void LandscapeComponent::SetHeightmapPath(const FilePath& path)
{
    heightmapPath = path;
    landscape->SetHeightmapPathname(heightmapPath);
}

const FilePath& LandscapeComponent::GetHeighmapPath() const
{
    return heightmapPath;
}

void LandscapeComponent::SetLandscapeMaterialPath(const FilePath& path)
{
    landscapeMaterial = GetEngineContext()->assetManager->LoadAsset<Material>(path);
    DVASSERT(landscapeMaterial != nullptr);
    if (landscapeMaterial != nullptr)
        landscape->SetLandscapeMaterial(landscapeMaterial->GetMaterial());
}

const FilePath& LandscapeComponent::GetLandscapeMaterialPath() const
{
    return landscapeMaterial->GetFilepath();
}

void LandscapeComponent::SetLandscapeSize(float32 size)
{
    landscapeSize = size;
    landscape->SetLandscapeSize(landscapeSize);
}

float32 LandscapeComponent::GetLandscapeSize() const
{
    return landscapeSize;
}

void LandscapeComponent::SetLandscapeHeight(float32 height)
{
    landscapeHeight = height;
    landscape->SetLandscapeHeight(landscapeHeight);
}

float32 LandscapeComponent::GetLandscapeHeight() const
{
    return landscapeHeight;
}

void LandscapeComponent::SetMiddleLODLevel(uint32 level)
{
    middleLODLevel = level;
    landscape->SetMiddleLODLevel(middleLODLevel);
}

uint32 LandscapeComponent::GetMiddleLODLevel() const
{
    return middleLODLevel;
}

void LandscapeComponent::SetMacroLODLevel(uint32 level)
{
    macroLODLevel = level;
    landscape->SetMacroLODLevel(macroLODLevel);
}

uint32 LandscapeComponent::GetMacroLODLevel() const
{
    return macroLODLevel;
}

void LandscapeComponent::SetMaxTexturingLevel(uint32 level)
{
    maxTexturingLevel = level;
    landscape->SetMaxTexturingLevel(maxTexturingLevel);
}

uint32 LandscapeComponent::GetMaxTexturingLevel() const
{
    return maxTexturingLevel;
}

void LandscapeComponent::SetTessellationLevelCount(uint32 levelCount)
{
    tessellationLevelCount = levelCount;
    landscape->SetTessellationLevels(tessellationLevelCount);
}

uint32 LandscapeComponent::GetTessellationLevelCount() const
{
    return tessellationLevelCount;
}

void LandscapeComponent::SetTessellationHeight(float32 height)
{
    tessellationHeight = height;
    landscape->SetTessellationHeight(tessellationHeight);
}

float32 LandscapeComponent::GetTessellationHeight() const
{
    return tessellationHeight;
}
}
