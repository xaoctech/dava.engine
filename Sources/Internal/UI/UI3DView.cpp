#include "UI/UI3DView.h"
#include "Engine/Engine.h"
#include "Scene3D/Scene.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/RenderHelper.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/SnapToLandscapeControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/Update/UIUpdateComponent.h"
#include "UI/Scene3D/UISceneComponent.h"
#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UI3DView)
{
    ReflectionRegistrator<UI3DView>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UI3DView* o) { o->Release(); })
    .Field("drawToFrameBuffer", &UI3DView::GetDrawToFrameBuffer, &UI3DView::SetDrawToFrameBuffer)
    .Field("frameBufferScaleFactor", &UI3DView::GetFrameBufferScaleFactor, &UI3DView::SetFrameBufferScaleFactor)
    .Field("scene", &UI3DView::GetScene, &UI3DView::SetScene)
    .End();
}

UI3DView::UI3DView(const Rect& rect)
    : UIControl(rect)
    , scene(nullptr)
    , drawToFrameBuffer(false)
    , fbScaleFactor(1.f)
    , fbRenderSize()
{
    GetOrCreateComponent<UIUpdateComponent>(); //TODO Remove this code. move Update And Draw methods to UIRenderSystem.
    GetOrCreateComponent<UISceneComponent>();
}

UI3DView::~UI3DView()
{
    SafeRelease(scene);
}

