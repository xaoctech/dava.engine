#include "UIScreenshoter.h"
#include "Engine/Engine.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/RenderHelper.h"
#include "Render/Texture.h"
#include "Scene3D/Scene.h"
#include "UI/Render/UIRenderSystem.h"
#include "UI/UI3DView.h"
#include "UI/UIControlSystem.h"
#include "UI/Update/UIUpdateSystem.h"
#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

namespace DAVA
{
UIScreenshoter::UIScreenshoter()
{
}

UIScreenshoter::~UIScreenshoter()
{
    waiters.clear();
}

void UIScreenshoter::OnFrame()
{
    auto itEnd = waiters.end();
    for (auto it = waiters.begin(); it != itEnd;)
    {
        if (rhi::SyncObjectSignaled(it->syncObj))
        {
            if (it->callback != nullptr)
            {
                it->callback(it->texture);
            }

            it = waiters.erase(it);
            itEnd = waiters.end();
        }
        else
        {
            ++it;
        }
    }
}

Asset<Texture> UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, bool clearAlpha, bool prepareControl)
{
    return MakeScreenshot(control, format, nullptr, clearAlpha, prepareControl);
}

Asset<Texture> UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, Function<void(const Asset<Texture>&)> callback, bool clearAlpha, bool prepareControl)
{
    const Vector2 size(GetEngineContext()->uiControlSystem->vcs->ConvertVirtualToPhysical(control->GetSize()));
    AssetManager* assetManager = GetEngineContext()->assetManager;

    Texture::RenderTargetTextureKey key;
    key.width = static_cast<int32>(size.dx);
    key.height = static_cast<int32>(size.dy);
    key.format = format;
    key.isDepth = false;
    Asset<Texture> screenshot = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);

    key.isDepth = true;
    Asset<Texture> depthTarget = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);

    MakeScreenshotInternal(control, screenshot, depthTarget, callback, clearAlpha, prepareControl);

    return screenshot;
}

void UIScreenshoter::MakeScreenshot(UIControl* control, const Asset<Texture>& screenshot, const Asset<Texture>& depthTarget, bool clearAlpha, bool prepareControl)
{
    MakeScreenshotInternal(control, screenshot, depthTarget, nullptr, clearAlpha, prepareControl);
}

void UIScreenshoter::MakeScreenshot(UIControl* control, const Asset<Texture>& screenshot, const Asset<Texture>& depthTarget,
                                    Function<void(const Asset<Texture>&)> callback, bool clearAlpha, bool prepareControl,
                                    const rhi::Viewport& viewport)
{
    MakeScreenshotInternal(control, screenshot, depthTarget, callback, clearAlpha, prepareControl, viewport);
}

void UIScreenshoter::Unsubscribe(const Asset<Texture>& screenshot)
{
    for (auto& waiter : waiters)
    {
        if (waiter.texture == screenshot)
        {
            waiter.callback = nullptr;
        }
    }
}

void UIScreenshoter::MakeScreenshotInternal(UIControl* control, Asset<Texture> screenshot, Asset<Texture> depthBuffer,
                                            Function<void(Asset<Texture>)> callback, bool clearAlpha,
                                            bool prepareControl, const rhi::Viewport& viewport)
{
    if (control == nullptr)
        return;
    DVASSERT(screenshot);
    // Prepare waiter
    ScreenshotWaiter waiter;
    waiter.texture = screenshot;
    waiter.depth = depthBuffer;
    waiter.callback = callback;
    waiter.syncObj = rhi::GetCurrentFrameSyncObject();
    waiters.push_back(waiter);
    // End preparing

    UIControlSystem* controlSystem = GetEngineContext()->uiControlSystem;

    // Render to texture
    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.colorAttachment = screenshot->handle;
    desc.depthAttachment = depthBuffer->handle;
    desc.width = viewport.width ? viewport.width : screenshot->width;
    desc.height = viewport.height ? viewport.height : screenshot->height;
    desc.format = screenshot->GetFormat();
    desc.priority = PRIORITY_SCREENSHOT + PRIORITY_MAIN_2D;
    desc.clearTarget = controlSystem->GetRenderSystem()->GetUI3DViewCount() == 0;
    desc.transformVirtualToPhysical = true;

    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    if (prepareControl)
    {
        controlSystem->ForceUpdateControl(0.0f, control);
    }
    controlSystem->ForceDrawControl(control);

    //[CLEAR ALPHA]
    if (clearAlpha)
    {
        Vector2 rectSize = controlSystem->vcs->ConvertPhysicalToVirtual(Vector2(float32(screenshot->width), float32(screenshot->height)));
        RenderSystem2D::Instance()->FillRect(Rect(0.0f, 0.0f, rectSize.dx, rectSize.dy), Color::White, RenderSystem2D::DEFAULT_2D_FILL_ALPHA_MATERIAL);
    }

    RenderSystem2D::Instance()->EndRenderTargetPass();
    // End render
}
};
