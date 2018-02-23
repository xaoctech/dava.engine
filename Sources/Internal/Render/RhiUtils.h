#pragma once

#include "Render/RHI/rhi_Public.h"
#include "Math/Color.h"

namespace DAVA
{
namespace RhiUtils
{
class VertexTextureSet
{
public:
    VertexTextureSet(std::initializer_list<rhi::HTexture> init);
    operator rhi::HTextureSet() const;

private:
    rhi::TextureSetDescriptor textureDescr;
    friend class TextureSet;
};

class FragmentTextureSet
{
public:
    FragmentTextureSet() = default;
    FragmentTextureSet(std::initializer_list<rhi::HTexture> init);

    rhi::HTexture& operator[](uint32 index);

    operator rhi::HTextureSet() const;

private:
    rhi::TextureSetDescriptor textureDescr;
    friend class TextureSet;
};

class TextureSet
{
public:
    TextureSet(const VertexTextureSet& vs, const FragmentTextureSet& fs);

    operator rhi::HTextureSet() const;
    rhi::TextureSetDescriptor td;
};

void SetTargetClearColor(rhi::RenderPassConfig& config, float32 r, float32 g, float32 b, float32 a, int32 attachment = 0);
void SetTargetClearColor(rhi::RenderPassConfig& config, const Color& color, int32 attachment = 0);
}
}