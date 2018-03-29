#include "RuntimeTextures.h"
#include "Engine/Engine.h"
#include "Render/Renderer.h"
#include "Render/RenderBase.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/ServiceTextures.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Material/NMaterialManager.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceManagerTypes.h"
#include "Render/Renderer.h"

namespace DAVA
{
namespace
{
const FastName RUNTIME_TEXTURE_NAMES[RuntimeTextures::RUNTIME_TEXTURES_COUNT] =
{
  FastName("unknownTexture"),
  FastName("reflectionTexture"),
  FastName("refractionTexture"),
  FastName("UVPickingTexture"),
  FastName("TEXTURE_SHARED_DEPTHBUFFER"),
  FastName("gBuffer0"),
  FastName("gBuffer1"),
  FastName("gBuffer2"),
  FastName("gBuffer3"),
  FastName("gBuffer0_copy"),
  FastName("gBuffer1_copy"),
  FastName("gBuffer2_copy"),
  FastName("gBuffer3_copy"),
  FastName("indirectSpecularLookup"),
  FastName("hammersleySet"),
  FastName("noiseTexture64x64"),
  FastName("directionalShadowMap"),
  FastName("velocityBuffer")
};

const static PixelFormat REFLECTION_PIXEL_FORMAT = PixelFormat::FORMAT_RGB565;
const static PixelFormat REFRACTION_PIXEL_FORMAT = PixelFormat::FORMAT_RGB565;
const static PixelFormat PICKING_PIXEL_FORMAT = PixelFormat::FORMAT_RGBA32F;
const static PixelFormat SHADOWMAP_PIXEL_FORMAT = PixelFormat::FORMAT_R16F;
const static PixelFormat GBUFFER_PIXEL_FORMAT = PixelFormat::FORMAT_RGBA8888;
const static PixelFormat LDR_PIXEL_FORMAT = PixelFormat::FORMAT_RGBA8888;
const static PixelFormat VELOCITY_PIXEL_FORMAT = PixelFormat::FORMAT_RG16F;
const static int32 SHADOW_CASCADE_SIZE = 1024;

static uint64 RuntimeTexturesInvalidateCallback = 0;
}

#define USE_BLUE_NOISE_TEXTURE 1
#define LOAD_BRDF_LOOKUP_TEXTURE 0

RuntimeTextures::RuntimeTextures()
{
    RuntimeTexturesInvalidateCallback = NMaterialManager::Instance().RegisterInvalidateCallback([this]() {
        rhi::DeleteTexture(runtimeTextures[TEXTURE_INDIRECT_SPECULAR_LOOKUP]);
        runtimeTextures[TEXTURE_INDIRECT_SPECULAR_LOOKUP] = rhi::HTexture();
    });
}

RuntimeTextures::~RuntimeTextures()
{
    NMaterialManager::Instance().UnregisterInvalidateCallback(RuntimeTexturesInvalidateCallback);
}

void RuntimeTextures::Reset(Size2i screenDim)
{
    // GFX_COMPLETE not affected by screen dim for now, later ensure gbuffer is enough
    const static int32 REFLECTION_TEX_SIZE = 512;
    const static int32 REFRACTION_TEX_SIZE = 512;
    const static int32 PICKING_TEX_SIZE = 2048;
    const static int32 TEXTURE_GLOBAL_REFLECTION = 512;

    Size2i GBUFFER_TEX_SIZE;

    DeviceManager* deviceManager = GetEngineContext()->deviceManager;
    if (deviceManager->GetDisplayCount() > 0)
    {
        const DisplayInfo& primaryDisplay = deviceManager->GetPrimaryDisplay();
        GBUFFER_TEX_SIZE = Size2i(static_cast<int32>(primaryDisplay.rect.dx), static_cast<int32>(primaryDisplay.rect.dy));
    }
    else
    {
        GBUFFER_TEX_SIZE = Size2i(2048, 2048);
    }
    Size2i VELOCITY_BUFFER_SIZE = GBUFFER_TEX_SIZE;

    runtimeTextureSizes[TEXTURE_REFLECTION] = Size2i(REFLECTION_TEX_SIZE, REFLECTION_TEX_SIZE);
    runtimeTextureSizes[TEXTURE_REFRACTION] = Size2i(REFRACTION_TEX_SIZE, REFRACTION_TEX_SIZE);
    runtimeTextureSizes[TEXTURE_UVPICKING] = Size2i(PICKING_TEX_SIZE, PICKING_TEX_SIZE);
    runtimeTextureSizes[TEXTURE_SHARED_DEPTHBUFFER] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);

