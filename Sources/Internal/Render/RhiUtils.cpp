#include "Render/RhiUtils.h"

namespace DAVA
{
RhiUtils::VertexTextureSet::VertexTextureSet(std::initializer_list<rhi::HTexture> init)
{
    textureDescr.vertexTextureCount = 0;
    for (const rhi::HTexture& it : init)
    {
        if (it.IsValid())
        {
            textureDescr.vertexTexture[textureDescr.vertexTextureCount] = it;
            textureDescr.vertexTextureCount++;
        }
    }
}

RhiUtils::VertexTextureSet::operator rhi::HTextureSet() const
{
    return rhi::AcquireTextureSet(textureDescr);
}

RhiUtils::FragmentTextureSet::FragmentTextureSet(std::initializer_list<rhi::HTexture> init)
{
    textureDescr.vertexTextureCount = 0;
    for (const rhi::HTexture& it : init)
    {
        if (it.IsValid())
        {
            textureDescr.fragmentTexture[textureDescr.fragmentTextureCount] = it;
            textureDescr.fragmentTextureCount++;
        }
    }
}

rhi::HTexture& RhiUtils::FragmentTextureSet::operator[](uint32 index)
{
    textureDescr.fragmentTextureCount = std::max(textureDescr.fragmentTextureCount, index + 1);
    return textureDescr.fragmentTexture[index];
}

RhiUtils::FragmentTextureSet::operator rhi::HTextureSet() const
{
    return rhi::AcquireTextureSet(textureDescr);
}

RhiUtils::TextureSet::TextureSet(const VertexTextureSet& vs, const FragmentTextureSet& fs)
{
    td.vertexTextureCount = vs.textureDescr.vertexTextureCount;
    for (size_t i = 0; i < td.vertexTextureCount; ++i)
    {
        td.vertexTexture[i] = vs.textureDescr.vertexTexture[i];
    }

    td.fragmentTextureCount = fs.textureDescr.fragmentTextureCount;
    for (size_t i = 0; i < td.fragmentTextureCount; ++i)
    {
        td.fragmentTexture[i] = fs.textureDescr.fragmentTexture[i];
    }
}

RhiUtils::TextureSet::operator rhi::HTextureSet() const
{
    rhi::HTextureSet set = rhi::AcquireTextureSet(td);
    return set;
}

void RhiUtils::SetTargetClearColor(rhi::RenderPassConfig& config, float32 r, float32 g, float32 b, float32 a, int32 attachment)
{
    config.colorBuffer[attachment].clearColor[0] = r;
    config.colorBuffer[attachment].clearColor[1] = g;
    config.colorBuffer[attachment].clearColor[2] = b;
    config.colorBuffer[attachment].clearColor[3] = a;
}

void RhiUtils::SetTargetClearColor(rhi::RenderPassConfig& config, const Color& color, int32 attachment)
{
    config.colorBuffer[attachment].clearColor[0] = color.r;
    config.colorBuffer[attachment].clearColor[1] = color.g;
    config.colorBuffer[attachment].clearColor[2] = color.b;
    config.colorBuffer[attachment].clearColor[3] = color.a;
}
}
