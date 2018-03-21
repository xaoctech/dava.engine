#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseTextureRenderLandscapeTool.h"

#include <Asset/Asset.h>
#include <Base/RefPtr.h>
#include <Render/RHI/rhi_Public.h>

namespace DAVA
{
class NMaterial;
class Texture;
struct Rect;
class UVPickTextureCopier final
{
public:
    UVPickTextureCopier();
    ~UVPickTextureCopier();

    void BlitTextureRect(const Rect& srcRect, Asset<Texture> targetTexture, int32 basePriority);
    int32 GetPriority(int32 basePriority) const;

private:
    RefPtr<NMaterial> blitMaterial;
    rhi::Packet blitPacket;
    rhi::HVertexBuffer vertexBuffer;
    rhi::HSamplerState samplerState;
    rhi::HTextureSet textureSet;
};

class TextureBlitter final
{
public:
    TextureBlitter();
    ~TextureBlitter();

    struct TargetInfo
    {
        Asset<Texture> renderTarget;
        uint32 textureLevel = 0;
    };
    void BlitTexture(const TargetInfo& renderTerget, RefPtr<NMaterial> material, int32 priority);

private:
    RefPtr<NMaterial> blitMaterial;
    rhi::Packet blitPacket;
    rhi::HVertexBuffer vertexBuffer;
};

class BrushApplyHelper final
{
public:
    BrushApplyHelper();
    ~BrushApplyHelper();

    bool ApplyBrush(const BaseTextureRenderLandscapeTool::BrushPhaseDescriptor& brush, const Rect& rect, int32 basePriority);
    int32 GetPriority(int32 basePriority) const;

private:
    rhi::Packet packet;
    rhi::HVertexBuffer vertexBuffer;
};
} // namespace DAVA