    runtimeTextureSizes[TEXTURE_GBUFFER_0] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_GBUFFER_1] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_GBUFFER_2] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_GBUFFER_3] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_GBUFFER_0_COPY] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_GBUFFER_1_COPY] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_GBUFFER_2_COPY] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_GBUFFER_3_COPY] = Size2i(GBUFFER_TEX_SIZE.dx, GBUFFER_TEX_SIZE.dy);
    runtimeTextureSizes[TEXTURE_HAMMERSLEY_SET] = Size2i(1024, 1);

#if (!LOAD_BRDF_LOOKUP_TEXTURE)
    runtimeTextures[TEXTURE_INDIRECT_SPECULAR_LOOKUP] = rhi::HTexture();
    runtimeTextureSizes[TEXTURE_INDIRECT_SPECULAR_LOOKUP] = Size2i(128, 128);
#endif

    int32 cascadesCount = Renderer::GetRuntimeFlags().GetFlagValue(RuntimeFlags::Flag::SHADOW_CASCADES);
    runtimeTextureSizes[TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER] = Size2i(SHADOW_CASCADE_SIZE, cascadesCount * SHADOW_CASCADE_SIZE);

    runtimeTextureSizes[TEXTURE_VELOCITY] = Size2i(VELOCITY_BUFFER_SIZE.dx, VELOCITY_BUFFER_SIZE.dy);

    samplerDescriptors[TEXTURE_UVPICKING].addrU = rhi::TEXADDR_CLAMP;
    samplerDescriptors[TEXTURE_UVPICKING].addrV = rhi::TEXADDR_CLAMP;
    samplerDescriptors[TEXTURE_UVPICKING].addrW = rhi::TEXADDR_CLAMP;
    samplerDescriptors[TEXTURE_UVPICKING].magFilter = rhi::TEXFILTER_NEAREST;
    samplerDescriptors[TEXTURE_UVPICKING].minFilter = rhi::TEXFILTER_NEAREST;
    samplerDescriptors[TEXTURE_UVPICKING].mipFilter = rhi::TEXMIPFILTER_NONE;

    samplerDescriptors[TEXTURE_SCREEN_SPACE_NOISE].addrU = rhi::TEXADDR_WRAP;
    samplerDescriptors[TEXTURE_SCREEN_SPACE_NOISE].addrV = rhi::TEXADDR_WRAP;
    samplerDescriptors[TEXTURE_SCREEN_SPACE_NOISE].addrW = rhi::TEXADDR_WRAP;
    samplerDescriptors[TEXTURE_SCREEN_SPACE_NOISE].magFilter = rhi::TEXFILTER_NEAREST;
    samplerDescriptors[TEXTURE_SCREEN_SPACE_NOISE].minFilter = rhi::TEXFILTER_NEAREST;
    samplerDescriptors[TEXTURE_SCREEN_SPACE_NOISE].mipFilter = rhi::TEXMIPFILTER_NONE;

    samplerDescriptors[TEXTURE_INDIRECT_SPECULAR_LOOKUP].addrU = rhi::TEXADDR_CLAMP;
    samplerDescriptors[TEXTURE_INDIRECT_SPECULAR_LOOKUP].addrV = rhi::TEXADDR_CLAMP;
    samplerDescriptors[TEXTURE_INDIRECT_SPECULAR_LOOKUP].addrW = rhi::TEXADDR_CLAMP;
    samplerDescriptors[TEXTURE_INDIRECT_SPECULAR_LOOKUP].magFilter = rhi::TEXFILTER_LINEAR;
    samplerDescriptors[TEXTURE_INDIRECT_SPECULAR_LOOKUP].minFilter = rhi::TEXFILTER_LINEAR;
    samplerDescriptors[TEXTURE_INDIRECT_SPECULAR_LOOKUP].mipFilter = rhi::TEXMIPFILTER_NONE;
}

RuntimeTextures::eRuntimeTextureSemantic RuntimeTextures::GetRuntimeTextureSemanticByName(const FastName& name)
{
    for (int32 k = 0; k < RUNTIME_TEXTURES_COUNT; ++k)
        if (name == RUNTIME_TEXTURE_NAMES[k])
            return eRuntimeTextureSemantic(k);
    return TEXTURE_STATIC;
}

