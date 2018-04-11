#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include <Render/Highlevel/Landscape.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Material/NMaterial.h>
#include <Render/RHI/rhi_Type.h>

namespace DAVA
{
const FastName LandscapeProxy::LANDSCAPE_TEXTURE_TOOL("toolTexture");
const FastName LandscapeProxy::LANDSCAPE_TEXTURE_CURSOR("cursorTexture");
const FastName LandscapeProxy::LANSDCAPE_FLAG_CURSOR("LANDSCAPE_CURSOR");
const FastName LandscapeProxy::LANSDCAPE_FLAG_TOOL("LANDSCAPE_TOOL");
const FastName LandscapeProxy::LANSDCAPE_FLAG_TOOL_MIX("LANDSCAPE_TOOL_MIX");
const FastName LandscapeProxy::LANDSCAPE_PARAM_CURSOR_COORD_SIZE("cursorCoordSize");

LandscapeProxy::LandscapeProxy(Landscape* landscape, Entity* node)
{
    DVASSERT(landscape != nullptr);

    baseLandscape = SafeRetain(landscape);

    ReloadLayersCountDependentResources();

    landscapeEditorMaterial = new NMaterial();
    landscapeEditorMaterial->SetMaterialName(FastName("Landscape.Tool.Material"));
    landscapeEditorMaterial->SetFXName(FastName("~res:/Materials2/Landscape.Tool.material"));
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_TOOL, 0);
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_CURSOR, 0);
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_TOOL_MIX, 0);
    landscapeEditorMaterial->AddProperty(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data, rhi::ShaderProp::TYPE_FLOAT4);

    cursorTexture = CreateSingleMipTexture(DefaultCursorPath());
    cursorTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    cursorTexture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);
    landscapeEditorMaterial->AddTexture(LANDSCAPE_TEXTURE_CURSOR, cursorTexture);

    Renderer::GetSignals().needRestoreResources.Connect(this, &LandscapeProxy::RestoreResources);
}

LandscapeProxy::~LandscapeProxy()
{
    Renderer::GetSignals().needRestoreResources.Disconnect(this);
    SafeRelease(landscapeEditorMaterial);

    SafeRelease(baseLandscape);
    for (auto& imageCopy : tilemaskImageCopies)
        SafeRelease(imageCopy);

    for (auto& tilemaskDrawTex : tilemaskDrawTextures)
    {
        SafeRelease(tilemaskDrawTex[TILEMASK_TEXTURE_SOURCE]);
        SafeRelease(tilemaskDrawTex[TILEMASK_TEXTURE_DESTINATION]);
    }

    SafeRelease(cursorTexture);
}

void LandscapeProxy::RestoreResources()
{
    if (rhi::NeedRestoreTexture(cursorTexture->handle))
    {
        ScopedPtr<Image> image(ImageSystem::LoadSingleMip(DefaultCursorPath()));
        rhi::UpdateTexture(cursorTexture->handle, image->GetData(), 0);
    }
}

void LandscapeProxy::ReloadLayersCountDependentResources()
{
    tilemaskDrawTextures.clear();
    for (uint32 i = 0; i < GetLayersCount(); ++i)
    {
        tilemaskDrawTextures.emplace_back();
        tilemaskDrawTextures[i][TILEMASK_TEXTURE_SOURCE] = nullptr;
        tilemaskDrawTextures[i][TILEMASK_TEXTURE_DESTINATION] = nullptr;
    }
}

void LandscapeProxy::SetMode(LandscapeProxy::eLandscapeMode _mode)
{
    if (mode != _mode)
    {
        mode = _mode;
        if (mode == LandscapeProxy::MODE_CUSTOM_LANDSCAPE)
        {
            landscapeEditorMaterial->SetParent(baseLandscape->GetLandscapeMaterial());
            baseLandscape->SetLandscapeMaterial(landscapeEditorMaterial);
        }
        else
        {
            baseLandscape->SetLandscapeMaterial(landscapeEditorMaterial->GetParent());
            landscapeEditorMaterial->SetParent(nullptr);
        }
    }
}

const AABBox3& LandscapeProxy::GetLandscapeBoundingBox()
{
    return baseLandscape->GetBoundingBox();
}

Texture* LandscapeProxy::GetLandscapeTexture(uint32 layerIndex, const FastName& level)
{
    return baseLandscape->GetPageMaterials(layerIndex, 0)->GetEffectiveTexture(level);
}

Color LandscapeProxy::GetLandscapeTileColor(uint32 layerIndex, const FastName& level)
{
    const float32* prop = baseLandscape->GetPageMaterials(layerIndex, 0)->GetEffectivePropValue(level);
    if (prop)
        return Color(prop[0], prop[1], prop[2], 1.f);
    else
        return Color::White;
}

void LandscapeProxy::SetLandscapeTileColor(uint32 layerIndex, const FastName& level, const Color& color)
{
    NMaterial* landscapeMaterial = baseLandscape->GetPageMaterials(layerIndex, 0);
    while (landscapeMaterial)
    {
        if (landscapeMaterial->HasLocalProperty(level))
            break;

        landscapeMaterial = landscapeMaterial->GetParent();
    }

    if (landscapeMaterial)
    {
        landscapeMaterial->SetPropertyValue(level, color.color);
    }
}

