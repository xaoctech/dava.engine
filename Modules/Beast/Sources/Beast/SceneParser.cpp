#include "Beast/SceneParser.h"
#include "Beast/BeastMaterial.h"
#include "Beast/BeastTexture.h"
#include "Beast/BeastMesh.h"
#include "Beast/BeastMeshInstance.h"
#include "Beast/LandscapeGeometry.h"
#include "Beast/BeastLight.h"
#include "Beast/BeastPointCloud.h"

#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/LightComponent.h>
#include <Scene3D/Entity.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Image/ImageSystem.h>
#include <Render/TextureDescriptor.h>
#include <FileSystem/FileSystem.h>
#include <Logger/Logger.h>

SceneParser::SceneParser(BeastManager* _beastManager, DAVA::Function<void()> parsingCompletedCallback)
    : beastManager(_beastManager)
    , sceneParsingCompleteCallback(parsingCompletedCallback)
{
}

SceneParser::~SceneParser()
{
    ClearScene();
    DVASSERT(false);
    //     if (thumbnailsRequestId != LandscapeThumbnails::InvalidID)
    //     {
    //         LandscapeThumbnails::CancelRequest(thumbnailsRequestId);
    //     }
}

MaxLodMaxSwitch SceneParser::FindMaxLod(DAVA::Entity* node)
{
    maxLodMaxSwitch.maxLodLevel = 0;
    maxLodMaxSwitch.maxSwitchIndex = 0;

    if (beastManager->GetMode() == eBeastMode::MODE_LIGHTMAPS)
    {
        DAVA::ScopedPtr<DAVA::Texture> pinkTexture(DAVA::Texture::CreatePink());
        FindMaxLodRecursive(node, pinkTexture);
    }

    return maxLodMaxSwitch;
}

void SceneParser::FindMaxLodRecursive(DAVA::Entity* node, DAVA::Texture* pinkTexture)
{
    DAVA::int32 nodesCount = node->GetChildrenCount();
    for (DAVA::int32 nodeIndex = 0; nodeIndex < nodesCount; ++nodeIndex)
    {
        DAVA::Entity* child = node->GetChild(nodeIndex);
        DAVA::RenderObject* ro = GetRenderObject(child);
        if (ro)
        {
            DAVA::uint32 count = ro->GetRenderBatchCount();
            for (DAVA::uint32 ri = 0; ri < count; ++ri)
            {
                DAVA::NMaterial* material = ro->GetRenderBatch(ri)->GetMaterial();
                if (IsMaterialTemplateContainString(material, "TextureLightmap"))
                {
                    if (material->HasLocalTexture(DAVA::NMaterialTextureName::TEXTURE_LIGHTMAP))
                        material->SetTexture(DAVA::NMaterialTextureName::TEXTURE_LIGHTMAP, pinkTexture);
                    else
                        material->AddTexture(DAVA::NMaterialTextureName::TEXTURE_LIGHTMAP, pinkTexture);
                }
            }

            maxLodMaxSwitch.maxLodLevel = DAVA::Max(maxLodMaxSwitch.maxLodLevel, ro->GetMaxLodIndex());
            maxLodMaxSwitch.maxSwitchIndex = DAVA::Max(maxLodMaxSwitch.maxSwitchIndex, ro->GetMaxSwitchIndex());
        }

        FindMaxLodRecursive(child, pinkTexture);
    }
}

void SceneParser::StartParsing()
{
    shouldCancel = false;
    entitiesParsed = false;
    landscapeTextureProcessed = false;
    directionalLightEntity = nullptr;
}

void SceneParser::FindAndInitDirectionalLight(DAVA::Entity* node)
{
    DAVA::LightComponent* lightComponent = GetLightComponent(node);
    if (lightComponent != nullptr)
    {
        ParseLightComponent(lightComponent);
    }
    else
    {
        DAVA::int32 nodesCount = node->GetChildrenCount();
        for (DAVA::int32 nodeIndex = 0; nodeIndex < nodesCount; ++nodeIndex)
        {
            FindAndInitDirectionalLight(node->GetChild(nodeIndex));
        }
    }
}

void SceneParser::ValidateDirectionalLight()
{
    if (directionalLightEntity == nullptr)
    {
        DAVA::Logger::Warning("No directional lights found on scene, default light direction will be used");
        mainLightDirection = DAVA::Vector3(0.0f, 0.0f, 1.0f);
    }
}