rhi::HTexture RuntimeTextures::GetRuntimeTexture(eRuntimeTextureSemantic semantic)
{
    DVASSERT(semantic != TEXTURE_STATIC);
    DVASSERT(semantic < RUNTIME_TEXTURES_COUNT);

    if (!runtimeTextures[semantic].IsValid())
        InitRuntimeTexture(semantic);

    return runtimeTextures[semantic];
}

rhi::HTexture RuntimeTextures::GetPinkTexture(rhi::TextureType type)
{
    int32 textureIndex = type - 1;

    if (!pinkTexture[textureIndex])
        pinkTexture[textureIndex] = Texture::CreatePink(type);

    return pinkTexture[textureIndex]->handle;
}

rhi::SamplerState::Descriptor::Sampler RuntimeTextures::GetPinkTextureSamplerState(rhi::TextureType type)
{
    int32 textureIndex = type - 1;

    if (!pinkTexture[textureIndex])
        pinkTexture[textureIndex] = Texture::CreatePink(type);

    return pinkTexture[textureIndex]->samplerState;
}

void RuntimeTextures::InvalidateTexture(eRuntimeTextureSemantic i)
{
    if (!runtimeTextures[i].IsValid())
        return;

    rhi::DeleteTexture(runtimeTextures[i]);
    runtimeTextures[i] = rhi::HTexture();
}

void RuntimeTextures::ClearRuntimeTextures()
{
    for (int32 i = 0; i < RUNTIME_TEXTURES_COUNT; i++)
        InvalidateTexture(static_cast<eRuntimeTextureSemantic>(i));

    SafeRelease(pinkTexture[0]);
    SafeRelease(pinkTexture[1]);
}

