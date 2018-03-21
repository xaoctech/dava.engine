#pragma once

#include "Render/Texture.h"

namespace DAVA
{
namespace ServiceTextures
{
Asset<Texture> CreateHammersleySet(std::array<uint32, 4> sizes);
rhi::HTexture CreateHammersleySet(uint32 count);
rhi::HTexture GenerateSplitSumApproximationLookupTexture(uint32 width, uint32 height);
rhi::HTexture GenerateNoiseTexture(uint32 width, uint32 height);

rhi::HTexture GeneratePointLightLookupTexture(uint32 faceSize);
};
}
