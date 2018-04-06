#pragma once

#include "Math/Color.h"
#include "Math/Matrix4.h"
#include "Math/Vector.h"
#include "Base/FastName.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/Texture.h"

namespace DAVA
{
class RuntimeTextures
{
public:
    enum eRuntimeTextureSemantic
    {
        TEXTURE_STATIC = 0,
        TEXTURE_REFLECTION,
        TEXTURE_REFRACTION,
        TEXTURE_UVPICKING,
        TEXTURE_SHARED_DEPTHBUFFER,

        TEXTURE_GBUFFER_0,
        TEXTURE_GBUFFER_1,
        TEXTURE_GBUFFER_2,
        TEXTURE_GBUFFER_3,
        TEXTURE_GBUFFER_0_COPY,
        TEXTURE_GBUFFER_1_COPY,
        TEXTURE_GBUFFER_2_COPY,
        TEXTURE_GBUFFER_3_COPY,

        TEXTURE_INDIRECT_SPECULAR_LOOKUP,
        TEXTURE_HAMMERSLEY_SET,
        TEXTURE_SCREEN_SPACE_NOISE,

        TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER,

        TEXTURE_VELOCITY,

        RUNTIME_TEXTURES_END,
        RUNTIME_TEXTURES_COUNT = RUNTIME_TEXTURES_END,
    };

public:
    RuntimeTextures();
    ~RuntimeTextures();

    void Teardown();
    static RuntimeTextures::eRuntimeTextureSemantic GetRuntimeTextureSemanticByName(const FastName& name);

    void Reset(Size2i screenDim);

    rhi::HTexture GetRuntimeTexture(eRuntimeTextureSemantic semantic);
    rhi::SamplerState::Descriptor::Sampler GetRuntimeTextureSamplerState(eRuntimeTextureSemantic semantic);
    Size2i GetRuntimeTextureSize(eRuntimeTextureSemantic semantic);
    PixelFormat GetRuntimeTextureFormat(eRuntimeTextureSemantic semantic);

    rhi::HTexture GetPinkTexture(rhi::TextureType type);
    rhi::SamplerState::Descriptor::Sampler GetPinkTextureSamplerState(rhi::TextureType type);

    void InvalidateTexture(eRuntimeTextureSemantic);
    void ClearRuntimeTextures();

private:
    void InitRuntimeTexture(eRuntimeTextureSemantic semantic);

    rhi::HTexture runtimeTextures[RUNTIME_TEXTURES_COUNT]{};
    rhi::SamplerState::Descriptor::Sampler samplerDescriptors[RUNTIME_TEXTURES_COUNT]{};
    Size2i runtimeTextureSizes[RUNTIME_TEXTURES_COUNT]{};
    PixelFormat runtimeTexturesFormat[RUNTIME_TEXTURES_COUNT]{};
    Asset<Texture> pinkTexture[2]{};
};
}
