#pragma once

#include "Render/RHI/rhi_Type.h"
#include "Base/RefPtr.h"
#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/QuadRenderer.h"
#include "Render/Texture.h"

namespace DAVA
{
class RenderSystem;
class RenderPass;
class NMaterial;
class Camera;

class CubemapRenderer
{
public:
    CubemapRenderer();
    ~CubemapRenderer();

    void RenderCubemap(RenderSystem* renderSystem, RenderPass* renderPass, const Vector3& point,
                       const Asset<Texture>& target, const Asset<Texture>& depthStencil, uint32 drawLayersMask);

    void ConvoluteDiffuseCubemap(Asset<Texture>& inputTexture, rhi::HTexture cubemapOutput,
                                 uint32 outputWidth, uint32 outputHeight, uint32 outputMipLevels);

    void ConvoluteSpecularCubemap(Asset<Texture>& inputTexture, Asset<Texture>& outputTexture, uint32 outputMipLevels);

    void EdgeFilterCubemap(Asset<Texture>& inputTexture, Asset<Texture>& outputTexture, uint32 outputMipLevels);
    void CopyCubemap(const Asset<Texture>& inputTexture, uint32 inputStartMip, uint32 mipCount,
                     const Asset<Texture>& outputTexture, uint32 outputStartMip);

    void ConvoluteSphericalHarmonics(Asset<Texture>& input, rhi::HTexture target);

    void InvalidateMaterials();

    static void GetBasisVectors(uint32 faceIndex, Vector3& direction, Vector3& up);

private:
    void SetupCameraToRenderFromPointToFaceIndex(const Vector3& point, uint32 faceIndex);
    void RenderFace(RenderSystem* renderSystem, RenderPass* renderPass, uint32 visibleLayers);

    rhi::RenderPassConfig renderTargetConfig;

    Camera* cubemapCamera = nullptr;
    QuadRenderer quadRenderer;

    NMaterial* cubemapFunctionsMaterial = nullptr;
    Asset<Texture> hammersleyTexture;

    const FastName CUBEMAP_SPECULAR_CONVOLUTION = FastName("ConvolutionSpecular");
    const FastName CUBEMAP_DIFFUSE_CONVOLUTION = FastName("ConvolutionDiffuse");
    const FastName SH_DIFFUSE_CONVOLUTION = FastName("ConvolutionSH");
    const FastName CUBEMAP_DOWNSAMPLE_FILTER = FastName("Downsampling");

    const FastName CUBEMAP_BASIS_A = FastName("cubemap_basis_a");
    const FastName CUBEMAP_BASIS_B = FastName("cubemap_basis_b");
    const FastName CUBEMAP_BASIS_C = FastName("cubemap_basis_c");

    const FastName CUBEMAP_DST_LAST_MIP = FastName("cubemap_dst_last_mip");
    const FastName CUBEMAP_SRC_LAST_MIP = FastName("cubemap_src_last_mip");
    const FastName CUBEMAP_DST_MIP_LEVEL = FastName("cubemap_dst_mip_level");
    const FastName CUBEMAP_SRC_MIP_LEVEL = FastName("cubemap_src_mip_level");
    const FastName CUBEMAP_FACE_INDEX = FastName("cubemap_face_index");

    const FastName CUBEMAP_FACE_SIZE = FastName("cubemap_face_size");
    const FastName CUBEMAP_SRC_SIZE = FastName("cubemap_src_size");

    const FastName HAMMERSLEY_SAMPLER_NAME = FastName("smp_hammersley");
    const FastName SRC_SAMPLER_NAME = FastName("smp_src");

    const FastName DYNAMIC_HAMMERSLEY_SET_SIZE = FastName("dynamicHammersleySetSize");
    const FastName DYNAMIC_HAMMERSLEY_SET_INDEX = FastName("dynamicHammersleySetIndex");
    const std::array<uint32, 4> HAMMERSLEY_SIZES;
};
}