void UI3DView::SetScene(Scene* _scene)
{
    SafeRelease(scene);

    scene = SafeRetain(_scene);

    if (scene)
    {
        float32 aspect = size.dx / size.dy;
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

Scene* UI3DView::GetScene() const
{
    return scene;
}

void UI3DView::AddControl(UIControl* control)
{
    DVASSERT(0 && "UI3DView do not support children");
}

void UI3DView::Update(float32 timeElapsed)
{
    if (scene)
    {
        scene->Update(timeElapsed);
    }
}

void UI3DView::Draw(const UIGeometricData& geometricData)
{
    if (!scene)
        return;

    RenderSystem2D::Instance()->Flush();

    const RenderSystem2D::RenderTargetPassDescriptor& currentTarget = RenderSystem2D::Instance()->GetActiveTargetDescriptor();

    Rect viewportRect = geometricData.GetUnrotatedRect();

    if (currentTarget.transformVirtualToPhysical)
        viewportRc = GetEngineContext()->uiControlSystem->vcs->ConvertVirtualToPhysical(viewportRect);
    else
        viewportRc = viewportRect;

    uint32 priority = currentTarget.priority;

    uint32 targetWidth = 0;
    uint32 targetHeight = 0;
    PixelFormat targetFormat = PixelFormat::FORMAT_INVALID;

    rhi::HTexture colorTexture;
    rhi::HTexture depthStencilTexture;

    if (drawToFrameBuffer)
    {
        viewportRc.x = 0.0f;
        viewportRc.y = 0.0f;
        viewportRc.dx *= fbScaleFactor;
        viewportRc.dy *= fbScaleFactor;

        PrepareFrameBuffer();

        priority += PRIORITY_SERVICE_3D;
        colorTexture = colorFrameBuffer->handle;
        depthStencilTexture = depthFrameBuffer->handle;
        targetFormat = colorFrameBuffer->GetFormat();
        targetWidth = colorFrameBuffer->width;
        targetHeight = colorFrameBuffer->height;
    }
    else
    {
        if (currentTarget.transformVirtualToPhysical)
        {
            viewportRc += GetEngineContext()->uiControlSystem->vcs->GetPhysicalDrawOffset();
        }

        priority += basePriority;
        colorTexture = currentTarget.colorAttachment;
        depthStencilTexture = currentTarget.depthAttachment.IsValid() ? currentTarget.depthAttachment : rhi::HTexture(rhi::DefaultDepthBuffer);

        if (currentTarget.colorAttachment == rhi::InvalidHandle)
        {
            targetFormat = PixelFormat::FORMAT_RGBA8888;
            targetWidth = Renderer::GetFramebufferWidth();
            targetHeight = Renderer::GetFramebufferHeight();
        }
        else
        {
            targetFormat = currentTarget.format;
            targetWidth = currentTarget.width;
            targetHeight = currentTarget.height;
        }
    }

    DVASSERT(targetWidth > 0);
    DVASSERT(targetHeight > 0);
    DVASSERT(targetFormat != PixelFormat::FORMAT_INVALID);

    scene->SetMainRenderTarget(colorTexture, depthStencilTexture, !clearColorContent, currentTarget.clearColor);
    scene->SetMainPassProperties(priority, viewportRc, targetWidth, targetHeight, targetFormat);
    scene->Draw();

    if (drawToFrameBuffer)
    {
        RenderSystem2D::Instance()->DrawTexture(colorFrameBuffer, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, geometricData.GetUnrotatedRect(), Rect(Vector2(), fbTexSize));
    }
}

bool UI3DView::IsClearRequested() const
{
    return clearColorContent;
}

void UI3DView::SetClearRequested(bool requested)
{
    clearColorContent = requested;
}

void UI3DView::SetSize(const DAVA::Vector2& newSize)
{
    UIControl::SetSize(newSize);
    float32 aspect = size.dx / size.dy;

    if (scene)
    {
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

UI3DView* UI3DView::Clone()
{
    UI3DView* ui3DView = new UI3DView(GetRect());
    ui3DView->CopyDataFrom(this);
    return ui3DView;
}

void UI3DView::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);

    UI3DView* srcView = DynamicTypeCheck<UI3DView*>(srcControl);
    drawToFrameBuffer = srcView->drawToFrameBuffer;
    fbScaleFactor = srcView->fbScaleFactor;
    fbRenderSize = srcView->fbRenderSize;
    fbTexSize = srcView->fbTexSize;
}

void UI3DView::Input(UIEvent* currentInput)
{
    if (scene != nullptr)
    {
        scene->Input(currentInput);
    }

    UIControl::Input(currentInput);
}

void UI3DView::InputCancelled(UIEvent* currentInput)
{
    if (scene != nullptr)
    {
        scene->InputCancelled(currentInput);
    }

    UIControl::InputCancelled(currentInput);
}

void UI3DView::SetDrawToFrameBuffer(bool enable)
{
    drawToFrameBuffer = enable;

    if (!enable)
    {
        colorFrameBuffer.reset();
        depthFrameBuffer.reset();
    }
}

void UI3DView::SetFrameBufferScaleFactor(float32 scale)
{
    fbScaleFactor = scale;
}

void UI3DView::PrepareFrameBuffer()
{
    DVASSERT(scene);

    fbRenderSize = GetEngineContext()->uiControlSystem->vcs->ConvertVirtualToPhysical(GetSize()) * fbScaleFactor;

    if (colorFrameBuffer == nullptr || colorFrameBuffer->width < fbRenderSize.dx || colorFrameBuffer->height < fbRenderSize.dy)
    {
        colorFrameBuffer.reset();
        depthFrameBuffer.reset();

        Texture::RenderTargetTextureKey key;
        key.width = static_cast<int32>(fbRenderSize.dx);
        key.height = static_cast<int32>(fbRenderSize.dy);
        key.format = FORMAT_RGBA8888;
        key.isDepth = false;
        key.needPixelReadback = false;
        key.textureType = rhi::TEXTURE_TYPE_2D;
        key.ensurePowerOf2 = true;

        AssetManager* assetManager = GetEngineContext()->assetManager;
        colorFrameBuffer = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);

        key.isDepth = true;
        depthFrameBuffer = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
    }

    Vector2 fbSize = Vector2(static_cast<float32>(colorFrameBuffer->width), static_cast<float32>(colorFrameBuffer->height));

    fbTexSize = fbRenderSize / fbSize;
}
}