void RuntimeTextures::InitRuntimeTexture(eRuntimeTextureSemantic semantic)
{
    DVASSERT(!runtimeTextures[semantic].IsValid());

    rhi::Texture::Descriptor descriptor;
    descriptor.width = runtimeTextureSizes[semantic].dx;
    descriptor.height = runtimeTextureSizes[semantic].dy;

    bool memorylessFetchAttachments = rhi::DeviceCaps().isFramebufferFetchSupported &&
    ((Renderer::GetCurrentRenderFlow() == RenderFlow::TileBasedHDRForward) ||
     (Renderer::GetCurrentRenderFlow() == RenderFlow::TileBasedHDRDeferred));

    switch (semantic)
    {
    case RuntimeTextures::TEXTURE_REFLECTION:
    {
        PixelFormatDescriptor formatDesc = PixelFormatDescriptor::GetPixelFormatDescriptor(REFLECTION_PIXEL_FORMAT);
        PixelFormat format = rhi::DeviceCaps().textureFormat[formatDesc.format].renderable ? REFLECTION_PIXEL_FORMAT : PixelFormat::FORMAT_RGBA8888;

        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = PixelFormatDescriptor::GetPixelFormatDescriptor(format).format;
        runtimeTextures[semantic] = rhi::CreateTexture(descriptor);

        samplerDescriptors[semantic].addrU = rhi::TEXADDR_MIRROR;
        samplerDescriptors[semantic].addrV = rhi::TEXADDR_MIRROR;
        samplerDescriptors[semantic].addrW = rhi::TEXADDR_MIRROR;
        samplerDescriptors[semantic].magFilter = rhi::TEXFILTER_LINEAR;
        samplerDescriptors[semantic].minFilter = rhi::TEXFILTER_LINEAR;
        samplerDescriptors[semantic].mipFilter = rhi::TEXMIPFILTER_NONE;

        runtimeTexturesFormat[semantic] = format;
        break;
    }

    case RuntimeTextures::TEXTURE_REFRACTION:
    {
        PixelFormatDescriptor formatDesc = PixelFormatDescriptor::GetPixelFormatDescriptor(REFRACTION_PIXEL_FORMAT);
        PixelFormat format = rhi::DeviceCaps().textureFormat[formatDesc.format].renderable ? REFRACTION_PIXEL_FORMAT : PixelFormat::FORMAT_RGBA8888;

        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = PixelFormatDescriptor::GetPixelFormatDescriptor(format).format;
        runtimeTextures[semantic] = rhi::CreateTexture(descriptor);

        samplerDescriptors[semantic].addrU = rhi::TEXADDR_MIRROR;
        samplerDescriptors[semantic].addrV = rhi::TEXADDR_MIRROR;
        samplerDescriptors[semantic].addrW = rhi::TEXADDR_MIRROR;
        samplerDescriptors[semantic].magFilter = rhi::TEXFILTER_LINEAR;
        samplerDescriptors[semantic].minFilter = rhi::TEXFILTER_LINEAR;
        samplerDescriptors[semantic].mipFilter = rhi::TEXMIPFILTER_NONE;

        runtimeTexturesFormat[semantic] = format;
        break;
    }

    case RuntimeTextures::TEXTURE_UVPICKING:
    {
        PixelFormatDescriptor formatDesc = PixelFormatDescriptor::GetPixelFormatDescriptor(PICKING_PIXEL_FORMAT);
        PixelFormat format = rhi::DeviceCaps().textureFormat[formatDesc.format].renderable ? PICKING_PIXEL_FORMAT : PixelFormat::FORMAT_RGBA8888;

        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = PixelFormatDescriptor::GetPixelFormatDescriptor(format).format;
        runtimeTextures[semantic] = rhi::CreateTexture(descriptor);
        runtimeTexturesFormat[semantic] = format;
        break;
    }

    case RuntimeTextures::TEXTURE_VELOCITY:
    {
        descriptor.cpuAccessRead = false;
        descriptor.cpuAccessWrite = false;
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.memoryless = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_RG16F;
        runtimeTextures[semantic] = rhi::CreateTexture(descriptor);
        runtimeTexturesFormat[semantic] = VELOCITY_PIXEL_FORMAT;
        break;
    }

    case RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER:
    {
        descriptor.cpuAccessRead = false;
        descriptor.cpuAccessWrite = false;
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.memoryless = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_D24S8;
        runtimeTextures[semantic] = rhi::CreateTexture(descriptor);
        runtimeTexturesFormat[semantic] = PixelFormat::FORMAT_INVALID;
        break;
    }

    case RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER:
    {
        rhi::TextureFormat textureFormat = (Renderer::GetCurrentRenderFlow() == RenderFlow::LDRForward) ? rhi::TEXTURE_FORMAT_D16 : rhi::TEXTURE_FORMAT_D32F;

        int32 cascadesCount = std::max(1, Renderer::GetRuntimeFlags().GetFlagValue(RuntimeFlags::Flag::SHADOW_CASCADES));
        runtimeTextureSizes[TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER] = Size2i(SHADOW_CASCADE_SIZE, cascadesCount * SHADOW_CASCADE_SIZE);

        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.cpuAccessRead = false;
        descriptor.cpuAccessWrite = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = textureFormat;
        descriptor.width = runtimeTextureSizes[TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER].dx;
        descriptor.height = runtimeTextureSizes[TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER].dy;
        runtimeTextures[semantic] = rhi::CreateTexture(descriptor);
        runtimeTexturesFormat[semantic] = PixelFormat::FORMAT_INVALID;

        samplerDescriptors[semantic].addrU = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].addrV = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].addrW = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].magFilter = rhi::TEXFILTER_LINEAR;
        samplerDescriptors[semantic].minFilter = rhi::TEXFILTER_LINEAR;
        samplerDescriptors[semantic].mipFilter = rhi::TEXMIPFILTER_NONE;
        samplerDescriptors[semantic].anisotropyLevel = 1;
        samplerDescriptors[semantic].comparisonEnabled = true;
        samplerDescriptors[semantic].comparisonFunction = rhi::CMP_LESSEQUAL;
        break;
    }
    case RuntimeTextures::TEXTURE_GBUFFER_0:
    case RuntimeTextures::TEXTURE_GBUFFER_1:
    case RuntimeTextures::TEXTURE_GBUFFER_2:
    case RuntimeTextures::TEXTURE_GBUFFER_3:
    case RuntimeTextures::TEXTURE_GBUFFER_0_COPY:
    case RuntimeTextures::TEXTURE_GBUFFER_1_COPY:
    case RuntimeTextures::TEXTURE_GBUFFER_2_COPY:
    case RuntimeTextures::TEXTURE_GBUFFER_3_COPY:
    {
        descriptor.memoryless = memorylessFetchAttachments && (semantic != RuntimeTextures::TEXTURE_GBUFFER_3);
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        DVASSERT(PixelFormatDescriptor::GetPixelFormatDescriptor(GBUFFER_PIXEL_FORMAT).format == rhi::TEXTURE_FORMAT_R8G8B8A8);

        if ((semantic == TEXTURE_GBUFFER_3) || (semantic == TEXTURE_GBUFFER_3_COPY))
        {
            descriptor.format = rhi::TEXTURE_FORMAT_R32F;
            runtimeTexturesFormat[semantic] = PixelFormat::FORMAT_RGBA32F;
        }
        else if ((semantic == TEXTURE_GBUFFER_1) || (semantic == TEXTURE_GBUFFER_1_COPY))
        {
            descriptor.format = rhi::TEXTURE_FORMAT_RGB10A2;
            runtimeTexturesFormat[semantic] = PixelFormat::FORMAT_RGBA8888;
        }
        else
        {
            descriptor.format = rhi::TEXTURE_FORMAT_R8G8B8A8;
            runtimeTexturesFormat[semantic] = GBUFFER_PIXEL_FORMAT;
        }

        runtimeTextures[semantic] = rhi::CreateTexture(descriptor);

        samplerDescriptors[semantic].addrU = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].addrV = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].addrW = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].magFilter = rhi::TEXFILTER_NEAREST;
        samplerDescriptors[semantic].minFilter = rhi::TEXFILTER_NEAREST;
        samplerDescriptors[semantic].mipFilter = rhi::TEXMIPFILTER_NONE;
        break;
    }
    case RuntimeTextures::TEXTURE_INDIRECT_SPECULAR_LOOKUP:
    {
#if (LOAD_BRDF_LOOKUP_TEXTURE)
        ScopedPtr<Image> image(ImageSystem::LoadSingleMip("~res:/Textures/brdflookup.png"));
        runtimeTextureSizes[semantic] = Size2i(image->width, image->height);
        runtimeTextures[semantic] = Texture::CreateFromData(image, false)->handle;
#else
        runtimeTextures[semantic] = ServiceTextures::GenerateSplitSumApproximationLookupTexture(descriptor.width, descriptor.height)->handle;
#endif
        break;
    }
    case RuntimeTextures::TEXTURE_HAMMERSLEY_SET:
    {
        runtimeTextures[semantic] = ServiceTextures::CreateHammersleySet(1024)->handle;
        samplerDescriptors[semantic].addrU = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].addrV = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].addrW = rhi::TEXADDR_CLAMP;
        samplerDescriptors[semantic].magFilter = rhi::TEXFILTER_NEAREST;
        samplerDescriptors[semantic].minFilter = rhi::TEXFILTER_NEAREST;
        samplerDescriptors[semantic].mipFilter = rhi::TEXMIPFILTER_NONE;
        break;
    }
    case RuntimeTextures::TEXTURE_SCREEN_SPACE_NOISE:
    {
#if (USE_BLUE_NOISE_TEXTURE)
        ScopedPtr<Image> image(ImageSystem::LoadSingleMip("~res:/Textures/bluenoise.png"));
        runtimeTextureSizes[semantic] = Size2i(image->width, image->height);
        DVASSERT(image->format == PixelFormat::FORMAT_RGBA8888);

        for (uint32 i = 0, e = image->width * image->height; i < e; ++i)
        {
            float angle = static_cast<float>(image->data[4 * i]) / 255.0f;
            float sinA = std::sin(2.0f * PI * angle);
            float cosA = std::cos(2.0f * PI * angle);
            image->data[4 * i + 0] = static_cast<uint32>((cosA * 0.5f + 0.5f) * 255.0f);
            image->data[4 * i + 1] = static_cast<uint32>((sinA * 0.5f + 0.5f) * 255.0f);
        }
        Texture* noiseTexture = Texture::CreateFromData(image, false);
        runtimeTextures[semantic] = noiseTexture->handle;
#else
        runtimeTextureSizes[TEXTURE_SCREEN_SPACE_NOISE] = Size2i(64, 64);
        Texture* noiseTexture = ServiceTextures::GenerateNoiseTexture(64, 64);
        runtimeTextures[TEXTURE_SCREEN_SPACE_NOISE] = noiseTexture->handle;
#endif
        break;
    }
    default:
        DVASSERT(false, "Trying to init unknown texture as runtime");
        break;
    }
}

rhi::SamplerState::Descriptor::Sampler RuntimeTextures::GetRuntimeTextureSamplerState(eRuntimeTextureSemantic semantic)
{
    return samplerDescriptors[semantic];
}

Size2i RuntimeTextures::GetRuntimeTextureSize(eRuntimeTextureSemantic semantic)
{
    return runtimeTextureSizes[semantic];
}

PixelFormat RuntimeTextures::GetRuntimeTextureFormat(eRuntimeTextureSemantic semantic)
{
    return runtimeTexturesFormat[semantic];
}
}
