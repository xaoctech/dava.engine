#include "Render/Highlevel/CubemapRenderer.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/Camera.h"
#include "Render/ServiceTextures.h"

namespace DAVA
{
const rhi::TextureFace targetFaces[6] =
{
  rhi::TEXTURE_FACE_POSITIVE_X,
  rhi::TEXTURE_FACE_NEGATIVE_X,
  rhi::TEXTURE_FACE_POSITIVE_Y,
  rhi::TEXTURE_FACE_NEGATIVE_Y,
  rhi::TEXTURE_FACE_POSITIVE_Z,
  rhi::TEXTURE_FACE_NEGATIVE_Z,
};

static const Vector4 cubeBasisA[6] =
{
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f)
};

static const Vector4 cubeBasisB[6] =
{
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, 0.0f, +1.0f, 0.0f),
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f)
};

static const Vector4 cubeBasisC[6] =
{
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(0.0f, 1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  Vector4(0.0f, 0.0f, -1.0f, 0.0f)
};

CubemapRenderer::CubemapRenderer()
    : HAMMERSLEY_SIZES({ 64, 128, 512, 1024 })
{
    cubemapCamera = new Camera();
    cubemapCamera->SetupPerspective(90.0f, 1.0f, 0.1f, 5000.0f);

    renderTargetConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.priority = DAVA::PRIORITY_SERVICE_3D;
    // std::fill_n(renderTargetConfig.colorBuffer[0].clearColor, 4, 1.0f);

    hammersleyTexture = ServiceTextures::CreateHammersleySet(HAMMERSLEY_SIZES);

    cubemapFunctionsMaterial = new NMaterial();
    cubemapFunctionsMaterial->SetFXName(FastName("~res:/Materials2/CubemapConvolution.material"));

    cubemapFunctionsMaterial->AddProperty(CUBEMAP_BASIS_A, (const float32*)&cubeBasisA[0], rhi::ShaderProp::TYPE_FLOAT3);
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_BASIS_B, (const float32*)&cubeBasisB[0], rhi::ShaderProp::TYPE_FLOAT3);
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_BASIS_C, (const float32*)&cubeBasisC[0], rhi::ShaderProp::TYPE_FLOAT3);

    float32 tempFloat = 0;
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_DST_LAST_MIP, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_SRC_LAST_MIP, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_DST_MIP_LEVEL, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_SRC_MIP_LEVEL, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_FACE_INDEX, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);
    cubemapFunctionsMaterial->AddProperty(CUBEMAP_FACE_SIZE, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);

    cubemapFunctionsMaterial->AddProperty(DYNAMIC_HAMMERSLEY_SET_SIZE, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);
    cubemapFunctionsMaterial->AddProperty(DYNAMIC_HAMMERSLEY_SET_INDEX, &tempFloat, rhi::ShaderProp::TYPE_FLOAT1);

    //CUBEMAP_SRC_MIP_LEVEL
    cubemapFunctionsMaterial->AddTexture(HAMMERSLEY_SAMPLER_NAME, hammersleyTexture);
    //cubemapFunctionsMaterial->AddTexture(SRC_SAMPLER_NAME, hammersleyTexture); // GFX_COMPLETE

    cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_DIFFUSE_CONVOLUTION);
    cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_SPECULAR_CONVOLUTION);
    cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_DOWNSAMPLE_FILTER);
}

CubemapRenderer::~CubemapRenderer()
{
    SafeRelease(cubemapCamera);
    SafeRelease(cubemapFunctionsMaterial);
    SafeRelease(hammersleyTexture);
}

void CubemapRenderer::InvalidateMaterials()
{
    cubemapFunctionsMaterial->InvalidateRenderVariants();
}

