#include "SceneFileConverter.h"

#include "Base/Any.h"
#include "Asset/AssetManager.h"
#include "Base/UnordererMap.h"
#include "Engine/Engine.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/Prefab.h"
#include "Scene3D/Scene.h"
#include "Scene3D/AssetLoaders/PrefabAssetLoader.h"
#include "Scene3D/AssetLoaders/GeometryAssetLoader.h"
#include "Scene3D/AssetLoaders/MaterialAssetLoader.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/LandscapeComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/LightmapComponent.h"
#include "Scene3D/Components/LightmapDataComponent.h"
#include "Render/3D/Geometry.h"
#include "Render/Material/Material.h"
#include "Render/Highlevel/MeshLODDescriptor.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/SpeedTreeObject.h"

namespace DAVA
{
namespace SceneFileConverterDetails
{
static const FastName PARAM_LIGHTMAP_SIZE("lightmapSize");
static const FastName FLAG_ILLUMINATION_SHADOW_CASTER("ILLUMINATION_SHADOW_CASTER");
static const FastName FLAG_ILLUMINATION_SHADOW_RECEIVER("ILLUMINATION_SHADOW_RECEIVER");
static const FastName TEXTURE_SHADOW_AO("shadowaotexture");
};

void SceneFileConverter::ConvertSceneToLevelFormat(Scene* scene, const FilePath& rootFolder)
{
    scene->SetGlobalMaterial(nullptr);

    FileSystem::Instance()->CreateDirectory(rootFolder, true);

    SceneFileConverter::ConvertRenderComponentsRecursive(scene, rootFolder);

    AssetManager* assetManager = GetEngineContext()->assetManager;

    int32 childrenCount = scene->GetChildrenCount();
    for (int32 childIndex = 0; childIndex < childrenCount; ++childIndex)
    {
        Entity* child = scene->GetChild(childIndex);

        if (child->GetComponent<PrefabComponent>() != nullptr)
            continue;

        KeyedArchive* props = GetCustomPropertiesArchieve(child);
        if (props != nullptr && props->IsKeyExists("editor.referenceToOwner"))
        {
            FilePath ownerPath = FilePath(props->GetString("editor.referenceToOwner"));
            props->DeleteKey("editor.referenceToOwner");

            FilePath prefabPath = FilePath::CreateWithNewExtension(ownerPath, ".prefab");
            PrefabAssetLoader::PathKey assetKey(prefabPath);

            Matrix4 prefabLocalTransform = child->GetLocalTransform();

            Asset<Prefab> prefab = assetManager->FindAsset<Prefab>(assetKey);
            if (prefab == nullptr)
            {
                FileSystem::Instance()->CreateDirectory(prefabPath.GetDirectory(), true);

                child->SetLocalTransform(Matrix4::IDENTITY);
                prefab = assetManager->CreateAsset<Prefab>(assetKey);

                ScopedPtr<Entity> childClone(child->Clone());
                prefab->ConstructFrom(childClone);
                assetManager->SaveAsset(prefab);
            }

            Entity* prefabEntity = new Entity();
            PrefabComponent* prefabComponent = new PrefabComponent();
            prefabComponent->SetFilepath(prefabPath);
            prefabEntity->AddComponent(prefabComponent);
            prefabEntity->SetName(prefabPath.GetFilename().c_str());
            prefabEntity->SetLocalTransform(prefabLocalTransform);

            scene->InsertBeforeNode(prefabEntity, child);
            scene->RemoveNode(child);
        }
    }
}

void SceneFileConverter::ConvertRenderComponentsRecursive(Entity* entity, const FilePath& rootFolder)
{
    DVASSERT(rootFolder.IsDirectoryPathname());
    DVASSERT(entity->GetScene()->GetGlobalMaterial() == nullptr);

    int32 childrenCount = entity->GetChildrenCount();
    for (int32 childIndex = 0; childIndex < childrenCount; ++childIndex)
        ConvertRenderComponentsRecursive(entity->GetChild(childIndex), rootFolder);

    FilePath assetsPath = FindAssetsFolder(entity);
    if (assetsPath.IsEmpty())
        assetsPath = rootFolder + "assets/";

    FileSystem::Instance()->CreateDirectory(assetsPath, true);

    RenderComponent* renderComponent = GetRenderComponent(entity);
    if (renderComponent != nullptr)
    {
        bool converted = false;

        RenderObject* renderObject = renderComponent->GetRenderObject();
        switch (renderObject->GetType())
        {
        case RenderObject::TYPE_MESH:
        case RenderObject::TYPE_SKINNED_MESH:
        case RenderObject::TYPE_SPEED_TREE:
            converted = ConvertMesh(static_cast<Mesh*>(renderObject), entity, assetsPath);
            break;

        case RenderObject::TYPE_LANDSCAPE:
            converted = ConvertLandscape(static_cast<Landscape*>(renderObject), entity, assetsPath);

        default:
            break;
        }

        if (converted)
            entity->RemoveComponent(renderComponent);
    }
}

bool SceneFileConverter::ConvertMesh(Mesh* mesh, Entity* entity, const FilePath& assetsPath)
{
    AssetManager* assetManager = GetEngineContext()->assetManager;

    int32 lodCount = mesh->GetMaxLodIndex() + 1;

    Vector<MeshLODDescriptor> lodDescriptors(Max(1, lodCount));
    if (lodCount == 0)
    {
        lodDescriptors[0].geometryPath = assetsPath + Format("%s.geo", entity->GetName().c_str());
        lodDescriptors[0].geometryAsset = assetManager->CreateAsset<Geometry>(GeometryAssetLoader::PathKey(lodDescriptors[0].geometryPath));
    }
    else
    {
        for (int32 l = 0; l < lodCount; ++l)
        {
            lodDescriptors[l].lodIndex = l;
            lodDescriptors[l].geometryPath = assetsPath + Format("%s_lod%d.geo", entity->GetName().c_str(), l);
            lodDescriptors[l].geometryAsset = assetManager->CreateAsset<Geometry>(GeometryAssetLoader::PathKey(lodDescriptors[l].geometryPath));
        }
    }

    LightmapComponent* lightmapComponent = new LightmapComponent();
    bool hasLightmapInfo = false;

    uint32 batchCount = mesh->GetRenderBatchCount();
    lightmapComponent->SetParamsCount(mesh->GetRenderBatchCount());

    int32 lodIndex = -1, switchIndex = -1;
    for (uint32 b = 0; b < batchCount; ++b)
    {
        RenderBatch* batch = mesh->GetRenderBatch(b, lodIndex, switchIndex);
        NMaterial* material = batch->GetMaterial();

        //////////////////////////////////////////////////////////////////////////
        //lightmaps
        LightmapComponent::LightmapParam& lightmapParam = lightmapComponent->GetLightmapParam(b);

        lightmapParam.SetCastShadow(material->GetEffectiveFlagValue(SceneFileConverterDetails::FLAG_ILLUMINATION_SHADOW_CASTER) != 0);
        lightmapParam.SetReceiveShadow(material->GetEffectiveFlagValue(SceneFileConverterDetails::FLAG_ILLUMINATION_SHADOW_RECEIVER) != 0);

        const float32* lighmapSize = material->GetEffectivePropValue(SceneFileConverterDetails::PARAM_LIGHTMAP_SIZE);
        if (lighmapSize != nullptr)
            lightmapParam.SetLightmapSize(uint32(*lighmapSize));

        hasLightmapInfo |= (lightmapParam.IsReceiveShadow() || lightmapParam.IsCastShadow() || lighmapSize != nullptr);

        //////////////////////////////////////////////////////////////////////////

        Asset<Material> materialAsset = CreateMaterialAsset(material, assetsPath);

        MeshLODDescriptor& lodDesc = lodDescriptors[Max(0, lodIndex)];

        MeshBatchDescriptor batchDesc;
        batchDesc.switchIndex = switchIndex;

        batchDesc.materialAsset = materialAsset;
        batchDesc.materialPath = FilePath(GetEngineContext()->assetManager->GetAssetFileInfo(materialAsset).fileName);

        batchDesc.geometryIndex = lodDesc.geometryAsset->GetPolygonGroupCount();
        lodDesc.geometryAsset->AddPolygonGroup(batch->GetPolygonGroup());

        batchDesc.jointTargets = mesh->GetJointTargets(batch);

        lodDesc.batchDescriptors.emplace_back(batchDesc);
    }

    for (const MeshLODDescriptor& desc : lodDescriptors)
    {
        assetManager->SaveAsset(desc.geometryAsset);
    }

    if (mesh->GetType() == RenderObject::TYPE_SPEED_TREE)
    {
        SpeedTreeComponent* speedTreeComponent = entity->GetComponent<SpeedTreeComponent>();
        DVASSERT(speedTreeComponent != nullptr);

        speedTreeComponent->SetMeshDescriptor(lodDescriptors);
        speedTreeComponent->GetSpeedTreeObject()->SetFlags(mesh->GetFlags());
        speedTreeComponent->GetSpeedTreeObject()->SetStaticOcclusionIndex(mesh->GetStaticOcclusionIndex());
    }
    else
    {
        MeshComponent* meshComponent = new MeshComponent();
        meshComponent->SetMeshDescriptor(lodDescriptors);
        meshComponent->GetMesh()->SetFlags(mesh->GetFlags());
        meshComponent->GetMesh()->SetStaticOcclusionIndex(mesh->GetStaticOcclusionIndex());

        entity->AddComponent(meshComponent);
    }

    if (hasLightmapInfo)
        entity->AddComponent(lightmapComponent);
    else
        SafeDelete(lightmapComponent);

    return true;
}

bool SceneFileConverter::ConvertLandscape(Landscape* landscape, Entity* entity, const FilePath& assetsPath)
{
    LandscapeComponent* landscapeComponent = new LandscapeComponent();

    landscapeComponent->SetHeightmapPath(landscape->GetHeightmapPathname());
    landscapeComponent->SetLandscapeSize(landscape->GetLandscapeSize());
    landscapeComponent->SetLandscapeHeight(landscape->GetLandscapeHeight());
    landscapeComponent->SetMiddleLODLevel(landscape->GetMiddleLODLevel());
    landscapeComponent->SetMacroLODLevel(landscape->GetMacroLODLevel());
    landscapeComponent->SetMaxTexturingLevel(landscape->GetMaxTexturingLevel());
    landscapeComponent->SetTessellationLevelCount(landscape->GetTessellationLevels());
    landscapeComponent->SetTessellationHeight(landscape->GetTessellationHeight());
    landscapeComponent->GetLandscape()->GetSubdivision()->SetMetrics(landscape->GetSubdivision()->GetMetrics());

    DecorationData* decoration = landscapeComponent->GetLandscape()->GetDecorationData();
    decoration->CopyParameters(landscape->GetDecorationData());
    decoration->SetDecorationPath(ConvertDecoration(decoration->GetDecorationPath()));

    AssetManager* assetManager = GetEngineContext()->assetManager;
    Asset<Material> landscapeMaterialAsset = CreateMaterialAsset(landscape->GetLandscapeMaterial(), assetsPath);
    AssetFileInfo fileInfo = assetManager->GetAssetFileInfo(landscapeMaterialAsset);
    landscapeComponent->SetLandscapeMaterialPath(FilePath(fileInfo.fileName));

    const float* lightmapSizeProperty = landscape->GetLandscapeMaterial()->GetEffectivePropValue(SceneFileConverterDetails::PARAM_LIGHTMAP_SIZE);

    landscapeComponent->SetLayersCount(landscape->GetLayersCount());
    for (uint32 l = 0; l < landscapeComponent->GetLayersCount(); ++l)
    {
        for (uint32 i = 0; i < 3; ++i)
        {
            Asset<Material> pageMaterialAsset = CreateMaterialAsset(landscape->GetPageMaterials(l, i), assetsPath);
            AssetFileInfo fileInfo = assetManager->GetAssetFileInfo(pageMaterialAsset);
            landscapeComponent->SetPageMaterialPath(l, i, FilePath(fileInfo.fileName));

            if (lightmapSizeProperty == nullptr)
                lightmapSizeProperty = landscape->GetPageMaterials(l, i)->GetEffectivePropValue(SceneFileConverterDetails::PARAM_LIGHTMAP_SIZE);
        }
    }

    landscapeComponent->GetLandscape()->SetFlags(landscape->GetFlags());
    landscapeComponent->GetLandscape()->SetStaticOcclusionIndex(landscape->GetStaticOcclusionIndex());

    entity->AddComponent(landscapeComponent);

    //////////////////////////////////////////////////////////////////////////
    //lightmap
    LightmapComponent* lightmapComponent = new LightmapComponent();
    lightmapComponent->SetParamsCount(1);
    lightmapComponent->GetLightmapParam(0).SetCastShadow(true);
    lightmapComponent->GetLightmapParam(0).SetReceiveShadow(true);
    if (lightmapSizeProperty != nullptr)
        lightmapComponent->GetLightmapParam(0).SetLightmapSize(uint32(*lightmapSizeProperty));

    //////////////////////////////////////////////////////////////////////////

    entity->AddComponent(lightmapComponent);

    LightmapDataComponent* lightmapDataComponent = new LightmapDataComponent();
    entity->AddComponent(lightmapDataComponent);

    Texture* lightmapTexture = landscape->GetLandscapeMaterial()->GetEffectiveTexture(SceneFileConverterDetails::TEXTURE_SHADOW_AO);
    if (lightmapTexture != nullptr && lightmapTexture->GetPathname().Exists())
    {
        lightmapDataComponent->RebuildIDs();
        const FastName& lightmapID = lightmapDataComponent->GetLightmapID(landscapeComponent->GetLandscape());
        lightmapDataComponent->SetLightmapData(lightmapID, Vector4(0.f, 0.f, 1.f, 1.f), lightmapTexture->GetPathname());
    }

    return true;
}

FilePath SceneFileConverter::ConvertDecoration(const FilePath& decorationPath)
{
    FilePath prefabPath;
    if (!decorationPath.IsEmpty() && decorationPath.IsEqualToExtension(".sc2"))
    {
        ScopedPtr<Scene> scene(new Scene(Scene::SceneSystemsPolicy::WithoutSystems));
        if (scene->LoadScene(decorationPath) == SceneFileV2::ERROR_NO_ERROR)
        {
            SceneFileConverter::ConvertRenderComponentsRecursive(scene, decorationPath.GetDirectory());

            prefabPath = FilePath::CreateWithNewExtension(decorationPath, ".prefab");
            PrefabAssetLoader::PathKey assetKey(prefabPath);
            AssetManager* assetManager = GetEngineContext()->assetManager;
            Asset<Prefab> decorationPrefab = assetManager->FindAsset<Prefab>(assetKey);
            if (decorationPrefab == nullptr)
            {
                decorationPrefab = GetEngineContext()->assetManager->CreateAsset<Prefab>(assetKey);
            }

            decorationPrefab->ConstructFrom(scene);
            assetManager->SaveAsset(decorationPrefab);
        }
    }

    return prefabPath;
}

FilePath SceneFileConverter::FindAssetsFolder(Entity* forEntity)
{
    while (forEntity != nullptr)
    {
        KeyedArchive* props = GetCustomPropertiesArchieve(forEntity);
        if (props != nullptr && props->IsKeyExists("editor.referenceToOwner"))
        {
            FilePath ownerPath = FilePath(props->GetString("editor.referenceToOwner"));
            return ownerPath.GetDirectory() + "assets/";
        }

        forEntity = forEntity->GetParent();
    }

    return FilePath();
}

Asset<Material> SceneFileConverter::CreateMaterialAsset(NMaterial* material, const FilePath& pathRoot)
{
    DVASSERT(material != nullptr);
    AssetManager* assetManager = GetEngineContext()->assetManager;

    Vector<NMaterial*> materialHierarchyCopy;
    while (material != nullptr)
    {
        NMaterial* materialCopy = material->Clone(GetMaterialType(material));
        materialCopy->SetParent(nullptr);

        bool removeUV = (material->GetEffectiveFlagValue(NMaterialFlagName::FLAG_USE_BAKED_LIGHTING) != 0); //uv scale/offset can be used not only for lightmaps
        RemoveRedundantProps(materialCopy, removeUV);

        NMaterial* parent = material->GetParent();
        bool skipMaterial = CompareMaterialLocalProps(materialCopy, parent);

        if (parent != nullptr && parent->GetChildren().size() == 1u)
        {
            CopyMaterialLocalProps(materialCopy, parent);
            skipMaterial = true;
        }

        if (skipMaterial)
            materialCopy->Release();
        else
            materialHierarchyCopy.push_back(materialCopy);

        material = parent;
    }

    String materialFileName;
    Asset<Material> result = nullptr;
    while (!materialHierarchyCopy.empty())
    {
        if (!materialFileName.empty())
            materialFileName.push_back('_');
        materialFileName += String(materialHierarchyCopy.back()->GetMaterialName().c_str());

        FilePath materialPath = pathRoot + materialFileName + ".mat";
        Asset<Material> materialAsset = assetManager->FindAsset<Material>(MaterialAssetLoader::PathKey(materialPath));
        if (materialAsset == nullptr)
        {
            materialAsset = assetManager->CreateAsset<Material>(MaterialAssetLoader::PathKey(materialPath));

            NMaterial* materialCopy = materialHierarchyCopy.back();
            materialAsset->SetMaterial(materialCopy);

            if (result != nullptr)
            {
                AssetFileInfo fileInfo = assetManager->GetAssetFileInfo(result);
                materialAsset->SetParentPath(FilePath(fileInfo.fileName));
            }

            assetManager->SaveAsset(materialAsset);
        }
        result = materialAsset;

        materialHierarchyCopy.back()->Release();
        materialHierarchyCopy.pop_back();
    }

    return result;
}

void SceneFileConverter::RemoveRedundantProps(NMaterial* material, bool removeUVScaleOffset)
{
    if (removeUVScaleOffset)
    {
        if (material->HasLocalProperty(NMaterialParamName::PARAM_UV_OFFSET))
            material->RemoveProperty(NMaterialParamName::PARAM_UV_OFFSET);

        if (material->HasLocalProperty(NMaterialParamName::PARAM_UV_SCALE))
            material->RemoveProperty(NMaterialParamName::PARAM_UV_SCALE);
    }

    //remove redundant props
    const static FastName MATERIAL_REDUNDANT_FLAGS[] = {
        SceneFileConverterDetails::FLAG_ILLUMINATION_SHADOW_CASTER,
        SceneFileConverterDetails::FLAG_ILLUMINATION_SHADOW_RECEIVER,
        FastName("ILLUMINATION_USED"),
        FastName("WIND_ANIMATION"),
    };
    const static FastName MATERIAL_REDUNDANT_PARAMS[] = {
        SceneFileConverterDetails::PARAM_LIGHTMAP_SIZE,
        FastName("lightmapTextureSize"),
        FastName("capturePositionInWorldSpace"),
        FastName("captureWorldToLocalMatrix"),
        FastName("captureSize"),
    };
    const static FastName MATERIAL_REDUNDANT_TEXTURES[] = {
        SceneFileConverterDetails::TEXTURE_SHADOW_AO,
        FastName("indirecttexture"),
    };

    for (const FastName& flag : MATERIAL_REDUNDANT_FLAGS)
    {
        if (material->HasLocalFlag(flag))
            material->RemoveFlag(flag);
    }

    for (const FastName& param : MATERIAL_REDUNDANT_PARAMS)
    {
        if (material->HasLocalProperty(param))
            material->RemoveProperty(param);
    }

    for (const FastName& texture : MATERIAL_REDUNDANT_TEXTURES)
    {
        if (material->HasLocalTexture(texture))
            material->RemoveTexture(texture);
    }
}

void SceneFileConverter::CopyMaterialLocalProps(NMaterial* from, NMaterial* to)
{
    for (auto& flag : from->GetLocalFlags())
    {
        if (to->HasLocalFlag(flag.first))
            to->SetFlag(flag.first, flag.second);
        else
            to->AddFlag(flag.first, flag.second);
    }
    for (auto& prop : from->GetLocalProperties())
    {
        if (to->HasLocalProperty(prop.first))
            to->SetPropertyValue(prop.first, prop.second->data.get());
        else
            to->AddProperty(prop.first, prop.second->data.get(), prop.second->type, prop.second->arraySize);
    }
    for (auto& texture : from->GetLocalTextures())
    {
        if (to->HasLocalTexture(texture.first))
            to->SetTexture(texture.first, texture.second->texture);
        else
            to->AddTexture(texture.first, texture.second->texture);
    }
}

bool SceneFileConverter::CompareMaterialLocalProps(NMaterial* material, NMaterial* referenceMaterial)
{
    if (material == nullptr || referenceMaterial == nullptr)
        return false;

    for (auto& flag : material->GetLocalFlags())
    {
        if (!referenceMaterial->HasLocalFlag(flag.first) || referenceMaterial->GetLocalFlagValue(flag.first) != flag.second)
            return false;
    }

    for (auto& property : material->GetLocalProperties())
    {
        if (!referenceMaterial->HasLocalProperty(property.first))
            return false;

        NMaterialProperty* prop = property.second;
        NMaterialProperty* refProp = referenceMaterial->GetLocalProperties().at(property.first);

        if (prop->type != refProp->type)
            return false;

        if (prop->arraySize != refProp->arraySize)
            return false;

        if (Memcmp(prop->data.get(), refProp->data.get(), ShaderDescriptor::CalculateDataSize(prop->type, prop->arraySize)) != 0)
            return false;
    }

    for (auto& texture : material->GetLocalTextures())
    {
        if (!referenceMaterial->HasLocalTexture(texture.first) || referenceMaterial->GetLocalTexture(texture.first)->GetPathname() != texture.second->texture->GetPathname())
            return false;
    }

    return true;
}

NMaterial::eType SceneFileConverter::GetMaterialType(NMaterial* material)
{
    DVASSERT(material != nullptr);

    static const UnorderedMap<FastName, NMaterial::eType> MATERIAL_TYPE_MAP = {
        { NMaterialName::LANDSCAPE, NMaterial::TYPE_LANDSCAPE },
        { NMaterialName::TILE_MASK, NMaterial::TYPE_LANDSCAPE },

        { NMaterialName::FORWARD_PBS, NMaterial::TYPE_COMMON },
        { NMaterialName::FORWARD_PBS_TRANSMITTANCE, NMaterial::TYPE_COMMON },
        { NMaterialName::FORWARD_PBS_VERTEX_AO, NMaterial::TYPE_COMMON },
        { NMaterialName::FORWARD_PBS_SPEEDTREE_TRANSMITTANCE, NMaterial::TYPE_COMMON },
        { NMaterialName::FORWARD_PBS_MULTITEXTURED, NMaterial::TYPE_COMMON },
        { NMaterialName::FORWARD_PBS_DETAIL_NORMAL, NMaterial::TYPE_COMMON },
    };

    if (material->GetMaterialType() == NMaterial::TYPE_LEGACY)
    {
        const FastName& fxName = material->GetEffectiveFXName();
        if (!fxName.empty())
        {
            auto found = MATERIAL_TYPE_MAP.find(fxName);
            if (found != MATERIAL_TYPE_MAP.end())
                return found->second;

            if (strstr(fxName.c_str(), "deferreddecal"))
                return NMaterial::TYPE_DECAL;

            if (strstr(fxName.c_str(), "vt-decal"))
                return NMaterial::TYPE_DECAL_VT;
        }
    }

    return material->GetMaterialType();
}
}
