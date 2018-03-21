#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "REPlatform/Deprecated/EditorConfig.h"
#include "REPlatform/DataNodes/ProjectManagerData.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include <TArc/Core/Deprecated.h>

#include <Asset/Asset.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Render/Texture.h>
#include <Render/Material/NMaterial.h>

namespace DAVA
{
CustomColorsProxy::CustomColorsProxy(int32 _size)
    : changedRect(Rect())
    , spriteChanged(false)
    , textureLoaded(false)
    , size(_size)
    , changes(0)
    , brushMaterial(new NMaterial())
{
    Texture::RenderTargetTextureKey key;
    key.width = size;
    key.height = size;
    key.textureType = rhi::TextureType::TEXTURE_TYPE_2D;
    key.format = PixelFormat::FORMAT_RGBA8888;
    key.isDepth = false;
    key.needPixelReadback = true;
    customColorsRenderTarget = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);

    // clear texture, to initialize frame buffer object
    // using PRIORITY_SERVICE_2D + 1 to ensure it will be cleared before drawing existing image into render target
    RenderHelper::CreateClearPass(customColorsRenderTarget->handle, rhi::HTexture(), PRIORITY_SERVICE_2D + 1, Color::Clear, rhi::Viewport(0, 0, size, size));

    brushMaterial->SetMaterialName(FastName("CustomColorsMaterial"));
    brushMaterial->SetFXName(FastName("~res:/ResourceEditor/LandscapeEditor/Materials/CustomColors.material"));
    brushMaterial->PreBuildMaterial(RenderSystem2D::RENDER_PASS_NAME);
}

void CustomColorsProxy::ResetLoadedState(bool isLoaded)
{
    textureLoaded = isLoaded;
}

bool CustomColorsProxy::IsTextureLoaded() const
{
    return textureLoaded;
}

CustomColorsProxy::~CustomColorsProxy()
{
}

const Asset<Texture>& CustomColorsProxy::GetTexture()
{
    return customColorsRenderTarget;
}

void CustomColorsProxy::ResetTargetChanged()
{
    spriteChanged = false;
}

bool CustomColorsProxy::IsTargetChanged()
{
    return spriteChanged;
}

Rect CustomColorsProxy::GetChangedRect()
{
    if (IsTargetChanged())
    {
        return changedRect;
    }

    return Rect();
}

void CustomColorsProxy::UpdateRect(const Rect& rect)
{
    Rect bounds(0.f, 0.f, static_cast<float32>(size), static_cast<float32>(size));
    changedRect = rect;
    bounds.ClampToRect(changedRect);

    spriteChanged = true;
}

int32 CustomColorsProxy::GetChangesCount() const
{
    return changes;
}

void CustomColorsProxy::ResetChanges()
{
    changes = 0;
}

void CustomColorsProxy::IncrementChanges()
{
    ++changes;
}

void CustomColorsProxy::DecrementChanges()
{
    --changes;
}

void CustomColorsProxy::UpdateSpriteFromConfig()
{
    if (NULL == customColorsRenderTarget)
    {
        return;
    }

    rhi::Viewport viewport;
    viewport.x = viewport.y = 0;
    viewport.width = viewport.height = size;

    ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
    DVASSERT(data);

    Vector<Color> customColors = data->GetEditorConfig()->GetColorPropertyValues("LandscapeCustomColors");
    if (customColors.empty())
    {
        RenderHelper::CreateClearPass(customColorsRenderTarget->handle, rhi::HTexture(), PRIORITY_CLEAR, Color::Clear, viewport);
    }
    else
    {
        uint32 defaultColorIndex = Deprecated::GetDataNode<GlobalSceneSettings>()->defaultCustomColorIndex;
        defaultColorIndex = Min(defaultColorIndex, static_cast<uint32>(customColors.size() - 1));
        Color color = customColors[defaultColorIndex];
        RenderHelper::CreateClearPass(customColorsRenderTarget->handle, rhi::HTexture(), PRIORITY_CLEAR, color, viewport);
    }
}

NMaterial* CustomColorsProxy::GetBrushMaterial() const
{
    return brushMaterial.get();
}
} // namespace DAVA