void CubemapRenderer::RenderCubemap(RenderSystem* renderSystem,
                                    RenderPass* renderPass,
                                    const Vector3& point,
                                    rhi::HTexture cubemapTarget,
                                    rhi::HTexture depthStencilTarget,
                                    uint32 width,
                                    uint32 height,
                                    uint32 drawLayersMask)
{
    renderTargetConfig.colorBuffer[0].texture = cubemapTarget;
    renderTargetConfig.depthStencilBuffer.texture = depthStencilTarget;
    renderTargetConfig.viewport.width = width;
    renderTargetConfig.viewport.height = height;

    cubemapCamera->SetPosition(point);

    renderTargetConfig.colorBuffer[0].clearColor[0] = 0.0;
    renderTargetConfig.colorBuffer[0].clearColor[1] = 0.0;
    renderTargetConfig.colorBuffer[0].clearColor[2] = 0.0;
    renderTargetConfig.colorBuffer[0].clearColor[3] = 0.0;
    renderTargetConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;

    for (DAVA::uint32 i = 0; i < 6; ++i)
    {
        SetupCameraToRenderFromPointToFaceIndex(point, i);
        RenderFace(renderSystem, renderPass, drawLayersMask);
    }
}

void CubemapRenderer::SetupCameraToRenderFromPointToFaceIndex(const DAVA::Vector3& point, DAVA::uint32 faceIndex)
{
    Vector3 dir;
    Vector3 up;
    GetBasisVectors(faceIndex, dir, up);
    renderTargetConfig.colorBuffer[0].textureFace = targetFaces[faceIndex];
    cubemapCamera->SetTarget(point + dir);
    cubemapCamera->SetUp(up);
}

void CubemapRenderer::RenderFace(RenderSystem* renderSystem, RenderPass* renderPass, uint32 drawLayersMask)
{
    rhi::RenderPassConfig oldPassConfig = renderPass->GetPassConfig();
    Camera* oldCamera = renderSystem->GetMainCamera();
    Camera* oldDrawCamera = renderSystem->GetDrawCamera();

    renderPass->GetPassConfig() = renderTargetConfig;
    renderSystem->SetMainCamera(cubemapCamera);
    renderSystem->SetDrawCamera(cubemapCamera);

    renderPass->Draw(renderSystem, drawLayersMask);

    renderPass->GetPassConfig() = oldPassConfig;
    renderSystem->SetMainCamera(oldCamera);
    renderSystem->SetDrawCamera(oldDrawCamera);
}

void CubemapRenderer::ConvoluteDiffuseCubemap(Texture* inputTexture, rhi::HTexture cubemapOutput, uint32 outputWidth, uint32 outputHeight, uint32 outputMipLevels)
{
    cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, inputTexture);

    float32 dstMipLevel = 0;
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_DST_MIP_LEVEL, &dstMipLevel);

    float32 faceSizeF = (float32)(outputWidth);
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_SIZE, &faceSizeF);

    cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_DIFFUSE_CONVOLUTION);
    for (uint32 face = 0; face < 6; ++face)
    {
        float32 faceF = (float32)face;
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_INDEX, &faceF);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_A, (const float32*)&cubeBasisA[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_B, (const float32*)&cubeBasisB[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_C, (const float32*)&cubeBasisC[face].x);
        cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_DIFFUSE_CONVOLUTION);

        quadRenderer.Render("ConvoluteDiffuseCubemap", cubemapFunctionsMaterial,
                            rhi::Viewport(0, 0, outputWidth, outputHeight), cubemapOutput, targetFaces[face], 0);
    }
}

void CubemapRenderer::ConvoluteSphericalHarmonics(Texture* inputTexture, rhi::HTexture target)
{
    cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, inputTexture);
    if (cubemapFunctionsMaterial->PreBuildMaterial(SH_DIFFUSE_CONVOLUTION))
    {
        quadRenderer.Render("ConvoluteSphericalHarmonics", cubemapFunctionsMaterial, rhi::Viewport(0, 0, 9, 1), target,
                            rhi::TextureFace::TEXTURE_FACE_NONE, 0, rhi::LOADACTION_NONE, 0);
    }
}