void LandscapeProxy::SetToolTexture(Texture* texture, bool mixColors)
{
    if (texture)
    {
        landscapeEditorMaterial->AddTexture(LANDSCAPE_TEXTURE_TOOL, texture);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL, 1);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL_MIX, (mixColors) ? 1 : 0);
    }
    else
    {
        landscapeEditorMaterial->RemoveTexture(LANDSCAPE_TEXTURE_TOOL);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL, 0);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL_MIX, 0);
    }
}

void LandscapeProxy::SetHeightmap(Heightmap* heightmap)
{
    baseLandscape->SetHeightmap(heightmap);
}

void LandscapeProxy::CursorEnable()
{
    landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_CURSOR, 1);
}

void LandscapeProxy::CursorDisable()
{
    landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_CURSOR, 0);
}

void LandscapeProxy::SetCursorTexture(Texture* texture)
{
    if (cursorTexture != texture)
    {
        SafeRelease(cursorTexture);
        cursorTexture = SafeRetain(texture);
    }

    landscapeEditorMaterial->SetTexture(LANDSCAPE_TEXTURE_CURSOR, texture);
}

void LandscapeProxy::SetCursorSize(float32 size)
{
    cursorCoordSize.z = size;
    cursorCoordSize.w = size;

    landscapeEditorMaterial->SetPropertyValue(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data);
}

void LandscapeProxy::SetCursorPosition(const Vector2& position)
{
    cursorCoordSize.x = position.x;
    cursorCoordSize.y = position.y;

    landscapeEditorMaterial->SetPropertyValue(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data);
}

Vector3 LandscapeProxy::PlacePoint(const Vector3& point)
{
    Vector3 landscapePoint;
    baseLandscape->PlacePoint(point, landscapePoint);

    return landscapePoint;
}

bool LandscapeProxy::IsTilemaskChanged()
{
    return (tilemaskWasChanged != 0);
}

void LandscapeProxy::ResetTilemaskChanged()
{
    tilemaskWasChanged = 0;
}

void LandscapeProxy::IncreaseTilemaskChanges()
{
    ++tilemaskWasChanged;
}

void LandscapeProxy::DecreaseTilemaskChanges()
{
    --tilemaskWasChanged;
}

bool LandscapeProxy::InitTilemaskImageCopy(const Vector<FilePath>& sourceTilemaskPath)
{
    for (auto& imageCopy : tilemaskImageCopies)
        SafeRelease(imageCopy);
    tilemaskImageCopies.clear();

    for (const auto& path : sourceTilemaskPath)
    {
        tilemaskImageCopies.push_back(ImageSystem::LoadSingleMip(path));
        if (tilemaskImageCopies.back() == nullptr)
            return false;
    }
    return true;
}

Image* LandscapeProxy::GetTilemaskImageCopy(uint32 layerIndex)
{
    return tilemaskImageCopies[layerIndex];
}

void LandscapeProxy::InitTilemaskDrawTextures()
{
    auto updateTexture = [](Texture*& texture, uint32 texSize) {
        if (texture != nullptr && texture->GetWidth() != texSize)
        {
            SafeRelease(texture);
        }

        if (texture == nullptr)
        {
            texture = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, rhi::TEXTURE_TYPE_2D);
        }
    };

    for (uint32 i = 0; i < tilemaskDrawTextures.size(); ++i)
    {
        uint32 texSize = static_cast<uint32>(GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK)->GetWidth());

        updateTexture(tilemaskDrawTextures[i][TILEMASK_TEXTURE_SOURCE], texSize);
        updateTexture(tilemaskDrawTextures[i][TILEMASK_TEXTURE_DESTINATION], texSize);
    }
}

Texture* LandscapeProxy::GetTilemaskDrawTexture(uint32 layerIndex, int32 number)
{
    if (number >= 0 && number < TILEMASK_TEXTURE_COUNT)
    {
        return tilemaskDrawTextures[layerIndex][number];
    }

    return NULL;
}

uint32 LandscapeProxy::GetTilemaskDrawTexturesCount() const
{
    return static_cast<uint32>(tilemaskDrawTextures.size());
}

void LandscapeProxy::SwapTilemaskDrawTextures()
{
    for (auto& tileMaskDrawTex : tilemaskDrawTextures)
    {
        Texture* temp = tileMaskDrawTex[TILEMASK_TEXTURE_SOURCE];
        tileMaskDrawTex[TILEMASK_TEXTURE_SOURCE] = tileMaskDrawTex[TILEMASK_TEXTURE_DESTINATION];
        tileMaskDrawTex[TILEMASK_TEXTURE_DESTINATION] = temp;
    }
}

Landscape* LandscapeProxy::GetBaseLandscape()
{
    return baseLandscape;
}

uint32 LandscapeProxy::GetLayersCount() const
{
    return baseLandscape->GetLayersCount();
}
} // namespace DAVA
