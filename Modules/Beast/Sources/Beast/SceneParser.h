#pragma once

#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include <FileSystem/FilePath.h>
#include <Math/Vector.h>
#include <Math/Matrix4.h>
#include <Render/Highlevel/LandscapeThumbnails.h>

namespace DAVA
{
class Entity;
class RenderBatch;
class RenderObject;
class RenderComponent;
class NMaterial;
class Landscape;
class Texture;
class LightComponent;
}

namespace Beast
{
struct MaxLodMaxSwitch
{
    DAVA::int32 maxLodLevel = 0;
    DAVA::int32 maxSwitchIndex = 0;
};

class BeastMaterial;
class BeastManager;
class SceneParser
{
public:
    SceneParser(BeastManager* beastManager, DAVA::Function<void()> parsingCompletedCallback);
    ~SceneParser();

    MaxLodMaxSwitch FindMaxLod(DAVA::Entity* node);

    void StartParsing();

    void FindAndInitDirectionalLight(DAVA::Entity* node);
    void ValidateDirectionalLight();
    void ParseNodeRecursive(DAVA::Entity* node, DAVA::uint32 depth = 0);
    void ClearScene();
    void SetLodLevel(DAVA::int32 _lodLevel);
    void SetSwitchIndex(DAVA::int32 _switchIndex);

    bool ShouldCancel() const;

    static DAVA::String ConstructMeshInstancePartName(DAVA::RenderBatch* batch, DAVA::int32 index);
    static bool IsMaterialTemplateContainString(DAVA::NMaterial* material, const DAVA::String& str);
    static DAVA::FilePath GetTemporaryFolder();
    static DAVA::FilePath GetLandscapeTemporaryFileName();

private:
    void ParseNodeMaterials(DAVA::RenderComponent* renderComponent);
    void ParseNodeMeshes(DAVA::RenderComponent* renderComponent);
    void ParseLandscapeNode(DAVA::Entity* node, DAVA::Landscape* landscape);
    void ParseLightComponent(DAVA::LightComponent* lightComponent);
    void FindMaxLodRecursive(DAVA::Entity* node, DAVA::Texture* pinkTexture);

    void ParseLandscapeMaterial(DAVA::Landscape* node);
    void OnLandscapeImageCaptured(DAVA::Landscape* landscape, DAVA::Texture* texture);
    void ContinueParsingLandscape(DAVA::Entity* node, DAVA::Landscape* landscape);

    void FinishParsingLandscape();
    void CheckIfParsingCompleted();

    DAVA::int32 FindDetailedSpeedTreeLOD(DAVA::RenderObject*) const;
    DAVA::Matrix4 BuildLeafTransformMatrix(const DAVA::Matrix4& entityInverseTransform);

private:
    BeastManager* beastManager = nullptr;
    BeastMaterial* landscapeMaterial = nullptr;
    MaxLodMaxSwitch maxLodMaxSwitch;
    DAVA::Entity* landscapeNode = nullptr;
    DAVA::Entity* directionalLightEntity = nullptr;
    DAVA::Function<void()> sceneParsingCompleteCallback;
    DAVA::int32 lodLevel = 0;
    DAVA::int32 switchIndex = 0;
    DAVA::Vector3 mainLightDirection;
    bool landscapeTextureProcessed = false;
    bool entitiesParsed = false;
    bool shouldCancel = false;

    DAVA::LandscapeThumbnails::RequestID thumbnailsRequestId = DAVA::LandscapeThumbnails::InvalidID;
};
}