void CubemapRenderer::ConvoluteSpecularCubemap(Texture* inputTexture, Texture* outputTexture, uint32 outputMipLevels)
{
    cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, inputTexture);

    float32 srcLastMip = float32(outputMipLevels - 1);
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_LAST_MIP, &srcLastMip);

    float32 dstLastMip = float32(outputMipLevels - 1);
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_DST_LAST_MIP, &dstLastMip);

    cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_SPECULAR_CONVOLUTION);

    for (uint32 face = 0; face < 6; ++face)
    {
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_A, (const float32*)&cubeBasisA[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_B, (const float32*)&cubeBasisB[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_C, (const float32*)&cubeBasisC[face].x);
        float32 faceF = (float32)face;
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_INDEX, &faceF);

        for (uint32 mip = 0; mip < outputMipLevels; ++mip)
        {
            float32 faceSizeF = (float32)(outputTexture->width / (1 << mip));
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_SIZE, &faceSizeF);

            if (faceSizeF >= 64)
            {
                float32 dynamicHammersleySetIndex = 0;
                float32 dynamicHammersleySetSize = static_cast<float32>(HAMMERSLEY_SIZES[static_cast<uint32>(dynamicHammersleySetIndex)]);
                cubemapFunctionsMaterial->SetPropertyValue(DYNAMIC_HAMMERSLEY_SET_INDEX, &dynamicHammersleySetIndex);
                cubemapFunctionsMaterial->SetPropertyValue(DYNAMIC_HAMMERSLEY_SET_SIZE, &dynamicHammersleySetSize);
            }
            else if (faceSizeF >= 16)
            {
                float32 dynamicHammersleySetIndex = 1;
                float32 dynamicHammersleySetSize = static_cast<float32>(HAMMERSLEY_SIZES[static_cast<uint32>(dynamicHammersleySetIndex)]);
                cubemapFunctionsMaterial->SetPropertyValue(DYNAMIC_HAMMERSLEY_SET_INDEX, &dynamicHammersleySetIndex);
                cubemapFunctionsMaterial->SetPropertyValue(DYNAMIC_HAMMERSLEY_SET_SIZE, &dynamicHammersleySetSize);
            }
            else
            {
                float32 dynamicHammersleySetIndex = 2;
                float32 dynamicHammersleySetSize = static_cast<float32>(HAMMERSLEY_SIZES[static_cast<uint32>(dynamicHammersleySetIndex)]);
                cubemapFunctionsMaterial->SetPropertyValue(DYNAMIC_HAMMERSLEY_SET_INDEX, &dynamicHammersleySetIndex);
                cubemapFunctionsMaterial->SetPropertyValue(DYNAMIC_HAMMERSLEY_SET_SIZE, &dynamicHammersleySetSize);
            }

            float32 srcMipLevel = float32(mip);
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_MIP_LEVEL, &srcMipLevel);

            float32 dstMipLevel = float32(mip);
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_DST_MIP_LEVEL, &dstMipLevel);

            cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_SPECULAR_CONVOLUTION);

            quadRenderer.Render("ConvoluteSpecularCubemap", cubemapFunctionsMaterial,
                                rhi::Viewport(0, 0, outputTexture->width / (1 << mip), outputTexture->height / (1 << mip)),
                                outputTexture->handle, (rhi::TextureFace)targetFaces[face], mip);
        }
    }
}

void CubemapRenderer::EdgeFilterCubemap(Texture* inputTexture,
                                        Texture* outputTexture,
                                        uint32 outputMipLevels)
{
    if (cubemapFunctionsMaterial->HasLocalTexture(SRC_SAMPLER_NAME))
        cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, inputTexture);
    else
        cubemapFunctionsMaterial->AddTexture(SRC_SAMPLER_NAME, inputTexture);

    float32 srcLastMip = float32(outputMipLevels - 1);
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_LAST_MIP, &srcLastMip);
    float32 dstLastMip = float32(outputMipLevels - 1);
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_DST_LAST_MIP, &dstLastMip);

    for (uint32 face = 0; face < 6; ++face)
    {
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_A, (const float32*)&cubeBasisA[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_B, (const float32*)&cubeBasisB[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_C, (const float32*)&cubeBasisC[face].x);
        float32 faceF = (float32)face;
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_INDEX, &faceF);

        for (uint32 mip = 0; mip < outputMipLevels; ++mip)
        {
            if (mip == 0)
            {
                cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, inputTexture);

                float32 srcLastMip = 1.f;
                cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_LAST_MIP, &srcLastMip);
                float32 srcMipLevel = 0.f;
                cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_MIP_LEVEL, &srcMipLevel);
            }
            else
            {
                cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, outputTexture);

                float32 srcLastMip = float32(mip);
                cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_LAST_MIP, &srcLastMip);
                float32 srcMipLevel = float32(mip - 1);
                cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_MIP_LEVEL, &srcMipLevel);
            }

            float32 dstMipLevel = float32(mip);
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_DST_MIP_LEVEL, &dstMipLevel);

            float32 faceSizeF = (float32)(outputTexture->width / (1 << mip));
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_SIZE, &faceSizeF);

            cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_DOWNSAMPLE_FILTER);

            quadRenderer.Render("EdgeFilterCubemap", cubemapFunctionsMaterial,
                                rhi::Viewport(0, 0, outputTexture->width / (1 << mip), outputTexture->height / (1 << mip)),
                                outputTexture->handle, (rhi::TextureFace)targetFaces[face], mip);
        }
    }
}