void SceneParser::ParseNodeRecursive(DAVA::Entity* node, DAVA::uint32 depth)
{
    DAVA::RenderComponent* renderComponent = GetRenderComponent(node);
    if (renderComponent)
    {
        DAVA::RenderObject* rObject = renderComponent->GetRenderObject();
        if (rObject && (rObject->GetType() == DAVA::RenderObject::TYPE_MESH || rObject->GetType() == DAVA::RenderObject::TYPE_SPEED_TREE))
        {
            ParseNodeMaterials(renderComponent);
            ParseNodeMeshes(renderComponent);
        }
    }

    auto landscape = GetLandscape(node);
    if (nullptr != landscape)
    {
        landscapeNode = node;
        ParseLandscapeMaterial(landscape);
    }

    // don't go deeper into lightNode
    if (GetLightComponent(node) == nullptr)
    {
        DAVA::int32 nodesCount = node->GetChildrenCount();
        for (DAVA::int32 nodeIndex = 0; nodeIndex < nodesCount; ++nodeIndex)
        {
            DAVA::Entity* child = node->GetChild(nodeIndex);
            ParseNodeRecursive(child, depth + 1);
        }
    }

    if (depth == 0)
    {
        entitiesParsed = true;
        landscapeTextureProcessed = landscapeTextureProcessed || (landscapeNode == nullptr) || (landscapeMaterial == nullptr);
        CheckIfParsingCompleted();
    }
}

void SceneParser::FinishParsingLandscape()
{
    landscapeTextureProcessed = true;
    CheckIfParsingCompleted();
}

void SceneParser::CheckIfParsingCompleted()
{
    if (entitiesParsed && landscapeTextureProcessed)
    {
        sceneParsingCompleteCallback();
    }
}

void SceneParser::ParseNodeMaterials(DAVA::RenderComponent* renderComponent)
{
    DAVA::RenderObject* ro = renderComponent->GetRenderObject();
    if ((ro == nullptr) || (ro->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE))
    {
        return;
    }

    DAVA::uint32 count = ro->GetRenderBatchCount();
    for (DAVA::uint32 ri = 0; ri < count; ++ri)
    {
        DAVA::NMaterial* material = ro->GetRenderBatch(ri)->GetMaterial();
        if (material == nullptr)
            continue;

        BeastTexture* beastTexture = nullptr;
        DAVA::Texture* diffuseTexture = material->GetEffectiveTexture(DAVA::NMaterialTextureName::TEXTURE_ALBEDO);
        if (diffuseTexture && diffuseTexture->GetDescriptor()->GetSourceTexturePathname().Exists())
        {
            beastTexture = BeastTexture::CreateWithName(BeastTexture::PointerToString(diffuseTexture), beastManager);
            beastTexture->InitWithTexture(diffuseTexture);
        }

        BeastMaterial* beastMaterial = BeastMaterial::CreateWithName(BeastMaterial::PointerToString(material), beastManager);
        beastMaterial->InitWithTextureAndDavaMaterial(beastTexture, material);

#ifdef BEAST_ENABLED_NORMALMAPS
        DAVA::Texture* normalTexture = material->GetTexture(DAVA::NMaterial::TEXTURE_NORMAL);
        if (!normalTexture && material->GetParent())
            normalTexture = material->GetParent()->GetTexture(DAVA::NMaterial::TEXTURE_NORMAL);

        if (normalTexture)
        {
            BeastTexture* beastNormalTexture = BeastTexture::CreateWithName(BeastTexture::PointerToString(normalTexture), beastManager);
            beastNormalTexture->InitWithTexture(normalTexture);
            beastMaterial->AttachNormalMap(beastNormalTexture);
        }
#endif
    }
}

DAVA::Matrix4 SceneParser::BuildLeafTransformMatrix(const DAVA::Matrix4& entityInverseTransform)
{
    DAVA::Vector3 localLightDirection = DAVA::MultiplyVectorMat3x3(mainLightDirection, entityInverseTransform);
    localLightDirection.Normalize();

    DAVA::Vector3 initialOrientation(0.0f, 0.0f, 1.0f);
    DAVA::Vector3 c = initialOrientation.CrossProduct(localLightDirection);
    DAVA::float32 e = initialOrientation.DotProduct(localLightDirection);
    DAVA::float32 h = 1.0f / (1.0f + e);

    DAVA::Matrix4 leafTransformMatrix = DAVA::Matrix4::IDENTITY;
    leafTransformMatrix._data[0][0] = -(e + h * c.x * c.x);
    leafTransformMatrix._data[0][1] = -(h * c.x * c.y - c.z);
    leafTransformMatrix._data[0][2] = h * c.x * c.z + c.y;
    leafTransformMatrix._data[1][0] = -(h * c.x * c.y + c.z);
    leafTransformMatrix._data[1][1] = -(e + h * c.y * c.y);
    leafTransformMatrix._data[1][2] = h * c.y * c.z - c.x;
    leafTransformMatrix._data[2][0] = -(h * c.x * c.z - c.y);
    leafTransformMatrix._data[2][1] = -(h * c.y * c.z + c.x);
    leafTransformMatrix._data[2][2] = e + h * c.z * c.z;
    return leafTransformMatrix;
}

