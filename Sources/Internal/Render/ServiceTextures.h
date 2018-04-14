#pragma once

#include "Render/Texture.h"

namespace DAVA
{
namespace ServiceTextures
{
Texture* CreateHammersleySet(uint32 count);
Texture* CreateHammersleySet(std::array<uint32, 4> sizes);
Texture* GenerateSplitSumApproximationLookupTexture(uint32 width, uint32 height);
Texture* GenerateNoiseTexture(uint32 width, uint32 height);

rhi::HTexture GeneratePointLightLookupTexture(uint32 faceSize);
rhi::HTexture GenerateAtmosphericTransmittanceTexture(uint32 width, uint32 height);
rhi::HTexture GenerateAtmosphericScatteringTexture(uint32 width, uint32 height, uint32 depth);
};
}
