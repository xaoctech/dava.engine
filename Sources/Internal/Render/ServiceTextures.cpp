#include "ServiceTextures.h"
#include "Render/Highlevel/QuadRenderer.h"
#include "Render/Material/NMaterial.h"
#include "Image/Image.h"
#include "Image/ImageSystem.h"

#define GENERATE_BRDF_LOOKUP_TEXTURE 1

namespace DAVA
{
namespace ServiceTextureDetails
{
uint32 ReverseBits(uint32 bits);
Vector2 HammersleySample(uint32 sampleIndex, uint32 sampleCount);
}

Asset<Texture> ServiceTextures::CreateHammersleySet(std::array<uint32, 4> sizes)
{
    uint32 height = static_cast<uint32>(sizes.size());
    uint32 width = *std::max_element(sizes.begin(), sizes.end());
    std::shared_ptr<uint8[]> buffer(new uint8[sizeof(Vector2) * width * height]);

    Vector2* data = reinterpret_cast<Vector2*>(buffer.get());

    for (uint32 y = 0; y < height; ++y)
    {
        for (uint32 x = 0; x < width; ++x)
        {
            if (x < sizes[y])
                data[y * width + x] = ServiceTextureDetails::HammersleySample(x, sizes[y]);
        }
    }

    Texture::UniqueTextureKey key(FORMAT_RG32F, width, height, false, buffer);
    Asset<Texture> texture = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
    texture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
    texture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    return texture;
}

rhi::HTexture ServiceTextures::CreateHammersleySet(uint32 count)
{
    Vector<Vector2> data(count);
    for (uint32 i = 0; i < count; ++i)
    {
        data[i] = ServiceTextureDetails::HammersleySample(i, count);
    }

    rhi::Texture::Descriptor descriptor;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = false;
    descriptor.width = count;
    descriptor.height = 1;
    descriptor.type = rhi::TEXTURE_TYPE_2D;
    descriptor.format = PixelFormatDescriptor::GetPixelFormatDescriptor(FORMAT_RG32F).format;
    descriptor.levelCount = 1;
    descriptor.initialData[0] = reinterpret_cast<uint8*>(data.data());
    return rhi::CreateTexture(descriptor);
}

rhi::HTexture ServiceTextures::GenerateNoiseTexture(uint32 width, uint32 height)
{
    uint32 count = width * height;
    Vector<uint32> data(count);
    for (uint32 i = 0; i < count; ++i)
    {
        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float sinA = std::sin(2.0f * PI * angle);
        float cosA = std::cos(2.0f * PI * angle);
        uint32 r = static_cast<uint32>((cosA * 0.5f + 0.5f) * 255.0f);
        uint32 g = static_cast<uint32>((sinA * 0.5f + 0.5f) * 255.0f);
        uint32 b = rand() % 255;
        uint32 a = rand() % 255;
        data[i] = r | (g << 8) | (b << 16) | (a << 24);
    }

    rhi::Texture::Descriptor descriptor;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = false;
    descriptor.width = width;
    descriptor.height = height;
    descriptor.type = rhi::TEXTURE_TYPE_2D;
    descriptor.format = PixelFormatDescriptor::GetPixelFormatDescriptor(FORMAT_RGBA8888).format;
    descriptor.levelCount = 1;
    descriptor.initialData[0] = data.data();
    return rhi::CreateTexture(descriptor);
}

rhi::HTexture ServiceTextures::GenerateSplitSumApproximationLookupTexture(uint32 width, uint32 height)
{
    rhi::Texture::Descriptor descriptor;
    descriptor.width = width;
    descriptor.height = height;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = true;
    descriptor.needRestore = false;
    descriptor.cpuAccessWrite = false;
    descriptor.type = rhi::TEXTURE_TYPE_2D;
    descriptor.format = PixelFormatDescriptor::GetPixelFormatDescriptor(FORMAT_RGBA8888).format;
    ;
    descriptor.sampleCount = 1;
    descriptor.levelCount = 1;
    descriptor.cpuAccessRead = true;

    rhi::HTexture result = rhi::CreateTexture(descriptor);

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetFXName(FastName("~res:/Materials2/CubemapConvolution.material"));

    material->PreBuildMaterial(FastName("IntegrateBRDFLookup"));
    QuadRenderer().Render("GenerateSplitSum", material, rhi::Viewport(0, 0, width, height), result, rhi::TextureFace::TEXTURE_FACE_NONE, 0, rhi::LOADACTION_CLEAR, +50);
    /*
    Renderer::RegisterSyncCallback(rhi::GetCurrentFrameSyncObject(), [result, width, height, textureFormat](rhi::HSyncObject obj) {
        void* data = rhi::MapTexture(result->handle);
        {
            ScopedPtr<Image> img(Image::CreateFromData(width, height, textureFormat, reinterpret_cast<uint8*>(data)));
            ImageSystem::Save("~doc:/brdflookup.png", img, textureFormat);
        }
        rhi::UnmapTexture(result->handle);
    });
	*/

    return result;
}

rhi::HTexture ServiceTextures::GeneratePointLightLookupTexture(uint32 faceSize)
{
    Vector<uint32> facesData[6];
    for (uint32 i = 0; i < 6; ++i)
    {
        uint32 faceIndex = 32 * i;
        facesData[i].resize(faceSize * faceSize * 4);
        uint32* data = facesData[i].data();
        for (uint32 y = 0; y < faceSize; ++y)
        {
            float dy = static_cast<float>(y) / static_cast<float>(faceSize - 1);
            for (uint32 x = 0; x < faceSize; ++x)
            {
                float dx = static_cast<float>(x) / static_cast<float>(faceSize - 1);
                uint32 r = Clamp(static_cast<uint32>(255.0f * dx), 0u, 255u);
                uint32 g = Clamp(static_cast<uint32>(255.0f * dy), 0u, 255u);
                uint32 b = faceIndex;
                uint32 a = 255;
                *data++ = (r) | (g << 8) | (b << 16) | (a << 24);
            }
        }
    }

    rhi::Texture::Descriptor desc;
    desc.autoGenMipmaps = false;
    desc.cpuAccessRead = false;
    desc.cpuAccessWrite = false;
    desc.format = rhi::TextureFormat::TEXTURE_FORMAT_R8G8B8A8;
    desc.width = faceSize;
    desc.height = faceSize;
    desc.isRenderTarget = false;
    desc.levelCount = 1;
    desc.needRestore = false; // TODO : make restorable
    desc.type = rhi::TextureType::TEXTURE_TYPE_CUBE;
    desc.initialData[0] = facesData[0].data();
    desc.initialData[1] = facesData[1].data();
    desc.initialData[2] = facesData[2].data();
    desc.initialData[3] = facesData[3].data();
    desc.initialData[4] = facesData[4].data();
    desc.initialData[5] = facesData[5].data();
    return rhi::CreateTexture(desc);
}

/*
 * Details implementation
 */
namespace ServiceTextureDetails
{
uint32 ReverseBits(uint32 bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
    bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
    return bits;
}

Vector2 HammersleySample(uint32 sampleIndex, uint32 sampleCount)
{
    float e1 = static_cast<float>(sampleIndex) / static_cast<float>(sampleCount);
    double e2 = static_cast<double>(ReverseBits(sampleIndex)) * 2.3283064365386963e-10;
    return Vector2(e1, static_cast<float>(e2));
}
}
}