DAVA::int32 SceneParser::FindDetailedSpeedTreeLOD(DAVA::RenderObject* ro) const
{
    DVASSERT(ro->GetType() == DAVA::RenderObject::TYPE_SPEED_TREE);

    DAVA::int32 maxLod = 0;
    DAVA::int32 maxVertexCount = 0;
    DAVA::uint32 totalBatches = ro->GetRenderBatchCount();
    for (DAVA::uint32 b = 0; b < totalBatches; ++b)
    {
        DAVA::int32 batchLod = 0;
        DAVA::int32 batchSwitch = 0;
        auto batch = ro->GetRenderBatch(b, batchLod, batchSwitch);
        auto vertices = batch->GetPolygonGroup()->GetVertexCount();
        if (vertices > maxVertexCount)
        {
            maxVertexCount = vertices;
            maxLod = batchLod;
        }
    }
    return maxLod;
}

void SceneParser::ParseNodeMeshes(DAVA::RenderComponent* renderComponent)
{
    DAVA::RenderObject* ro = renderComponent->GetRenderObject();
    if ((ro == nullptr) || (ro->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE))
    {
        return;
    }

    DAVA::Matrix4 entityInverseTransform;
    const DAVA::Matrix4& entityTransform = renderComponent->GetEntity()->GetWorldTransform();
    entityTransform.GetInverse(entityInverseTransform);

    bool lightmapModeUsed = (beastManager->GetMode() == eBeastMode::MODE_LIGHTMAPS) || (beastManager->GetMode() == eBeastMode::MODE_PREVIEW);
    bool shModeUsed = (beastManager->GetMode() == eBeastMode::MODE_SPHERICAL_HARMONICS);

    bool isSpeedTree = ro->GetType() == DAVA::RenderObject::TYPE_SPEED_TREE;
    if (isSpeedTree && shModeUsed)
    {
        // One BeastPointCloud for scene
        auto name = BeastPointCloud::PointerToString(renderComponent->GetEntity()->GetScene());
        BeastPointCloud* beastPointCloud = BeastPointCloud::CreateWithName(name, beastManager);
        beastPointCloud->AddBakeEntity(renderComponent->GetEntity());
    }
    else
    {
        DAVA::uint32 renderBatchesCount = ro->GetRenderBatchCount();
        DAVA::int32 lodForCurrentSwitch = -1;
        bool useLightMap = false;
        if (isSpeedTree)
        {
            lodForCurrentSwitch = FindDetailedSpeedTreeLOD(ro);
        }
        else
        {
            for (DAVA::uint32 ri = 0; ri < renderBatchesCount; ++ri)
            {
                // find max lod level for selected switch index
                DAVA::int32 rbLodIndex = 0;
                DAVA::int32 rbSwitchIndex = 0;
                DAVA::RenderBatch* batch = ro->GetRenderBatch(ri, rbLodIndex, rbSwitchIndex);

                if ((rbSwitchIndex == -1) || (rbSwitchIndex == switchIndex))
                {
                    lodForCurrentSwitch = DAVA::Max(lodForCurrentSwitch, rbLodIndex);
                }
            }

            useLightMap = (lodLevel <= lodForCurrentSwitch) || (lodForCurrentSwitch == -1);
            if (useLightMap)
            {
                // detect real lod level for current switch.
                // we select last lod if current lodLevel for parcing is greater than last lod for ro
                lodForCurrentSwitch = lodLevel;
            }
        }

        for (DAVA::uint32 ri = 0; ri < renderBatchesCount; ++ri)
        {
            DAVA::int32 rbLodIndex = 0;
            DAVA::int32 rbSwitchIndex = 0;
            DAVA::RenderBatch* batch = ro->GetRenderBatch(ri, rbLodIndex, rbSwitchIndex);
            DAVA::NMaterial* material = batch->GetMaterial();

            if (!DAVA::IsPointerToExactClass<DAVA::RenderBatch>(batch) || (nullptr == material))
                continue;
            if (material->GetEffectiveFlagValue(DAVA::NMaterialFlagName::FLAG_ILLUMINATION_USED) == 0)
                continue;

            DAVA::String fxNameString(material->GetEffectiveFXName().c_str());
            std::transform(fxNameString.begin(), fxNameString.end(), fxNameString.begin(), ::tolower);

            bool skyMaterial = fxNameString.find("skyobject") != DAVA::String::npos;
            bool leafMaterial = fxNameString.find("speedtreeleaf") != DAVA::String::npos;
            bool lightmapMaterial = fxNameString.find("lightmap") != DAVA::String::npos;

            if (skyMaterial || ((rbSwitchIndex != -1) && (rbSwitchIndex != switchIndex)))
                continue;

            if ((rbLodIndex != -1) && (rbLodIndex != lodForCurrentSwitch))
                continue;

            DAVA::String name = ConstructMeshInstancePartName(batch, ri);

            BeastMesh* beastMesh = BeastMesh::CreateWithName(name, beastManager);
            if (leafMaterial)
            {
                beastMesh->InitWithSpeedTreeLeaf(batch, ri, BuildLeafTransformMatrix(entityInverseTransform));
            }
            else
            {
                beastMesh->InitWithMeshInstancePart(batch, ri);
            }

            BeastMeshInstance* beastMeshInstance = BeastMeshInstance::CreateWithName(name, beastManager);
            beastMeshInstance->InitWithRenderBatchAndTransform(batch, ri, entityTransform);
            beastMeshInstance->SetLodLevel(lodLevel);
            if ((beastMeshInstance != nullptr) && useLightMap && lightmapMaterial && lightmapModeUsed)
            {
                const DAVA::float32* lightmapSize = material->GetEffectivePropValue(DAVA::NMaterialParamName::PARAM_LIGHTMAP_SIZE);
                DAVA::float32 targetLightmapSize = (lightmapSize == nullptr) ? DAVA::NMaterial::DEFAULT_LIGHTMAP_SIZE : *lightmapSize;
                beastMeshInstance->SetLightmapSize(static_cast<DAVA::int32>(targetLightmapSize));
                beastMeshInstance->UseLightMap();
            }
        }
    }
}

