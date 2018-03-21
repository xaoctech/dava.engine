#include "SceneImageGraber.h"

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Functional/Function.h>
#include <Job/JobManager.h>
#include <Math/MathHelpers.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/Highlevel/PostEffectRenderer.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/RenderBase.h>
#include <Render/Renderer.h>
#include <Render/RHI/rhi_Public.h>
#include <Render/RHI/rhi_Type.h>
#include <Render/Texture.h>
#include <Scene3D/Scene.h>

#include <QQuickWindow>
#include <QImage>

namespace SceneImageGrabber
{
namespace SceneImageGrabberDetail
{
struct InternalParams
{
    SceneImageGrabber::Params inputParams;
    DAVA::Asset<DAVA::Texture> renderTarget;
    DAVA::Asset<DAVA::Texture> renderTargetDepth;
};

void GrabImage(Params inputParams)
{
    DVASSERT(!inputParams.outputFile.IsEmpty());

    InternalParams internalParams;
    internalParams.inputParams = std::move(inputParams);

    DAVA::Size2i imageSize = internalParams.inputParams.imageSize;

    if (internalParams.inputParams.grabHDRTarget)
    {
        rhi::HTexture hdrTarget = internalParams.inputParams.scene->renderSystem->GetPostEffectRenderer()->GetHDRTarget();
        const DAVA::Size2i& hdrTargetSize = internalParams.inputParams.scene->renderSystem->GetPostEffectRenderer()->GetHDRTargetSize();

        void* mappedData = rhi::MapTexture(hdrTarget);
        {
            DAVA::ScopedPtr<DAVA::Image> image(DAVA::Image::CreateFromData(hdrTargetSize.dx, hdrTargetSize.dy, DAVA::PixelFormat::FORMAT_RGBA16F, static_cast<DAVA::uint8*>(mappedData)));
            DAVA::ScopedPtr<DAVA::Image> regionImage(DAVA::Image::CopyImageRegion(image, DAVA::Rect(0, 0, imageSize.dx, imageSize.dy)));
            regionImage->FlipVertical();
            DAVA::ImageSystem::Save(internalParams.inputParams.outputFile, regionImage, DAVA::PixelFormat::FORMAT_RGBA16F);
        }
        rhi::UnmapTexture(hdrTarget);
        return;
    }

    /*
     * Take screenshot
     */
    DAVA::AssetManager* assetManager = DAVA::GetEngineContext()->assetManager;
    DAVA::int32 maxSize = std::max(imageSize.dx, imageSize.dy);
    DAVA::Texture::RenderTargetTextureKey key;
    key.width = maxSize;
    key.height = maxSize;
    key.format = DAVA::FORMAT_RGBA8888;
    key.isDepth = false;
    internalParams.renderTarget = assetManager->GetAsset<DAVA::Texture>(key, DAVA::AssetManager::SYNC);
    key.isDepth = true;
    internalParams.renderTargetDepth = assetManager->GetAsset<DAVA::Texture>(key, DAVA::AssetManager::SYNC);

    DAVA::float32 cameraAspect = internalParams.inputParams.cameraToGrab->GetAspect();
    internalParams.inputParams.cameraToGrab->SetAspect(static_cast<DAVA::float32>(imageSize.dx) / imageSize.dy);
    internalParams.inputParams.scene->renderSystem->SetDrawCamera(internalParams.inputParams.cameraToGrab.Get());
    internalParams.inputParams.scene->renderSystem->SetMainCamera(internalParams.inputParams.cameraToGrab.Get());

    DAVA::Rect viewportRect(0, 0, imageSize.dx, imageSize.dy);
    internalParams.inputParams.scene->SetMainRenderTarget(internalParams.renderTarget->handle,
                                                          internalParams.renderTargetDepth->handle,
                                                          rhi::LOADACTION_CLEAR, DAVA::Color::Clear);
    internalParams.inputParams.scene->SetMainPassProperties(DAVA::PRIORITY_SERVICE_2D, viewportRect,
                                                            internalParams.renderTarget->width,
                                                            internalParams.renderTarget->height,
                                                            DAVA::PixelFormat::FORMAT_RGBA8888);
    internalParams.inputParams.scene->Draw();

    DAVA::RenderSystem2D* renderSystem = DAVA::RenderSystem2D::Instance();

    renderSystem->BeginRenderTargetPass(internalParams.renderTarget, internalParams.renderTargetDepth, false);
    renderSystem->FillRect(viewportRect, DAVA::Color::White, DAVA::RenderSystem2D::DEFAULT_2D_FILL_ALPHA_MATERIAL);
    renderSystem->EndRenderTargetPass();

    internalParams.inputParams.cameraToGrab->SetAspect(cameraAspect);

    DAVA::Renderer::RegisterSyncCallback(rhi::GetCurrentFrameSyncObject(), [internalParams](rhi::HSyncObject)
                                         {
                                             DAVA::FilePath filePath = internalParams.inputParams.outputFile;
                                             if (filePath.IsDirectoryPathname())
                                             {
                                                 filePath = DAVA::FilePath(filePath, "GrabbedScene.png");
                                             }

                                             DAVA::ScopedPtr<DAVA::Image> image(internalParams.renderTarget->CreateImageFromRegion());
                                             DAVA::Size2i imageSize = internalParams.inputParams.imageSize;
                                             DAVA::ScopedPtr<DAVA::Image> regionImage(DAVA::Image::CopyImageRegion(image, DAVA::Rect(0, 0, imageSize.dx, imageSize.dy)));
                                             regionImage->Save(filePath);
                                             if (internalParams.inputParams.readyCallback)
                                             {
                                                 internalParams.inputParams.readyCallback();
                                             }
                                         });
}
}

void GrabImage(Params params)
{
    if (params.processInDAVAFrame)
    {
        DAVA::GetEngineContext()->jobManager->CreateMainJob(DAVA::Bind(&SceneImageGrabberDetail::GrabImage, params), DAVA::JobManager::eMainJobType::JOB_MAINLAZY);
    }
    else
    {
        SceneImageGrabberDetail::GrabImage(params);
    }
}
}
