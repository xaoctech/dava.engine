#pragma once

#include <TArc/DataProcessing/SettingsNode.h>
#include <TArc/Qt/QtByteArray.h>

#include <AssetCache/AssetCacheConstants.h>
#include <TextureCompression/TextureConverter.h>

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>
#include <Math/Color.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Components/ActionComponent.h>

namespace DAVA
{
class PropertiesItem;
enum class RenderingBackend
{
    DX11 = 0,
    DX9,
    OpenGL
};

class GeneralSettings : public SettingsNode
{
public:
    bool reloadParticlesOnProjectOpening = true;
    bool previewEnabled = false;
    TextureConverter::eConvertQuality compressionQuality = TextureConverter::ECQ_DEFAULT;
    bool showErrorDialog = true;
    uint32 recentScenesCount = 15;

    // Material Editor settings
    Color materialEditorSwitchColor0 = Color(0.0f, 1.0f, 0.0f, 1.0f);
    Color materialEditorSwitchColor1 = Color(1.0f, 0.0f, 0.0f, 1.0f);
    Color materialEditorLodColor0 = Color(0.9f, 0.9f, 0.9f, 1.0f);
    Color materialEditorLodColor1 = Color(0.7f, 0.7f, 0.7f, 1.0f);
    Color materialEditorLodColor2 = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color materialEditorLodColor3 = Color(0.3f, 0.3f, 0.3f, 1.0f);

    // Particle Editor settings
    float32 particleDebugAlphaTheshold = 0.05f;

    // Lod Editor settings
    Color lodEditorLodColor0 = Color(0.2f, 0.35f, 0.62f, 1.0f);
    Color lodEditorLodColor1 = Color(0.25f, 0.45f, 0.78f, 1.0f);
    Color lodEditorLodColor2 = Color(0.33f, 0.56f, 0.97f, 1.0f);
    Color lodEditorLodColor3 = Color(0.62f, 0.75f, 0.98f, 1.0f);
    Color inactiveColor = Color(0.59f, 0.59f, 0.59f, 1.0f);
    bool fitSliders = false;

    // Height mask tools settings
    Color heightMaskColor0 = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color heightMaskColor1 = Color(0.0f, 0.0f, 0.0f, 1.0f);

    // Asset cache settings
    bool useAssetCache = false;
    String assetCacheIP = "";
    uint16 assetCachePort = AssetCache::ASSET_SERVER_PORT;
    uint32 assetCacheTimeout = 10;

    // Texture browser settings
    bool autoConversion = true;

    // Renderer settings
    RenderingBackend renderBackend = RenderingBackend::OpenGL;
    bool rhiThreadedMode = false;

    // Mouse settings
    bool wheelMoveCamera = true;
    float32 wheelMoveIntensity = 180.0f;
    bool invertWheel = false;

    void Load(const PropertiesItem& settingsNode) override;

    DAVA_VIRTUAL_REFLECTION(GeneralSettings, SettingsNode);
};

class CommonInternalSettings : public SettingsNode
{
public:
    enum MaterialLightViewMode : int32
    {
        VIEW_BASE_COLOR = 1,
        VIEW_ROUGHNESS = 2,
        VIEW_METALLNESS = 4,
        VIEW_AO = 8,
        VIEW_STATIC_SHADOWS = 16,
        VIEW_NORMALS = 32,
        VIEW_TRANSMITTANCE = 64,

        RESOLVE_DIRECT_LIGHT = 128,
        RESOLVE_IBL_LIGHT = 256,
        RESOLVE_AO = 512,
        RESOLVE_TRANSMITTANCE = 1024,
        RESOLVE_BASE_COLOR = 2048,
        RESOLVE_NORMAL_MAP = 4096,
        RESOLVE_DIFFUSE = 8192,
        RESOLVE_SPECULAR = 16384,

        RESOLVE_RESULT = RESOLVE_DIRECT_LIGHT | RESOLVE_IBL_LIGHT | RESOLVE_AO | RESOLVE_TRANSMITTANCE |
        RESOLVE_BASE_COLOR |
        RESOLVE_NORMAL_MAP |
        RESOLVE_DIFFUSE |
        RESOLVE_SPECULAR,

        VIEW_BY_COMPONENT_BIT = 32768,
        VIEW_LIGHTMAP_CANVAS_BIT = 65536,
        VIEW_TEXTURE_MIP_LEVEL_BIT = 131072,

        VIEW_BY_COMPONENT_MASK = 127,
    };

    eGPUFamily textureViewGPU = GPU_ORIGIN;
    eGPUFamily spritesViewGPU = GPU_ORIGIN;
    FilePath cubemapLastFaceDir;
    FilePath cubemapLastProjDir;
    FilePath emitterSaveDir;
    FilePath emitterLoadDir;
    MaterialLightViewMode materialLightViewMode = RESOLVE_RESULT;
    bool materialShowLightmapCanvas = false;
    bool lodEditorSceneMode = false;
    bool lodEditorRecursive = false;
    ActionComponent::Action::eEvent runActionEventType = ActionComponent::Action::EVENT_SWITCH_CHANGED;
    String beastLightmapsDefaultDir = String("lightmaps");
    String imageSplitterPath = String("");
    String imageSplitterPathSpecular = String("");
    bool enableSound = true;
    bool gizmoEnabled = true;
    bool validateMatrices = true;
    bool validateSameNames = true;
    bool validateCollisionProperties = true;
    bool validateTextureRelevance = true;
    bool validateMaterialGroups = true;
    bool validateShowConsole = true;
    QByteArray logWidgetState;

    DAVA_VIRTUAL_REFLECTION(CommonInternalSettings, SettingsNode);
};
} // namespace DAVA