DAVA::String SceneParser::ConstructMeshInstancePartName(DAVA::RenderBatch* batch, DAVA::int32 index)
{
    return DAVA::Format("%s_%d", BeastMesh::PointerToString(batch).c_str(), index);
}

void SceneParser::ParseLandscapeNode(DAVA::Entity* node, DAVA::Landscape* landscape)
{
}

void SceneParser::ParseLightComponent(DAVA::LightComponent* lightComponent)
{
    bool staticLightEnabled = true;

    DAVA::KeyedArchive* props = DAVA::GetCustomPropertiesArchieve(lightComponent->GetEntity());
    if (props)
    {
        staticLightEnabled = props->GetBool("editor.staticlight.enable", true);
    }

    if (staticLightEnabled)
    {
        auto light = lightComponent->GetLightObject();

        if (light->GetType() == DAVA::Light::TYPE_DIRECTIONAL)
        {
            if (directionalLightEntity == nullptr)
            {
                mainLightDirection = light->GetDirection();
                mainLightDirection.Normalize();
                directionalLightEntity = lightComponent->GetEntity();
            }
            else
            {
                DAVA::Logger::Error("Two or more static lights for Beast found on scene. %s is used",
                                    directionalLightEntity->GetName().c_str());
            }
        }

        BeastLight* beastLight = BeastLight::CreateWithName(BeastLight::PointerToString(light), beastManager);
        beastLight->InitWithLight(lightComponent->GetEntity(), light);
    }
}

void SceneParser::ParseLandscapeMaterial(DAVA::Landscape* landscape)
{
    DAVA::SafeRelease(landscapeMaterial);

    auto texture = landscape->GetMaterial()->GetEffectiveTexture(DAVA::Landscape::TEXTURE_COLOR);
    if ((texture != nullptr) && texture->GetPathname().Exists())
    {
        landscapeMaterial = BeastMaterial::CreateWithName(BeastMaterial::PointerToString(landscape), beastManager);

        DVASSERT(false);
        //        thumbnailsRequestId = LandscapeThumbnails::Create(landscape, DAVA::MakeFunction(this, &SceneParser::OnLandscapeImageCaptured));
    }
    else
    {
        DAVA::Logger::Error("Beast: Can't parse Landscape color texture!");
        landscapeNode = nullptr;
        shouldCancel = true;
        FinishParsingLandscape();
    }
}