void CubemapRenderer::CopyCubemap(Texture* inputTexture,
                                  uint32 inputStartMip, uint32 mipCount,
                                  Texture* outputTexture, uint32 outputStartMip)
{
    DVASSERT(inputTexture->width == inputTexture->height);
    DVASSERT(outputTexture->width == outputTexture->height);

    cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, inputTexture);

    float32 srcLastMip = float32(inputStartMip + mipCount - 1);
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_LAST_MIP, &srcLastMip);
    float32 dstLastMip = float32(outputStartMip + mipCount - 1);
    cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_DST_LAST_MIP, &dstLastMip);

    for (uint32 face = 0; face < 6; ++face)
    {
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_A, (const float32*)&cubeBasisA[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_B, (const float32*)&cubeBasisB[face].x);
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_BASIS_C, (const float32*)&cubeBasisC[face].x);
        float32 faceF = (float32)face;
        cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_INDEX, &faceF);

        for (uint32 mip = 0; mip < mipCount; ++mip)
        {
            cubemapFunctionsMaterial->SetTexture(SRC_SAMPLER_NAME, inputTexture);

            float32 srcMipLevel = float32(inputStartMip + mip);
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_SRC_MIP_LEVEL, &srcMipLevel);

            float32 dstMipLevel = float32(outputStartMip + mip);
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_DST_MIP_LEVEL, &dstMipLevel);

            uint32 inputMipWidth = inputTexture->width / (1 << (inputStartMip + mip));
            uint32 outputMipWidth = outputTexture->width / (1 << (outputStartMip + mip));

            DVASSERT(inputMipWidth == outputMipWidth);

            float32 faceSizeF = (float32)(inputMipWidth);
            cubemapFunctionsMaterial->SetPropertyValue(CUBEMAP_FACE_SIZE, &faceSizeF);

            cubemapFunctionsMaterial->PreBuildMaterial(CUBEMAP_DOWNSAMPLE_FILTER);

            quadRenderer.Render("CopyCubemap", cubemapFunctionsMaterial,
                                rhi::Viewport(0, 0, outputMipWidth, outputMipWidth),
                                outputTexture->handle, (rhi::TextureFace)targetFaces[face], mip);
        }
    }
}

void CubemapRenderer::GetBasisVectors(uint32 faceIndex, Vector3& direction, Vector3& up)
{
    static const Vector3 directions[] =
    {
      Vector3(+1.0f, 0.0f, 0.0f),
      Vector3(-1.0f, 0.0f, 0.0f),
      Vector3(0.0f, +1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, 0.0f, +1.0f),
      Vector3(0.0f, 0.0f, -1.0f),
    };
    DVASSERT(faceIndex < sizeof(directions) / sizeof(directions[0]));
    static const Vector3 ups[] =
    {
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, 0.0f, +1.0f),
      Vector3(0.0f, 0.0f, -1.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
    };
    DVASSERT(faceIndex < sizeof(ups) / sizeof(ups[0]));
    direction = directions[faceIndex];
    up = ups[faceIndex];
}
};
