#pragma once

#include "Asset/Asset.h"
#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "FileSystem/FilePath.h"
#include "Concurrency/Mutex.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/RenderBase.h"
#include "Render/UniqueStateSet.h"

#include "MemoryManager/MemoryProfiler.h"

namespace DAVA
{
/**
	\ingroup render
	\brief Class that represents texture objects in our SDK.
	This class support the following formats: RGBA8888, RGB565, RGBA4444, A8 on all platforms.
	For iOS it also support compressed PVR formats. (PVR2 and PVR4)
 */
class Image;
class TextureDescriptor;

class Texture : public AssetBase
{
    friend class TextureAssetLoader;
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_TEXTURE)
public:
    struct PathKey
    {
        PathKey(const FilePath& filePath);
        PathKey(const FilePath& filePath, rhi::TextureType typeHint);
        PathKey(const FilePath& filePath, eGPUFamily requestedGpu, rhi::TextureType typeHint = rhi::TEXTURE_TYPE_2D);

        bool operator==(const PathKey& other) const;

        FilePath path;
        eGPUFamily requestedGpu = GPU_INVALID;
        rhi::TextureType typeHint = rhi::TEXTURE_TYPE_2D;
    };
    static PathKey MakePinkKey(rhi::TextureType type = rhi::TEXTURE_TYPE_2D);

    struct UniqueTextureKey
    {
        UniqueTextureKey(PixelFormat format, uint32 width, uint32 height, bool generateMipMaps);
        UniqueTextureKey(PixelFormat format, uint32 width, uint32 height, bool generateMipMaps, std::shared_ptr<uint8[]> data);
        UniqueTextureKey(RefPtr<Image> image, bool generateMipMaps);
        UniqueTextureKey(Vector<RefPtr<Image>>&& images);

        size_t uniqueKey;
        Any creationData;

    private:
        UniqueTextureKey(Vector<RefPtr<Image>>&& images, bool generateMipMaps);
        static size_t nextUniqueKey;
    };

    struct RenderTargetTextureKey
    {
        RenderTargetTextureKey();
        RenderTargetTextureKey(const String& uniqueName);

        bool operator==(const RenderTargetTextureKey& other) const;

        uint32 width = 0;
        uint32 height = 0;
        uint32 sampleCount = 1;
        uint32 mipLevelsCount = 1;
        PixelFormat format = PixelFormat::FORMAT_INVALID;
        rhi::TextureType textureType = rhi::TEXTURE_TYPE_2D;
        bool isDepth = false;
        bool needPixelReadback = false;
        bool ensurePowerOf2 = true;

        String uniqueKey;

    private:
        static uint32 newUniqueKey;
    };

    enum TextureState : uint8
    {
        STATE_INVALID = 0,
        STATE_DATA_LOADED,
        STATE_VALID
    };

    const static uint32 MINIMAL_WIDTH = 8;
    const static uint32 MINIMAL_HEIGHT = 8;

    const static uint32 INVALID_CUBEMAP_FACE = -1;
    const static uint32 CUBE_FACE_COUNT = 6;

    static Array<String, CUBE_FACE_COUNT> FACE_NAME_SUFFIX;

    Texture(const Any& assetKey);
    ~Texture() override;

    void TexImage(int32 level, uint32 width, uint32 height, const void* _data, uint32 dataSize, uint32 cubeFaceId);
    void CreateCubemapMipmapImages(Vector<Vector<Image*>>& images, uint32 mipmapLevelCount);

    Image* CreateImageFromRegion(const Rect2i& rect = Rect2i(0, 0, -1, -1), uint32 level = 0);

    void SetWrapMode(rhi::TextureAddrMode wrapU, rhi::TextureAddrMode wrapV, rhi::TextureAddrMode wrapW = rhi::TEXADDR_WRAP);
    void SetMinMagFilter(rhi::TextureFilter minFilter, rhi::TextureFilter magFilter, rhi::TextureMipFilter mipFilter);

    /**
        \brief Function to receive pathname of texture object
        \returns pathname of texture
     */
    const FilePath& GetPathname() const;

    bool IsPinkPlaceholder() const;
    inline TextureState GetState() const;

    void SetDebugInfo(const String& _debugInfo);
    uint32 GetDataSize() const;

    inline eGPUFamily GetSourceFileGPUFamily() const;
    inline TextureDescriptor* GetDescriptor() const;

    PixelFormat GetFormat() const;
    uint32 GetMipLevelsCount() const;

    static rhi::HSamplerState CreateSamplerStateHandle(const rhi::SamplerState::Descriptor::Sampler& samplerState);

protected:
    void ReleaseTextureData();

public: // properties for fast access
    rhi::HTexture handle = rhi::HTexture();
    rhi::HSamplerState samplerStateHandle = rhi::HSamplerState();
    rhi::HTextureSet singleTextureSet = rhi::HTextureSet();
    rhi::SamplerState::Descriptor::Sampler samplerState;

    uint32 maxAnisotropyLevel = 16; // GFX_COMPLETE - used to disable anisotropy in GPU landscape editor

    eGPUFamily loadedAsFile = GPU_ORIGIN;
    uint32 width = 0; // texture width
    uint32 height = 0; // texture height
    uint32 textureType = rhi::TEXTURE_TYPE_2D;
    TextureState state = STATE_INVALID;
    bool isRenderTarget = false;
    bool isPink = false;
    bool isDepth = false;

#if defined(__DAVAENGINE_DEBUG__)
    FastName debugInfo;
#endif

    TextureDescriptor* texDescriptor = nullptr;
    uint32 levelsCount = 1;
    bool autoGeneratedMips = false;
};

// Implementation of inline functions

inline eGPUFamily Texture::GetSourceFileGPUFamily() const
{
    return loadedAsFile;
}

inline Texture::TextureState Texture::GetState() const
{
    return state;
}

inline TextureDescriptor* Texture::GetDescriptor() const
{
    return texDescriptor;
}

template <>
bool AnyCompare<Texture::PathKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<Texture::PathKey>;

template <>
bool AnyCompare<Texture::UniqueTextureKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<Texture::UniqueTextureKey>;

template <>
bool AnyCompare<Texture::RenderTargetTextureKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<Texture::RenderTargetTextureKey>;
}