void SceneParser::OnLandscapeImageCaptured(DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture)
{
    DAVA::ScopedPtr<DAVA::Image> image(landscapeTexture->CreateImageFromMemory());
    DAVA::ImageSystem::Save(GetLandscapeTemporaryFileName(), image);

    BeastTexture* beastTexture = BeastTexture::CreateWithName(BeastTexture::PointerToString(landscapeTexture), beastManager);
    beastTexture->InitWithFile(GetLandscapeTemporaryFileName());
    landscapeMaterial->InitWithTextureAndDavaMaterial(beastTexture, nullptr);

    ContinueParsingLandscape(landscapeNode, landscape);
}

void SceneParser::ContinueParsingLandscape(DAVA::Entity* node, DAVA::Landscape* landscape)
{
    auto davaLandscapeMaterial = landscape->GetRenderBatch(0)->GetMaterial();
    if (davaLandscapeMaterial->GetEffectiveFlagValue(DAVA::NMaterialFlagName::FLAG_ILLUMINATION_USED) == 0)
    {
        FinishParsingLandscape();
        return;
    }

    LandscapeGeometry geometry;
    if (landscape->GetLevel0Geometry(geometry.vertices, geometry.indices) == false)
    {
        FinishParsingLandscape();
        return;
    }
    geometry.ComputeNormals();

    auto name = BeastMeshInstance::PointerToString(landscape);
    BeastMesh* beastMesh = BeastMesh::CreateWithName(name, beastManager);
    beastMesh->InitWithLandscape(&geometry, landscapeMaterial);

    BeastMeshInstance* beastMeshInstance = BeastMeshInstance::CreateWithName(name, beastManager);
    beastMeshInstance->InitWithLandscape(landscape, beastMesh);
    beastMeshInstance->SetLodLevel(0);

    if (beastManager->GetMode() == eBeastMode::MODE_LIGHTMAPS || beastManager->GetMode() == eBeastMode::MODE_PREVIEW)
    {
        const DAVA::float32* lightmapSize = davaLandscapeMaterial->GetEffectivePropValue(DAVA::NMaterialParamName::PARAM_LIGHTMAP_SIZE);
        DAVA::float32 targetLightmapSize = (lightmapSize == nullptr) ? DAVA::NMaterial::DEFAULT_LIGHTMAP_SIZE : *lightmapSize;
        beastMeshInstance->SetLightmapSize(static_cast<DAVA::int32>(targetLightmapSize));
        beastMeshInstance->UseLightMap();
    }

    FinishParsingLandscape();
}

void SceneParser::ClearScene()
{
    BeastTexture::ReleaseResources();
    BeastMaterial::ReleaseResources();
    BeastMesh::ReleaseResources();
    BeastMeshInstance::ReleaseResources();
    BeastLight::ReleaseResources();
    BeastPointCloud::ReleaseResources();

    shouldCancel = false;
    entitiesParsed = false;
    landscapeTextureProcessed = false;
    landscapeMaterial = nullptr;
    landscapeNode = nullptr;
}

void SceneParser::SetLodLevel(DAVA::int32 _lodLevel)
{
    lodLevel = _lodLevel;
}

void SceneParser::SetSwitchIndex(DAVA::int32 _switchIndex)
{
    switchIndex = _switchIndex;
}

bool SceneParser::ShouldCancel() const
{
    return shouldCancel;
}

bool SceneParser::IsMaterialTemplateContainString(DAVA::NMaterial* material, const DAVA::String& _str)
{
    if (material)
    {
        DAVA::String str(_str);
        DAVA::String fxNameString(material->GetEffectiveFXName().c_str());
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        std::transform(fxNameString.begin(), fxNameString.end(), fxNameString.begin(), ::tolower);
        return fxNameString.find(str) != DAVA::String::npos;
    }
    return false;
}

DAVA::FilePath SceneParser::GetTemporaryFolder()
{
    return DAVA::FileSystem::Instance()->GetCurrentWorkingDirectory() + "temp_beast/";
}

DAVA::FilePath SceneParser::GetLandscapeTemporaryFileName()
{
    return GetTemporaryFolder() + "temp_landscape_color.png";
}
