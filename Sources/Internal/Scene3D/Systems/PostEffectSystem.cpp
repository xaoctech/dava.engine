#include "Scene3D/Systems/PostEffectSystem.h"

#include "Entity/ComponentUtils.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/PostEffectComponent.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/PostEffectRenderer.h"
#include "Render/Image/ImageSystem.h"
#include "Render/TextureDescriptor.h"
#include "Render/Texture.h"
#include "Logger/Logger.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Asset/AssetManager.h"

namespace DAVA
{
PostEffectSystem::PostEffectSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<PostEffectComponent>())
{
    float targetEv = GetEVFromCameraSettings(100.0f, 16.0f, 100.0f);
    currentTargetLuminance = GetLuminanceFromEV(targetEv);
    currentDynamicRange = GetLuminanceRangeFromEVRange(targetEv, Vector2(3.0f, 3.0f));

    if (scene)
    {
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::POSTEFFECT_CHANGED);
    }
}

void PostEffectSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (component->GetType() == Type::Instance<PostEffectComponent>() && event == EventSystem::POSTEFFECT_CHANGED)
    {
        PostEffectComponent* posteffect = static_cast<PostEffectComponent*>(component);
        PostEffectRenderer* renderer = GetScene()->GetRenderSystem()->GetPostEffectRenderer();
        PostEffectRenderer::Settings* settings = renderer->GetSettings();

        float32 cameraSettingsEv = GetEVFromCameraSettings(posteffect->GetShutterTimeInv(), posteffect->GetAperture(), posteffect->GetISO());
        float32 manualEv = posteffect->GetBaseEV();
        float32 targetEv = posteffect->GetUseEv() ? manualEv : cameraSettingsEv;

        currentTargetLuminance = GetLuminanceFromEV(targetEv);
        currentDynamicRange = GetLuminanceRangeFromEVRange(targetEv, posteffect->GetDynamicRange());

        settings->adaptationRange = GetLuminanceRangeFromEVRange(targetEv, posteffect->GetAdaptationRange());
        settings->adaptationSpeed = posteffect->GetAdaptationSpeed();
        settings->enableColorGrading = posteffect->GetColorGrading();
        settings->enableHeatMap = posteffect->GetHeapMapEnabled();
        settings->enableToneMapping = posteffect->GetToneMapping();
        settings->bloomColor = posteffect->GetBloomColor();
        settings->bloomEVC = posteffect->GetBloomEVC();
        settings->resetHistory = posteffect->GetResetHistory();

        posteffect->SetResetHistory(false);

        AssetManager* assetManager = GetEngineContext()->assetManager;

        if (posteffect->GetColorGradingTable().IsEmpty())
        {
            Texture::PathKey key("~res:/Textures/colorgrading.png");
            settings->colorGradingTable = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
        }
        else
        {
            Texture::PathKey key(posteffect->GetColorGradingTable());
            settings->colorGradingTable = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
        }
        posteffect->SetColorGradingTexture(settings->colorGradingTable);

        if (posteffect->GetHeatmapTable().IsEmpty())
        {
            Texture::PathKey key("~res:/Textures/heatmap.png");
            settings->heatmapTable = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
        }
        else
        {
            Texture::PathKey key(posteffect->GetHeatmapTable());
            settings->heatmapTable = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
        }
        posteffect->SetHeatmapTexture(settings->heatmapTable);

        FilePath lightMeterTablePath = posteffect->GetLightMeterTable();
        if (posteffect->GetLightMeterTable().IsEmpty())
            lightMeterTablePath = "~res:/Textures/lightmeter.tex";

        TextureDescriptor* desc = TextureDescriptor::CreateFromFile(lightMeterTablePath);
        ScopedPtr<Image> mask;
        if (desc)
        {
            mask = ScopedPtr<Image>(ImageSystem::LoadSingleMip(desc->GetSourceTexturePathname()));
        }
        if (mask)
        {
            uint8* ptr = mask->GetData();
            uint32 stride = 1;

            switch (mask->GetPixelFormat())
            {
            case PixelFormat::FORMAT_A8:
            {
                stride = 1;
                break;
            }
            case PixelFormat::FORMAT_RGBA8888:
            {
                stride = 4;
                break;
            }
            default:
            {
                Logger::Error("Invalid format for light meter mask. Should be R8 (grayscale) or RGBA8");
                break;
            }
            }

            uint64 totalWeight = mask->GetWidth() * mask->GetHeight() * 255;
            uint64 pixelWeight = 0;
            for (uint32 y = 0; y < mask->GetHeight(); ++y)
            {
                for (uint32 x = 0; x < mask->GetWidth(); ++x)
                {
                    pixelWeight += *ptr;
                    ptr += stride;
                }
            }
            double maskWeight = static_cast<double>(totalWeight) / static_cast<double>(pixelWeight);
            settings->lightMeterTableWeight = static_cast<float>(maskWeight);
        }
        else
        {
            settings->lightMeterTableWeight = 1.0f;
        }

        Texture::PathKey key(lightMeterTablePath);
        settings->lightMeterTable = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
        posteffect->SetLightMeterTexture(settings->lightMeterTable);
        SafeDelete(desc);
    }
}

void PostEffectSystem::AddEntity(Entity* entity)
{
    Component* c = entity->GetComponent<PostEffectComponent>();
    if (c)
    {
        ImmediateEvent(c, EventSystem::POSTEFFECT_CHANGED);
    }
}

void PostEffectSystem::Process(float timeElapsed)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_CAMERA_TARGET_LUMINANCE, &currentTargetLuminance, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_CAMERA_DYNAMIC_RANGE, currentDynamicRange.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
}
};
